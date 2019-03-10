//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/phytype.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_phytype(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("phytype");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const phytype::match_data =
    {
        hpx::util::make_tuple("phytype",
            std::vector<std::string>{"phytype(_1)"},
            &create_phytype, &create_primitive<phytype>,
            R"(arg
            Args:

                arg (primitive type) : return the type of the arg

            Returns:

                The variant index of the primitive type)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    phytype::phytype(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> phytype::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                // If a single None is passed in python, we get a
                // zero length arg list here.
                if(args.size() == 0) {
                    std::int64_t tid = 0;
                    return primitive_argument_type{tid};
                } else {
                    std::int64_t tid = args[0].index();
                    return primitive_argument_type{tid};
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
