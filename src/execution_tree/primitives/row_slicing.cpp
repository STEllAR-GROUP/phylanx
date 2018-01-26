//  Copyright (c) 2017 Bibek Wagle
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/row_slicing.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::row_slicing_operation>
    row_slicing_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(row_slicing_operation_type,
    phylanx_row_slicing_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(row_slicing_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const row_slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice_row",
            std::vector<std::string>{"slice_row(_1, _2, _3)"},
            &create<row_slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    row_slicing_operation::row_slicing_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct slicing_row : std::enable_shared_from_this<slicing_row>
        {
            slicing_row() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

            primitive_result_type row_slicing0or1d(args_type && args) const
            {
                // return the input as it is if the input is of zero dimension or
                // one dimension. The values passed to row_start, row_stop
                // does not have an effect on the result.

                return primitive_result_type{std::move(args[0])};
            }

            primitive_result_type row_slicing2d(args_type && args) const
            {
                // returns the sliced matrix, depending upon the values
                // provided in row_start, row_stop(exclusive)

                // parameters required by phylanx to create a slice is as follows:
                // matrix The matrix containing the submatrix.
                // row_start The index of the first row of the submatrix.
                // row_stop The index of the last row(exclusive) of the submatrix.

                auto row_start = extract_integer_value(args[1]);
                auto row_stop = extract_integer_value(args[2]);
                auto num_matrix_cols = args[0].dimensions()[1];

                // parameters required by blaze to create a submatrix is as follows:
                // submatrix(matrix,row,column,m,n)
                // matrix The matrix containing the submatrix.
                // row The index of the first row of the submatrix.
                // column The index of the first column of the submatrix.
                // m The number of rows of the submatrix.
                // n The number of columns of the submatrix.
                // return View on the specific submatrix of the matrix.

                // The following math is a result of converting the arguments
                // provided in slice primitive so that equivalent operation is
                // performed in blaze.
                // matrix = matrix
                // row = row_start
                // column = 0
                // m = (row_stop - row_start)
                // n = number of columns in the input matrix

                if (row_start < 0 && row_stop > 0)    // row slice from the end
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "row_slicing_operation::row_slicing_operation",
                        "row_stop can not be positive if row_start is negative");
                }
                if (row_start >= 0 && row_stop < 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "row_slicing_operation::row_slicing_operation",
                        "row_stop can not be negative if row_start is positive");
                }

                using storage1d_type = typename arg_type::storage1d_type;
                using storage2d_type = typename arg_type::storage2d_type;

                auto arg0 = args[0].matrix();

                if (row_start < 0 && row_stop <= 0)
                {
                    auto num_rows = arg0.rows();

                    // return a vector and not a matrix if the slice contains
                    // exactly one row
                    if (row_stop - row_start == 1)
                    {
                        auto sv = blaze::trans(blaze::row(
                            blaze::submatrix(arg0,
                                num_rows + row_start, 0,
                                1, num_matrix_cols),
                            0));

                        storage1d_type v{sv};
                        return ir::node_data<double>{std::move(v)};
                    }

                    auto sm = blaze::submatrix(arg0,
                        num_rows + row_start, 0,
                        -row_start + row_stop, num_matrix_cols);

                    storage2d_type m{sm};
                    return ir::node_data<double>{std::move(m)};
                }

                // return a vector and not a matrix if the slice contains
                // exactly one row
                if (row_stop - row_start == 1)
                {
                    auto sv = blaze::trans(blaze::row(
                        blaze::submatrix(arg0,
                            row_start, 0,
                            1, num_matrix_cols),
                        0));

                    storage1d_type v{sv};
                    return ir::node_data<double>{std::move(v)};
                }

                auto sm = blaze::submatrix(arg0,
                    row_start, 0,
                    row_stop - row_start, num_matrix_cols);

                storage2d_type m{sm};
                return ir::node_data<double>{std::move(m)};
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "row_slicing_operation::row_slicing_operation",
                        "the row_slicing_operation primitive requires exactly "
                            "three arguments");
                }

                bool arguments_valid = true;
                for (std::size_t i = 0; i != operands.size(); ++i)
                {
                    if (!valid(operands[i]))
                    {
                        arguments_valid = false;
                    }
                }

                if (!arguments_valid)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "row_slicing_operation::eval",
                        "the row_slicing_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_result_type
                    {
                        std::size_t matrix_dims = args[0].num_dimensions();
                        switch (matrix_dims)
                        {
                        case 0: HPX_FALLTHROUGH;
                        case 1:
                            return this_->row_slicing0or1d(std::move(args));

                        case 2:
                            return this_->row_slicing2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "row_slicing_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    hpx::future<primitive_result_type> row_slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::slicing_row>()->eval(args, noargs);
        }

        return std::make_shared<detail::slicing_row>()->eval(operands_, args);
    }
}}}
