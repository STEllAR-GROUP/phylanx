// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_ROW_SET_HPP
#define PHYLANX_ROW_SET_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class row_set_operation : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        row_set_operation() = default;

         /**
         * @brief Row Set Primitive
         *
         * This primitive returns a sets value to specific row.
         * @param operands Vector of phylanx node data objects of
         * size five
         *
         * If used inside PhySL:
         *
         *      set_row (input, col_start, col_stop, steps, value )
         *
         *          input         : Vector or a Matrix
         *          row_start     : Starting index of the set
         *          row_stop      : Stopping index of the set
         *          steps         : Go from row_start to row_stop in steps
         *          value         : The value to set
         *
         *  Note: Indices and steps can have negative vlaues and negative values
         *  indicate direction, similar to python.
         */

        row_set_operation(
            std::vector<primitive_argument_type>&& operands);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };

    PHYLANX_EXPORT primitive create_row_set_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "");
}}}

#endif //PHYLANX_ROW_SET_HPP
