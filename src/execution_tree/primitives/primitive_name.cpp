//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_name.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_primitive_name(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__name");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const primitive_name::match_data =
    {
        hpx::util::make_tuple("__name",
            std::vector<std::string>{"__name(_1)"},
            &create_primitive_name, &create_primitive<primitive_name>,
            R"(arg
            Args:

                arg (primitive type) : return the name of the arg

            Returns:

                The name of the primitive type)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    primitive_name::primitive_name(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> primitive_name::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "primitive_name::eval",
                generate_error_message(
                    "phylanx.__name requires exactly one argument"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& arg)
                -> primitive_argument_type
            {
                using namespace phylanx::execution_tree::detail;
                std::string name = get_primitive_argument_type_name(arg.index());
                return primitive_argument_type{std::move(name)};
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
