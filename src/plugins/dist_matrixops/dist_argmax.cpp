// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argmax.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argminmax_impl.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_argmax::match_data = {
        hpx::util::make_tuple("argmax_d",
            std::vector<std::string>{"argmax_d(_1, _2)", "argmax_d(_1)"},
            &create_dist_argmax, &execution_tree::create_primitive<dist_argmax>,
            R"(
            a, axis
            Args:

                a (array) : a vector, matrix, or tensor
                axis (optional, int): the axis along which to find the max

            Returns:

            The index of the maximum value in the array. If an axis is
            specified, a vector of maxima along the axis is returned.)")};

    ///////////////////////////////////////////////////////////////////////////
    dist_argmax::dist_argmax(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}    // namespace phylanx::dist_matrixops::primitives
