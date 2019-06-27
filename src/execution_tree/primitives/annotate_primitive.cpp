//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/annotate_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_annotate(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("annotate");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const annotate_primitive::match_data =
    {
        hpx::util::make_tuple("annotate",
            std::vector<std::string>{
                "annotate(_1_target, __arg(_2_args, nil))"
            },
            &create_annotate, &create_primitive<annotate_primitive>,
            R"(target, args

            Args:

                target : the value that has to be annotated
                *args (arg list) : a list of arguments

            Returns:

            The `target` annotated with the list of values given by `*args`.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    annotate_primitive::annotate_primitive(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> annotate_primitive::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // operands_[0] is expected to be the target,
        // operands_[1] is a list of arguments used for annotation

        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotate_primitive::annotate",
                generate_error_message(
                    "the annotate primitive requires two operands"));
        }

        auto f = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& target,
                    hpx::future<ir::range>&& args)
            ->  primitive_argument_type
            {
                // retrieve annotation
                annotation ann = annotation{args.get()};

                // If the annotation contains information related to the
                // locality of the data we should perform an all_to_all
                // operation to collect the information about all connected
                // objects.
                annotation locality_ann;
                if (ann.find("locality", locality_ann, this_->name_,
                        this_->codename_))
                {
                    primitive_argument_type&& t = target.get();
                    t.set_annotation(
                        meta_annotation(hpx::launch::sync, locality_ann,
                            std::move(ann), this_->name_, this_->codename_),
                        this_->name_, this_->codename_);
                    return primitive_argument_type{std::move(t)};
                }

                // set the annotation on the target
                primitive_argument_type&& t = target.get();
                t.set_annotation(std::move(ann), this_->name_, this_->codename_);
                return primitive_argument_type{std::move(t)};
            },
            std::move(f),
            list_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}

