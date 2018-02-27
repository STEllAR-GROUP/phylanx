// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_COLUMN_SET_HPP
#define PHYLANX_COLUMN_SET_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <memory>
#include <string>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class column_set_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<column_set_operation>
    {
    protected:
        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;

        using storage0d_type = typename arg_type::storage0d_type;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

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
         *  Note: Indices and steps can have negative values and negative values
         *  indicate direction, similar to python.
         */

        column_set_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        bool check_col_set_parameters(std::int64_t start, std::int64_t stop,
            std::int64_t step, std::size_t array_length) const;
        std::vector<std::int64_t> create_list_col_set(std::int64_t start,
            std::int64_t stop,
            std::int64_t step,
            std::int64_t array_length) const;
        primitive_argument_type column_set0d(args_type&& args) const;
        primitive_argument_type column_set1d(args_type&& args) const;
        primitive_argument_type column_set2d(args_type&& args) const;
    };

    PHYLANX_EXPORT primitive create_column_set_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif //PHYLANX_COLUMN_SET_HPP
