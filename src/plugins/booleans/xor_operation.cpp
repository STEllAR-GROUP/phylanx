//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/xor_operation.hpp>
#include <phylanx/plugins/booleans/logical_operation_impl.hpp>

#include <hpx/include/util.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct xor_op
        {
            HPX_FORCEINLINE bool operator()(bool lhs, bool rhs) const
            {
                return (lhs && !rhs) || (!lhs && rhs);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    static constexpr char const* const help_string = R"(
        a, b
        Args:

            a (boolean) : a boolean argument
            *b (boolean list) : a list of boolean arguments

        Returns:

            The logical xor of `a` and `b`.
    )";

    match_pattern_type const xor_operation::match_data[2] =
    {
        match_pattern_type("__xor",
            std::vector<std::string>{"__xor(_1, __2)"},
            &create_xor_operation, &create_primitive<xor_operation>,
            help_string),

        match_pattern_type("logical_xor",
            std::vector<std::string>{"logical_xor(_1, __2)"},
            &create_xor_operation, &create_primitive<xor_operation>,
            help_string)
    };

    ///////////////////////////////////////////////////////////////////////////
    xor_operation::xor_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
