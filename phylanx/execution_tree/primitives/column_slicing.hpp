// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_COLUMN_SLICING_HPP
#define PHYLANX_COLUMN_SLICING_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class column_slicing_operation : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        column_slicing_operation() = default;

         /**
         * @brief Column Slicing Primitive
         *
         * This primitive returns a slice of the original data.
         * @param operands Vector of phylanx node data objects of
         * size either three or four
         *
         * If used inside PhySL:
         *
         *      slice_column (input, col_start, col_stop, steps(optional) )
         *
         *          input : Scalar, Vector or a Matrix
         *          col_start     : Starting index of the slice
         *          col_stop      : Stopping index of the slice
         *          steps          : Go from col_start to col_stop in steps
         *
         *  Note: Indices and steps can have negative vlaues and negative values
         *  indicate direction, similar to python.
         */

        column_slicing_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };

    PHYLANX_EXPORT primitive create_column_slicing_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif //PHYLANX_COLUMN_SLICING_HPP
