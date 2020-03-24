// Copyright (c) 2020 Rory Hector
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/gauss_inverse.hpp>
#include <phylanx/util/detail/bad_swap.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/lcos/future.hpp>

#include <hpx/parallel/algorithms/for_each.hpp>
#include <hpx/parallel/execution_policy.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include<tuple>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

// This is just boilerplate
namespace phylanx { namespace execution_tree { namespace primitives {
    // Declare the help_string details, just for explaining what
    // a primitive is doing, if needed.
    constexpr char const* const help_string = R"(
        invGJE(matrix)
        Args:
            matrix (vector<vector<double>>)
        Returns:
            the inverse of the matrix
    )";

    // For the plugin file, you need to tell Phylanx these things about
    // your primtiive in a tuple
    // (1) name of the primitive, e.g. invGJE
    // (2) a description of how the primitive can be invoked in the compiler
    //        _k means the primitive should be invoked with k arguments
    // (3) a creation function (defined in header)
    // (4) a connection between the name and the type (always the same)
    // (5) describes the usage of the primitive
    match_pattern_type const matrix_GJE_Inverse::match_data = {
        hpx::util::make_tuple("invGJE", std::vector<std::string>{"invGJE(_1)"},
            &create_matrix_GJE_Inverse, &create_primitive<matrix_GJE_Inverse>,
            help_string)};

    // Pretty much boilerplate, just change the name, or may need to use
    // std::vector<primitive_arguments_type>&& for multiple args.
    // Passes the arguments along to the constructor of the base class.
    // Stores the three arguments as members.
    // Name = name of primitive instance, Codename = file used to generate it
    matrix_GJE_Inverse::matrix_GJE_Inverse(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    // This is where the computation of the inverse should be performed.
    template <typename T>
    primitive_argument_type matrix_GJE_Inverse::gaussInverse2d(
        ir::node_data<T>&& op) const
    {
        if (op.dimension(0) != op.dimension(1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "inverse::gaussInverse2d",
                generate_error_message(
                    "matrices to invert have to be quadratic"));
        }
        // Do the standard calculations here

        // Get necessary data about input for calculation
        blaze::DynamicMatrix<double> myMatrix = op.matrix();
        std::size_t n = myMatrix.rows();

        // Definition of a nxn double row-major identity matrix
        blaze::DynamicMatrix<double> invMatrix = blaze::IdentityMatrix<double>(n);

        // For swapping rows back into place
        std::vector<std::tuple<std::size_t, std::size_t>> swappedRows;

        // Do gaussian elimination to get upper triangular
        // matrix with 1's across diagonal
        for (std::int64_t current_row = 0; current_row < n; current_row++)
        {
            // Swaps current row with nearest subsequent row such that
            // after swapping A[i][i] != 0.
            if (myMatrix(current_row, current_row) == 0)    // if A[i][i] = 0,
            {
                bool rowFound = false;
                std::size_t checkOffset = 1;
                while (current_row + checkOffset < n && !rowFound)
                {
                    if (myMatrix((current_row + checkOffset), current_row) != 0)
                    {
                        swap(column(myMatrix, current_row),
                            column(myMatrix, (current_row + checkOffset) % n));
                        swappedRows.emplace_back(
                            std::tuple<std::size_t, std::size_t>(
                                 current_row,
                                 (current_row + checkOffset) % n));
                        rowFound = true;
                    }
                    else
                        checkOffset++;
                }

                // swap row with nearest subsequent row such that after
                // swapping A[i][i] != 0
                // if fails, inverse does not exist
                if (!rowFound)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "gauss_inverse::eval",
                        generate_error_message("inverse does not exist"));
                }
                current_row--;    // After swapping, make sure to retry this row
            }
            else    // the inversion has not already failed
            {
                double scale = myMatrix(current_row, current_row);
                for (std::int64_t col = 0; col < n; col++)
                {
                    myMatrix(current_row, col) =
                        myMatrix(current_row, col) / scale;
                    invMatrix(current_row, col) =
                        invMatrix(current_row, col) / scale;
                }
                if (current_row < n - 1)
                {
                    for (std::int64_t nextRow = current_row + 1; nextRow < n;
                         nextRow++)
                    {
                        double factor = myMatrix(nextRow, current_row);
                        for (std::int64_t nextCol = 0; nextCol < n; nextCol++)
                        {
                            myMatrix(nextRow, nextCol) =
                                myMatrix(nextRow, nextCol) -
                                (factor * myMatrix(current_row, nextCol));
                            invMatrix(nextRow, nextCol) =
                                invMatrix(nextRow, nextCol) -
                                (factor * invMatrix(current_row, nextCol));
                        }
                    }
                }
            }
        }

        // Back substitution phase, going from bottom to top
        // in matrix zeroing out columns except diagonal
        for (std::int64_t zeroCol = n - 1; zeroCol > 0; zeroCol--)
        {
            for (std::int64_t row = zeroCol - 1; row >= 0; row--)
            {
                double factor = myMatrix(row, zeroCol);
                for (std::int64_t col = 0; col < n; col++)
                {
                    myMatrix(row, col) =
                        myMatrix(row, col) - (factor * myMatrix(zeroCol, col));
                    invMatrix(row, col) = invMatrix(row, col) -
                        (factor * invMatrix(zeroCol, col));
                }
            }
        }

        // Swap rows back into place
        for (std::int64_t i = swappedRows.size()-1; i >= 0; i--)
        {
            swap(row(invMatrix, std::get<0>(swappedRows[i])),
                row(invMatrix, std::get<1>(swappedRows[i])));
        }

        return primitive_argument_type{std::move(invMatrix)};
    }

    // It seems that this takes the primitive argument type,
    // extracts the actual type from it, e.g. 2D double vector
    // and sends it off to the function that actually does the
    // computation.
    primitive_argument_type matrix_GJE_Inverse::gaussInverse2d(
        primitive_argument_type&& op) const
    {
        switch (extract_common_type(op))
        {
        case node_data_type_double:
            return gaussInverse2d(
                extract_numeric_value_strict(std::move(op), name_, codename_));
        case node_data_type_bool:
        case node_data_type_int64:
        case node_data_type_unknown:
            return gaussInverse2d(
                extract_numeric_value(std::move(op), name_, codename_));
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "inverse_operation::gaussInverse2d",
            generate_error_message(
                "the inverse primitive requires for all arguments to "
                "be numeric data types"));
    }

    // Call the evaluation function
    // Petty much boilerplate, just gotta change names and stuff
    // if the constructor was called without an operand, just pass back
    // the args to the function, otherwise, pass the operands
    hpx::future<primitive_argument_type> matrix_GJE_Inverse::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // Check to make sure there is exactly one thing to invert
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "gauss_inverse::eval",
                generate_error_message(
                    "the gaussian inverse operation primitive requires"
                    "exactly one operand"));
        }

        // Check if there are no valid operands
        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "gauss_inverse::eval",
                generate_error_message(
                    "the gaussian_inverse_operation primitive requires that "
                    "the arguments given by the operands array is valid"));
        }

        // Get a future to the result of the actual computation
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    primitive_argument_type&& op) -> primitive_argument_type {
                    if ((extract_numeric_value_dimension(
                            op, this_->name_, this_->codename_)) == 2)
                    {
                        return this_->gaussInverse2d(std::move(op));
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "gauss_inverse::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}    // namespace phylanx::execution_tree::primitives
