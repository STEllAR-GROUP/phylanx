// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_SLICING_OPERATION_1153_10242017_HPP
#define PHYLANX_SLICING_OPERATION_1153_10242017_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class slicing_operation : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        slicing_operation() = default;

        /**
        * @brief Slicing Primitive
        *
        * This primitive returns a slice of the original data.
        * @param operands Vector of phylanx node data objects of
        * size either five or seven
        *
        * If used inside PhySL:
        *
        *      slice (input, row_start, row_stop, row_steps(optional)
        *                , col_start, col_stop, col_steps(optional)
        *          )
        *
        *          input : Scalar, Vector or a Matrix
        *          row_start     : Starting index of the slice (row)
        *          row_stop      : Stopping index of the slice (row)
        *          row_steps     : Go from row_start to row_stop in row_steps
        *          col_start     : Starting index of the slice (column)
        *          col_stop      : Stopping index of the slice (column)
        *          col_steps     : Go from col_start to col_stop in steps
        *
        *  Quirks: In case the input is a vector, row_start, row_stop and row_steps
        *  determine the result. col_start, col_stop and col_step are ignored internally.
        *
        *  Limitations: both row_steps and col_steps need to be provieded or omitted.
        *  Functionality for specifying only row_steps or col_steps is not present.
        *
        *  Note: Indices and steps can have negative vlaues and negative values
        *  indicate direction, similar to python.
        */

        slicing_operation(std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };

    PHYLANX_EXPORT primitive create_slicing_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "");
}}}

#endif //PHYLANX_SLICING_OPERATION_1153_10242017_HPP
