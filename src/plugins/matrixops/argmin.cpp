//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/matrixops/argmin.hpp>
#include <phylanx/plugins/matrixops/argminmax_impl.hpp>

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const argmin::match_data =
    {
        hpx::util::make_tuple("argmin",
            std::vector<std::string>{"argmin(_1, _2)", "argmin(_1)"},
            &create_argmin, &create_primitive<argmin>, R"(
            a, axis
            Args:

                a (array) : a vector, matrix, or tensor
                axis (optional, int) : the axis along which to find the min

            Returns:

            The index of the minimum value in the array. If an axis is
            specified, a vector of minima along the axis is returned.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct argmin_op
        {
            template <typename T>
            static T initial()
            {
                return (std::numeric_limits<T>::max)();
            }

            template <typename T>
            static bool compare(T lhs, T rhs)
            {
                return lhs < rhs;
            }

            template <typename Iter>
            Iter operator()(Iter begin, Iter end) const
            {
                return std::min_element(begin, end);
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    argmin::argmin(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
