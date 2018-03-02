// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_SLICING_OPERATION_1153_10242017_HPP
#define PHYLANX_SLICING_OPERATION_1153_10242017_HPP

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
        std::vector<int> create_list_slice(
            int start, int stop, int step, int array_length) const;
        primitive_argument_type slicing0d(args_type&& args) const;
        primitive_argument_type slicing1d(args_type&& args) const;
        primitive_argument_type slicing2d(args_type&& args) const;
    };

    PHYLANX_EXPORT primitive create_slicing_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif //PHYLANX_SLICING_OPERATION_1153_10242017_HPP
