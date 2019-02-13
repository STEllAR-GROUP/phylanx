//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/and_operation.hpp>
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
        struct and_op
        {
            HPX_FORCEINLINE bool operator()(bool lhs, bool rhs) const
            {
                return lhs && rhs;
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

        The logical and of `a` and `b`.
    )";

    match_pattern_type const and_operation::match_data[2] =
    {
        match_pattern_type("__and",
            std::vector<std::string>{"_1 && __2", "__and(_1, __2)"},
            &create_and_operation, &create_primitive<and_operation>,
            help_string),

        match_pattern_type("logical_and",
            std::vector<std::string>{"logical_and(_1, __2)"},
            &create_and_operation, &create_primitive<and_operation>,
            help_string)
    };

    ///////////////////////////////////////////////////////////////////////////
    and_operation::and_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
