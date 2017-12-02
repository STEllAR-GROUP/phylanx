//  Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/slicing_operation.hpp>
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
    phylanx::execution_tree::primitives::slicing_operation>
    slicing_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(slicing_operation_type,
    phylanx_slicing_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(slicing_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const slicing_operation::match_data =
    {
        hpx::util::make_tuple(
            "slice", "slice(_1, _2, _3, _4, _5)", &create<slicing_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    slicing_operation::slicing_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct slicing : std::enable_shared_from_this<slicing>
        {
            slicing() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

            using matrix_type = blaze::DynamicMatrix<double>;
            using submatrix_type = blaze::Submatrix<matrix_type>;

            using vector_type = blaze::DynamicVector<double>;
            using subvector_type = blaze::Subvector<vector_type>;

            primitive_result_type slicing0d(args_type && args) const
            {
                // return the input as it is if the input is of zero dimensions
                // the values passed to row_start, row_stop, col_start, col_stop
                // does not have an effect on the result.

                return primitive_result_type(std::move(args[0]));
            }

            primitive_result_type slicing1d(args_type && args) const
            {
                // return elements starting from col_start to col_stop(exclusive)
                // the values passed to row_stat and row_stop does not have an
                // effect on the result.

                auto col_start = extract_integer_value(args[3]);
                auto col_stop = extract_integer_value(args[4]);

                // parameters required by blaze to create a subvector is as follows:
                // subvector(vector,row,column,m,n)
                // vector The matrix containing the submatrix.
                // row The index of the first row of the submatrix.
                // column The index of the first column of the submatrix.
                // m The number of rows of the submatrix.
                // n The number of columns of the submatrix.
                // return View on the specific submatrix of the matrix.

                // The following math is a result of converting the arguments
                // provided in slice primitive so that equivalent operation is
                // performed in blaze.
                // vector = vector
                // column = col_start
                // n = (col_stop - col_start)

                subvector_type sv =
                    blaze::subvector(args[0].vector(),
                        col_start, (col_stop - col_start));

                return ir::node_data<double>{vector_type{std::move(sv)}};
            }

            primitive_result_type slicing2d(args_type && args) const
            {
                // returns the sliced matrix, depending upon the values
                // provided in row_start, row_stop(exclusive), col_start, col_stop(exclusive)

                // parameters required by phylanx to create a slice is as follows:
                // matrix The matrix containing the submatrix.
                // row_start The index of the first row of the submatrix.
                // row_stop The index of the last row(exclusive) of the submatrix.
                // col_start The index of the first column of the submatrix.
                // col_stop The index of the last column(exclusive) of the submatrix.

                auto row_start = extract_integer_value(args[1]);
                auto row_stop = extract_integer_value(args[2]);
                auto col_start = extract_integer_value(args[3]);
                auto col_stop = extract_integer_value(args[4]);

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
                // column = col_start
                // m = (row_stop - row_start)
                // n = (col_stop - col_start)

                submatrix_type sm =
                    blaze::submatrix(args[0].matrix(), row_start, col_start,
                        (row_stop - row_start), (col_stop - col_start));

                if ((row_stop - row_start) == 1)
                {
                    //return a vector in this case and not a matrix
                    return ir::node_data<double>{
                        std::move(blaze::trans(row(sm, 0)))};
                }
                else if ((col_stop - col_start) == 1)
                {
                    return ir::node_data<double>{std::move(column(sm, 0))};
                }
                else
                {
                    return ir::node_data<double>{matrix_type{std::move(sm)}};
                }
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 5)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "slicing_operation::slicing_operation",
                        "the slicing_operation primitive requires exactly "
                            "five arguments");
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
                        "slicing_operation::eval",
                        "the slicing_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_result_type
                    {
                        std::size_t lhs_dims = args[0].num_dimensions();
                        switch (lhs_dims)
                        {
                        case 0:
                            return this_->slicing0d(std::move(args));

                        case 1:
                            return this_->slicing1d(std::move(args));

                        case 2:
                            return this_->slicing2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "slicing_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    hpx::future<primitive_result_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::slicing>()->eval(args, noargs);
        }

        return std::make_shared<detail::slicing>()->eval(operands_, args);
    }
}}}
