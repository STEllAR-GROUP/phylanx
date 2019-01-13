// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/minimum.hpp>
#include <phylanx/plugins/arithmetics/numeric_impl.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct minimum_op
        {
            template <typename T1, typename T2>
            auto operator()(T1 const& t1, T2 const& t2) const
            ->  decltype((blaze::min)(t1, t2))
            {
                return (blaze::min)(t1, t2);
            }

            template <typename T1, typename T2>
            void op_assign(T1& t1, T2 const& t2) const
            {
                t1 = (blaze::min)(t1, t2);
            }
        };
    }

    //////////////////////////////////////////////////////////////////////////
    match_pattern_type const minimum::match_data =
    {
        match_pattern_type{"minimum",
            std::vector<std::string>{"minimum(_1, __2)"},
            &create_minimum, &create_primitive<minimum>, R"(
            x0, x1
            Args:

                 x0 (number): An addend\n"
                *x1 (number list): A list of one or more arrays.\n"

            Returns:

            The element-wise minimum of all input arrays.)",
            true
        }
    };

    //////////////////////////////////////////////////////////////////////////
    minimum::minimum(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
