//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_type.hpp>

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
    primitive create_primitive_type(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__type");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const primitive_type::match_data =
    {
        hpx::util::make_tuple("__type",
            std::vector<std::string>{"__type(_1)"},
            &create_primitive_type, &create_primitive<primitive_type>,
            R"(arg
            Args:

                arg (primitive type) : return the type of the arg

            Returns:

                The variant index of the primitive type)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    primitive_type::primitive_type(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> primitive_type::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "primitive_type::eval",
                generate_error_message(
                    "phylanx.__type requires exactly one argument"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& arg)
                -> primitive_argument_type
            {
                return primitive_argument_type{std::int64_t(arg.index())};
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
