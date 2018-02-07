//  Copyright (c) 2017 Bibek Wagle
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/slicing_operation.hpp>
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
    match_pattern_type const slicing_operation::match_data =
    {
        hpx::util::make_tuple("slice",
            std::vector<std::string>{"slice(_1, _2, _3, _4, _5)"},
            &create<slicing_operation>)
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

            primitive_argument_type slicing0d(args_type && args) const
            {
                // return the input as it is if the input is of zero dimensions
                // the values passed to row_start, row_stop, col_start, col_stop
                // does not have an effect on the result.

                return primitive_argument_type(std::move(args[0]));
            }

            primitive_argument_type slicing1d(args_type&& args) const
            {
                // return elements starting from col_start to col_stop(exclusive)
                // the values passed to row_stat and row_stop does not have an
                // effect on the result.

                auto col_start = args[3][0];
                auto col_stop = args[4][0];

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

                if (col_start < 0 && col_stop > 0)    // slice from the end
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "slicing_operation::slicing_operation",
                        "col_stop can not be positive if col_start is "
                            "negative");
                }

                using storage1d_type = typename arg_type::storage1d_type;
                using storage0d_type = typename arg_type::storage0d_type;

                auto arg0 = args[0].vector();

                if (col_start < 0 && col_stop <= 0)    // slice from the end
                {
                    auto sv = blaze::subvector(
                        arg0, arg0.size() + col_start, -col_start + col_stop);

                    storage1d_type v{sv};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(v)}};

                    if (sv.size() == 1)
                    {
                        return primitive_argument_type{sv[0]};
                    }
                }

                auto sv =
                    blaze::subvector(arg0, col_start, col_stop - col_start);

                if (sv.size() == 1)
                {
                    return primitive_argument_type{sv[0]};
                }

                storage1d_type v{sv};
                return primitive_argument_type{
                    ir::node_data<double>{std::move(v)}};
            }

            primitive_argument_type slicing2d(args_type&& args) const
            {
                // returns the sliced matrix, depending upon the values
                // provided in row_start, row_stop(exclusive), col_start,
                // col_stop(exclusive)

                // parameters required by phylanx to create a slice is as follows:
                // matrix The matrix containing the submatrix.
                // row_start The index of the first row of the submatrix.
                // row_stop The index of the last row(exclusive) of the submatrix.
                // col_start The index of the first column of the submatrix.
                // col_stop The index of the last column(exclusive) of the submatrix.

                auto row_start = args[1][0];
                auto row_stop = args[2][0];
                auto col_start = args[3][0];
                auto col_stop = args[4][0];

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

                if ((col_start < 0 && col_stop > 0) ||
                    (row_start < 0 && row_stop > 0))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "slicing_operation::slicing_operation",
                        "col_stop/row_stop can not be positive if "
                            "col_start/row_start is negative");
                }
                if ((col_start >= 0 && col_stop < 0) ||
                    (row_start >= 0 && row_stop < 0))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "slicing_operation::slicing_operation",
                        "col_stop/row_stop can not be negative if "
                            "col_start/row_start is positive");
                }

                using storage0d_type = typename arg_type::storage0d_type;
                using storage1d_type = typename arg_type::storage1d_type;
                using storage2d_type = typename arg_type::storage2d_type;

                auto arg0 = args[0].matrix();

                if (col_start < 0 && col_stop <= 0 && row_start >= 0 &&
                    row_stop > 0)    //column slice from the end
                {
                    auto num_cols = arg0.columns();

                    // return a vector and not a matrix if the slice contains
                    // exactly one row/column
                    if (row_stop - row_start == 1)
                    {
                        auto sv = blaze::trans(blaze::row(
                            blaze::submatrix(arg0,
                                row_start, num_cols + col_start,
                                1, -col_start + col_stop),
                            0));

                        if (sv.size() == 1)
                        {
                            return primitive_argument_type{sv[0]};
                        }

                        storage1d_type v{sv};
                        return primitive_argument_type{
                            ir::node_data<double>{std::move(v)}};
                    }
                    if (col_stop - col_start == 1)
                    {
                        auto sv = blaze::column(
                            blaze::submatrix(arg0,
                                row_start, num_cols + col_start,
                                row_stop - row_start, 1),
                            0);

                        if (sv.size() == 1)
                        {
                            return primitive_argument_type{sv[0]};
                        }

                        storage1d_type v{sv};
                        return primitive_argument_type{
                            ir::node_data<double>{std::move(v)}};
                    }

                    auto sm = blaze::submatrix(arg0,
                        row_start, num_cols + col_start,
                        row_stop - row_start, -col_start + col_stop);

                    storage2d_type m{sm};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(m)}};
                }

                if (row_start < 0 && row_stop <= 0 && col_start >= 0 &&
                    col_stop > 0)    // row slice from the end
                {
                    auto num_rows = arg0.rows();

                    // return a vector and not a matrix if the slice contains
                    // exactly one row/column
                    if (row_stop - row_start == 1)
                    {
                        auto sv = blaze::trans(blaze::row(
                            blaze::submatrix(arg0,
                                num_rows + row_start, col_start,
                                1, col_stop - col_start),
                            0));

                        if(sv.size() == 1)
                        {
                            return primitive_argument_type{sv[0]};
                        }

                        storage1d_type v{sv};
                        return primitive_argument_type{
                            ir::node_data<double>{std::move(v)}};
                    }
                    if (col_stop - col_start == 1)
                    {
                        auto sv = blaze::column(
                            blaze::submatrix(arg0,
                                num_rows + row_start, col_start,
                                -row_start + row_stop, 1),
                            0);

                        if (sv.size() == 1)
                        {
                            return primitive_argument_type{sv[0]};
                        }

                        storage1d_type v{sv};
                        return primitive_argument_type{
                            ir::node_data<double>{std::move(v)}};
                    }

                    auto sm = blaze::submatrix(arg0,
                        num_rows + row_start, col_start,
                        -row_start + row_stop, col_stop - col_start);

                    storage2d_type m{sm};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(m)}};
                }

                if (row_start < 0 && row_stop <= 0 && col_start < 0 &&
                    col_stop <= 0)    // row and column , both, slice from end
                {
                    auto num_rows = arg0.rows();
                    auto num_cols = arg0.columns();

                    // return a vector and not a matrix if the slice contains
                    // exactly one row/column
                    if (row_stop - row_start == 1)
                    {
                        auto sv = blaze::trans(blaze::row(
                            blaze::submatrix(arg0,
                                num_rows + row_start, num_cols + col_start,
                                1, -col_start + col_stop),
                            0));

                        if (sv.size() == 1)
                        {
                            return primitive_argument_type{sv[0]};
                        }

                        storage1d_type v{sv};
                        return primitive_argument_type{
                            ir::node_data<double>{std::move(v)}};
                    }
                    if (col_stop - col_start == 1)
                    {
                        auto sv = blaze::column(
                            blaze::submatrix(arg0,
                                num_rows + row_start, num_cols + col_start,
                                -row_start + row_stop, 1),
                            0);

                        if (sv.size() == 1)
                        {
                            return primitive_argument_type{sv[0]};
                        }

                        storage1d_type v{sv};
                        return primitive_argument_type{
                            ir::node_data<double>{std::move(v)}};
                    }

                    auto sm = blaze::submatrix(arg0,
                        num_rows + row_start, num_cols + col_start,
                        -row_start + row_stop, -col_start + col_stop);

                    storage2d_type m{sm};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(m)}};
                }

                // return a vector and not a matrix if the slice contains
                // exactly one row/column
                if (row_stop - row_start == 1)
                {
                    auto sv = blaze::trans(blaze::row(
                        blaze::submatrix(arg0,
                            row_start, col_start,
                            1, col_stop - col_start),
                        0));

                    if (sv.size() == 1)
                    {
                        return primitive_argument_type{sv[0]};
                    }

                    storage1d_type v{sv};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(v)}};
                }
                if (col_stop - col_start == 1)
                {
                    auto sv = blaze::column(
                        blaze::submatrix(arg0,
                            row_start, col_start,
                            row_stop - row_start, 1),
                        0);

                    if (sv.size() == 1)
                    {
                        return primitive_argument_type{sv[0]};
                    }

                    storage1d_type v{sv};
                    return primitive_argument_type{
                        ir::node_data<double>{std::move(v)}};
                }

                auto sm = blaze::submatrix(arg0,
                    row_start, col_start,
                    row_stop - row_start, col_stop - col_start);

                storage2d_type m{sm};
                return primitive_argument_type{
                    ir::node_data<double>{std::move(m)}};
            }

        public:
            hpx::future<primitive_argument_type> eval(
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
                    [this_](args_type&& args) -> primitive_argument_type
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
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> slicing_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::slicing>()->eval(args, noargs);
        }

        return std::make_shared<detail::slicing>()->eval(operands_, args);
    }
}}}
