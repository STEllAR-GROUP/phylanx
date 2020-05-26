//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/annotate_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ////////////////////////////////////////////////////////////////////////////
    primitive create_annotate(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
    {
        static std::string type("annotate");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    ////////////////////////////////////////////////////////////////////////////
    constexpr const char* const annotate_helpstring = R"(
        target, args

        Args:

            target : the value that has to be annotated
            *args (arg list) : an optional list of annotations (default is `None`)

        Returns:

        The `target` annotated with the list of values given by `*args`.)";

    match_pattern_type const annotate_primitive::match_data_annotate = {
        hpx::util::make_tuple("annotate",
            std::vector<std::string>{
                "annotate(_1_target, __arg(_2_args, nil))"},
            &create_annotate, &create_primitive<annotate_primitive>,
            annotate_helpstring)};

    constexpr const char* const annotate_d_helpstring = R"(
        target, name, args

        Args:

            target : the value that has to be annotated
            name : a unique string identifying the annotated object across
                   localities
            *args (arg list) : an optional list of annotations (default is `None`)

        Returns:

        The `target` annotated with the list of values given by `*args`. The
        `target` is also identified by the given `name` across localities.)";

    match_pattern_type const annotate_primitive::match_data_annotate_d = {
        hpx::util::make_tuple("annotate_d",
            std::vector<std::string>{
                "annotate_d(_1_target, _2_name, __arg(_3_args, nil))"},
            &create_annotate, &create_primitive<annotate_primitive>,
            annotate_d_helpstring)};

    ////////////////////////////////////////////////////////////////////////////
    annotate_primitive::annotate_primitive(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
      , func_name_(extract_function_name(name))
    {
    }

    ////////////////////////////////////////////////////////////////////////////
    primitive_argument_type annotate_primitive::annotate(
        primitive_argument_type&& target, ir::range&& args) const
    {
        // retrieve annotation and set the annotation on the target
        target.set_annotation(annotation{std::move(args)}, name_, codename_);
        return std::move(target);
    }

    ////////////////////////////////////////////////////////////////////////////
    primitive_argument_type annotate_primitive::annotate_d(
        primitive_argument_type&& target, std::string&& ann_name,
        ir::range&& args) const
    {
        // retrieve local annotation and generate the overall annotation
        annotation_information ann_info{std::move(ann_name), 0ll};
        annotation localities;
        if (phylanx::execution_tree::extract_string_value(*args.begin()) ==
            "args")
        {
            annotation tmp(args);
            annotation locality_ann;
            annotation tiles_ann;
            if (tmp.find("locality", locality_ann) &&
                tmp.find("tile", tiles_ann))
            {
                ir::range tmp = tiles_ann.get_range();
                localities = localities_annotation(target, locality_ann,
                    annotation{std::move(tmp)}, ann_info, name_, codename_);
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "annotate_primitive::annotate_d",
                    generate_error_message(
                        "locality and/or tile annotation not found"));
            }
        }
        else if (phylanx::execution_tree::extract_string_value(*args.begin()) ==
            "tile")
        {
            annotation locality_ann;
            localities = localities_annotation(target, locality_ann,
                annotation{std::move(args)}, ann_info, name_, codename_);
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotate_primitive::annotate_d",
                generate_error_message(
                    "no args annotation and tiles annotation not first"));
        }
        target.set_annotation(std::move(localities), name_, codename_);
        return std::move(target);
    }

    ////////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> annotate_primitive::eval_annotate(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotate_primitive::eval_annotate",
                generate_error_message(
                    "the annotate primitive requires two operands"));
        }

        auto f = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_)](
                hpx::future<primitive_argument_type>&& target,
                hpx::future<ir::range>&& args) -> primitive_argument_type {
                return this_->annotate(target.get(), args.get());
            },
            std::move(f),
            list_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }

    hpx::future<primitive_argument_type> annotate_primitive::eval_annotate_d(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "annotate_primitive::eval_annotate_d",
                generate_error_message(
                    "the annotate primitive requires three operands"));
        }

        auto ftarget = value_operand(operands[0], args, name_, codename_, ctx);
        auto fname = string_operand(operands[1], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_)](
                hpx::future<primitive_argument_type>&& target,
                hpx::future<std::string>&& name,
                hpx::future<ir::range>&& args) -> primitive_argument_type {
                return this_->annotate_d(target.get(), name.get(), args.get());
            },
            std::move(ftarget), std::move(fname),
            list_operand(operands[2], args, name_, codename_, std::move(ctx)));
    }

    hpx::future<primitive_argument_type> annotate_primitive::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // operands_[0] is expected to be the target,
        // operands_[1] is a list of arguments used for annotation

        if (func_name_ == "annotate_d")
        {
            return eval_annotate_d(operands, args, std::move(ctx));
        }

        return eval_annotate(operands, args, std::move(ctx));
    }
}}}
