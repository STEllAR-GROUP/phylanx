// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/matrixops/argmax.hpp>
#include <phylanx/plugins/matrixops/argminmax_impl.hpp>
#include <phylanx/util/detail/numeric_limits_min.hpp>

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const argmax::match_data =
    {
        hpx::util::make_tuple("argmax",
            std::vector<std::string>{"argmax(_1, _2)", "argmax(_1)"},
            &create_argmax, &create_primitive<argmax>, R"(
            a, axis
            Args:

                a (array) : a vector, matrix, or tensor
                axis (optional, int): the axis along which to find the max

            Returns:

            The index of the maximum value in the array. If an axis is
            specified, a vector of maxima along the axis is returned.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    argmax::argmax(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
