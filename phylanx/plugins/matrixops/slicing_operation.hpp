// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_SLICING_OPERATION_1153_10242017_HPP
#define PHYLANX_SLICING_OPERATION_1153_10242017_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/lcos/future.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree { namespace primitives
{
    class slicing_operation
      : public primitive_component_base
      , public std::enable_shared_from_this<slicing_operation>
    {
    protected:
        using arg_type = ir::node_data<double>;
        using args_type = std::vector<arg_type>;
        using storage0d_type = typename arg_type::storage0d_type;
        using storage1d_type = typename arg_type::storage1d_type;
        using storage2d_type = typename arg_type::storage2d_type;

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& operands,
            std::vector<primitive_argument_type> const& args) const;

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
        *      slice (input, '(row_start, row_stop, row_steps(optional))
        *                , '(col_start, col_stop, col_steps(optional))
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
        *  Limitations: both row_steps and col_steps need to be provided or omitted.
        *  Functionality for specifying only row_steps or col_steps is not present.
        *
        *  Note: Indices and steps can have negative values and negative values
        *  indicate direction, similar to python.
        */

        slicing_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename);

        hpx::future<primitive_argument_type> eval(
            std::vector<primitive_argument_type> const& params) const override;

    private:
        std::vector<std::int64_t> create_list_slice(std::int64_t& start,
            std::int64_t& stop, std::int64_t step,
            std::size_t array_length) const;

        primitive_argument_type slicing0d(arg_type&& arg) const;
        primitive_argument_type slicing1d(arg_type&& arg,
            std::vector<std::int64_t> const& extracted_row) const;
        primitive_argument_type slicing2d(arg_type&& arg,
            std::vector<std::int64_t> const& extracted_row,
            std::vector<std::int64_t> const& extracted_column) const;

        primitive_argument_type handle_numeric_operand(
            std::vector<primitive_argument_type>&& args) const;
        primitive_argument_type handle_list_operand(
            std::vector<primitive_argument_type>&& args) const;

        std::vector<std::int64_t> extract_slicing_args_list(
            std::vector<primitive_argument_type>&& args,
            std::size_t size) const;
        std::vector<std::int64_t> extract_slicing_args_vector(
            std::vector<primitive_argument_type>&& args,
            std::size_t size) const;
        void extract_slicing_args_matrix(
            std::vector<primitive_argument_type>&& args,
            std::vector<std::int64_t>& extracted_row,
            std::vector<std::int64_t>& extracted_column, std::size_t rows,
            std::size_t columns) const;

        primitive_argument_type slice_list(ir::range&& list,
            std::vector<std::int64_t> const& columns) const;

        std::vector<std::int64_t> extract_slicing(
            primitive_argument_type&& arg, std::size_t arg_size) const;

        std::int64_t extract_integer_value(primitive_argument_type const& val,
            std::int64_t default_value) const;
    };

    inline primitive create_slicing_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "")
    {
        return create_primitive_component(
            locality, "slice", std::move(operands), name, codename);
    }
}}}

#endif //PHYLANX_SLICING_OPERATION_1153_10242017_HPP
