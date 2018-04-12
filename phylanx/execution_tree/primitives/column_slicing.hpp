// Copyright (c) 2017 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined PHYLANX_COLUMN_SLICING_HPP
#define PHYLANX_COLUMN_SLICING_HPP

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
    class column_slicing_operation
        : public primitive_component_base
        , public std::enable_shared_from_this<column_slicing_operation>
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

    private:
        std::vector<int> create_list(
            int start, int stop, int step, int array_length) const;
        primitive_argument_type column_slicing0d(arg_type&& arg) const;
        primitive_argument_type column_slicing1d(
            arg_type&& arg, std::vector<double> extracted) const;
        primitive_argument_type column_slicing2d(
            arg_type&& arg, std::vector<double> extracted) const;
    };

    PHYLANX_EXPORT primitive create_column_slicing_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name = "", std::string const& codename = "");
}}}

#endif //PHYLANX_COLUMN_SLICING_HPP
