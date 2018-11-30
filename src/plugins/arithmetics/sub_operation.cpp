// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/sub_operation.hpp>
#include <phylanx/plugins/arithmetics/numeric_impl.hpp>

#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct sub_op
        {
            template <typename T1, typename T2>
            auto operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 - t2;
            }

            template <typename T1, typename T2>
            void op_assign(T1& t1, T2 const& t2) const
            {
                t1 -= t2;
            }
        };
    }

    //////////////////////////////////////////////////////////////////////////
    match_pattern_type const sub_operation::match_data =
    {
        match_pattern_type{"__sub",
            std::vector<std::string>{"_1 - __2", "__sub(_1, __2)"},
            &create_sub_operation, &create_primitive<sub_operation>, R"(
            x0, x1
            Args:

                 x0 (number): The value to subtract from\n"
                *x1 (number list): A list of values to subtract.\n"

            Returns:

            The difference of all arguments.)",
            true
        }
    };

    //////////////////////////////////////////////////////////////////////////
    sub_operation::sub_operation(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
