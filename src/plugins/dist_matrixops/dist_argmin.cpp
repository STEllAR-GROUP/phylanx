//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018-2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argmin.hpp>
#include <phylanx/plugins/dist_matrixops/dist_argminmax_impl.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_matrixops { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_argmin::match_data = {
        hpx::util::make_tuple("argmin_d",
            std::vector<std::string>{"argmin_d(_1, _2)", "argmin_d(_1)"},
            &create_dist_argmin, &execution_tree::create_primitive<dist_argmin>,
            R"(
            a, axis
            Args:

                a (array) : a vector, matrix, or tensor
                axis (optional, int) : the axis along which to find the min

            Returns:

            The index of the minimum value in the array. If an axis is
            specified, a vector of minima along the axis is returned.)")};

    ///////////////////////////////////////////////////////////////////////////
    dist_argmin::dist_argmin(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {
    }
}}}    // namespace phylanx::dist_matrixops::primitives
