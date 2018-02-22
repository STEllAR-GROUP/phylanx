// Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_COLUMN_SET_HPP
#define PHYLANX_COLUMN_SET_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <hpx/lcos/future.hpp>

#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class column_set_operation : public primitive_component_base
    {
    public:
        static match_pattern_type const match_data;

        column_set_operation() = default;

         /**
         * @brief Column Set Primitive
         *
         * This primitive returns a sets value to specific column.
         * @param operands Vector of phylanx node data objects of
         * size five
         *
         * If used inside PhySL:
         *
         *      set_column (input, col_start, col_stop, steps, value )
         *
         *          input         : Vector or a Matrix
         *          col_start     : Starting index of the set
         *          col_stop      : Stopping index of the set
         *          steps         : Go from col_start to col_stop in steps
         *          value         : The value to set
         *
         *  Note: Indices and steps can have negative vlaues and negative values
         *  indicate direction, similar to python.
         */

        column_set_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;
    };

    PHYLANX_EXPORT primitive create_column_set_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif //PHYLANX_COLUMN_SET_HPP
