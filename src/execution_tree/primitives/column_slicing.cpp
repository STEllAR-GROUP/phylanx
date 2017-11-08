// Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/column_slicing.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
        phylanx::execution_tree::primitives::column_slicing_operation>
        column_slicing_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
        column_slicing_operation_type, phylanx_column_slicing_operation_component,
        "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(column_slicing_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const column_slicing_operation::match_data =
    {
        hpx::util::make_tuple(
            "slice_column", "slice_column(_1, _2, _3)",
            &create<column_slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    column_slicing_operation::column_slicing_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct slicing_column : std::enable_shared_from_this<slicing_column>
        {
            slicing_column() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

            using matrix_type = blaze::DynamicMatrix<double>;
            using submatrix_type = blaze::Submatrix<matrix_type>;

            using vector_type = blaze::DynamicVector<double>;
            using subvector_type = blaze::Subvector<vector_type>;

            primitive_result_type column_slicing0d(args_type && args) const
            {
                // return the input as it is if the input is of zero dimension or
                // one dimension. The values passed to col_start, col_stop
                // does not have an effect on the result.

                return primitive_result_type(std::move(args[0]));
            }

            primitive_result_type column_slicing1d(args_type && args) const
            {
                // return elements starting from col_start to col_stop

                auto col_start  = extract_integer_value(args[1]);
                auto col_stop = extract_integer_value(args[2]);

                // parameters required by blaze to create a submatrix is as follows:
                // subvector(vector,column,n)
                // vector The vector containing the subvector.
                // column The index of the first column of the subvector.
                // n The number of columns of the subvector.
                // return View on the specific subvector of the vector.

                // The following math is a result of converting the arguments
                // provided in slice primitive so that equivalent operation is
                // performed in blaze.
                // vector = vector
                // column = col_start
                // n = (col_stop - col_start)+1

                subvector_type sv =
                    blaze::subvector(args[0].vector(),
                        col_start, (col_stop - col_start) + 1);

                return primitive_result_type(vector_type(std::move(sv)));
            }

            primitive_result_type column_slicing2d(args_type && args) const
            {
                // returns the sliced matrix, depending upon the values
                // provided in col_start, col_stop.

                // parameters required by phylanx to create a slice is as follows:
                // matrix The matrix containing the submatrix.
                // col_start The index of the first column of the submatrix.
                // col_stop The index of the last column of the submatrix.

                auto col_start = extract_integer_value(args[1]);
                auto col_stop = extract_integer_value(args[2]);
                auto num_matrix_rows = args[0].dimensions()[0];

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
                // row = 0
                // column = col_start
                // m = number of rows in the input matrix
                // n = (col_stop - col_start)+1

                submatrix_type sm =
                    blaze::submatrix(args[0].matrix(),
                        0, col_start,
                        num_matrix_rows, (col_stop - col_start) + 1);

                return primitive_result_type(matrix_type(std::move(sm)));
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
                            "column_slicing_operation::column_slicing_operation",
                        "the column_slicing_operation primitive requires exactly "
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
                        "column_slicing_operation::eval",
                        "the column_slicing_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_result_type
                    {
                        std::size_t matrix_dims = args[0].num_dimensions();
                        switch (matrix_dims)
                        {
                        case 0:
                            return this_->column_slicing0d(std::move(args));

                        case 1:
                            return this_->column_slicing1d(std::move(args));

                        case 2:
                            return this_->column_slicing2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "column_slicing_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args));
            }
        };
    }

    hpx::future<primitive_result_type> column_slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::slicing_column>()->eval(args, noargs);
        }

        return std::make_shared<detail::slicing_column>()->eval(operands_, args);
    }
}}}
