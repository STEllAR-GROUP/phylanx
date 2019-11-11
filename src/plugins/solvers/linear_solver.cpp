// Copyright (c) 2018 Shahrzad Shirzad
//               2018 Patrick Diehl
//               2019 Nanmiao Wu
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/solvers/linear_solver.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cstdint>

#include <blaze/Math.h>

#ifdef PHYLANX_HAVE_BLAZE_ITERATIVE
#include <BlazeIterative.hpp>
#endif


///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
///////////////////////////////////////////////////////////////////////////////
#define PHYLANX_LIN_MATCH_DATA(name,doc)                                       \
    match_pattern_type{name,                                                   \
        std::vector<std::string>{name "(_1, _2)", name "(_1, _2, _3)"},        \
        &create_linear_solver, &create_primitive<linear_solver>, doc}          \
    /**/

    std::vector<match_pattern_type> const linear_solver::match_data = {
        PHYLANX_LIN_MATCH_DATA("linear_solver_lu",
        R"(a, b
        Args:

            a (matrix) : a matrix
            b (vector) : a vector

        Returns:

        A matrix `x` such that `a x = b`.)"
        ),
        PHYLANX_LIN_MATCH_DATA("linear_solver_ldlt",
        R"(a, b, uplo
        Args:

            a (matrix) : a matrix
            b (vector) : a vector
            uplo (string) : either 'L' or 'U'

        Returns:

        A matrix `x` such that `a x = b`. If uplo = 'L', solve
        `a` as a lower triangular matrix, otherwise as an upper
        triangular matrix.)"
        ),
        PHYLANX_LIN_MATCH_DATA("linear_solver_cholesky",
        R"(a, b,uplo
        Args:

            a (matrix) : a matrix
            b (vector) : a vector
            uplo (string) : either 'L' or 'U'

        Returns:

        A matrix `x` such that `a x = b`, solved using the
        Cholesky (LLH) decomposition. If uplo = 'L', solve
        `a` as a lower triangular matrix, otherwise as an upper
        triangular matrix.)"
        ),
#ifdef PHYLANX_HAVE_BLAZE_ITERATIVE
        PHYLANX_LIN_MATCH_DATA("iterative_solver_conjugate_gradient",
         R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         conjugate gradient)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_bicgstab",
         R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         BiCGSTAB solver.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_bicgstab_lu",
         R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         BiCGSTAB solver utilizing the LU decomposition as
         its preconditioner.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_bicgstab_rq",
         R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         BiCGSTAB solver utilizing the RQ decomposition as
         its preconditioner.)"
         ),
         PHYLANX_LIN_MATCH_DATA("iterative_solver_bicgstab_qr",
         R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         BiCGSTAB solver utilizing the QR decomposition as
         its preconditioner.)"
         ),
         PHYLANX_LIN_MATCH_DATA("iterative_solver_bicgstab_cholesky",
         R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         BiCGSTAB solver utilizing the Cholesky decomposition as
         its preconditioner.)"
         ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_cg_jacobi",
                               R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         CG solver utilizing the Jacobi decomposition as
         its preconditioner.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_cg_ssor",
                               R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         CG solver utilizing the SSOR decomposition as
         its preconditioner.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_cg_incompleteCholesky",
                               R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         CG solver utilizing the incomplete Cholesky factorization as
         its preconditioner.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_cg_symmetric_gauss_seidel",
                               R"(a, b
         Args:

             a (matrix) : a matrix
             b (vector) : a vector

         Returns:

         A matrix `x` such that `a x = b`, solved using the
         CG solver utilizing the Symmetric Gauss Seidel as
         its preconditioner.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_lanczos",
                               R"(a, b, n, x
         Args:

             a (matrix) : a matrix
             b (vector) : a vector
             n (scalar) : a scalar
             x (vector) : a vector

         Returns:

         A vector of eigenvalues approximate the matrix `a`, whose dimension is decided
         by n, solved using the Lanczos solver.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_arnoldi",
                               R"(a, b, n, x
         Args:

             a (matrix) : a matrix
             b (vector) : a vector
             n (scalar) : a scalar
             x (vector) : a vector

         Returns:

         A vector of eigenvalues approximate the matrix `a`, whose dimension is decided
         by n, solved using the Arnoldi solver.)"
        ),
        PHYLANX_LIN_MATCH_DATA("iterative_solver_gmres",
                               R"(a, b, n, x
         Args:

             a (matrix) : a matrix
             b (vector) : a vector
             n (scalar) : a scalar
             x (vector) : a vector

         Returns:

         A vector of eigenvalues approximate the matrix `a`, solved
         using the GMRES solver.)"
        )
#endif
    };

#undef PHYLANX_LIN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    linear_solver::vector_function_ptr linear_solver::get_lin_solver_map(
        std::string const& name) const
    {
        static std::map<std::string, vector_function_ptr> lin_solver = {
            {"linear_solver_lu",
                // Linear solver based on LU decomposition of a general square matrix
                [](args_type&& args) -> arg_type {
                    storage2d_type A{blaze::trans(args[0].matrix())};
                    storage1d_type b{args[1].vector()};
                    const std::unique_ptr<int[]> ipiv(new int[b.size()]);
                    blaze::gesv(A, b, ipiv.get());
                    return arg_type{std::move(b)};
                }},
            {"linear_solver_ldlt",
                // Linear solver based on LDLT decomposition of a square
                // symmetric indefinite matrix
                [](args_type&& args) -> arg_type {
                    storage2d_type A{blaze::trans(args[0].matrix())};
                    storage1d_type b{args[1].vector()};
                    const std::unique_ptr<int[]> ipiv(new int[b.size()]);
                    blaze::sysv(A, b, 'U', ipiv.get());
                    return arg_type{std::move(b)};
                }},
            {"linear_solver_cholesky",
                // Linear solver based on cholesky(LLH) decomposition of a
                // square positive definite matrix
                [](args_type&& args) -> arg_type {
                    storage2d_type A{blaze::trans(args[0].matrix())};
                    storage1d_type b{args[1].vector()};
                    blaze::posv(A, b, 'U');
                    return arg_type{std::move(b)};
                }},
#ifdef PHYLANX_HAVE_BLAZE_ITERATIVE
            {"iterative_solver_conjugate_gradient",
                // Iterative conjugate gradient solver
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::ConjugateGradientTag tag;
                    b = blaze::iterative::solve(A, b, tag);
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_bicgstab",
                // Iterative BiCGSTAB solver
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::BiCGSTABTag tag;
                    b = blaze::iterative::solve(A, b, tag);
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_bicgstab_lu",
                // Iterative Precondition BiCGSTAB solver with LU preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionBiCGSTABTag tag;
                    b = blaze::iterative::solve(A, b, tag, "LU");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_bicgstab_rq",
                // Iterative Precondition BiCGSTAB solver with RQ preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionBiCGSTABTag tag;
                    b = blaze::iterative::solve(A, b, tag, "RQ");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_bicgstab_qr",
                // Iterative Precondition BiCGSTAB solver with QR preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionBiCGSTABTag tag;
                    b = blaze::iterative::solve(A, b, tag, "QR");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_bicgstab_cholesky",
                // Iterative Precondition BiCGSTAB solver with Cholesky preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionBiCGSTABTag tag;
                    b = blaze::iterative::solve(A, b, tag, "Cholesky");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_cg_jacobi",
                // Iterative Precondition CG solver with Jacobi preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionCGTag tag;
                    b = blaze::iterative::solve(A, b, tag, "Jacobi");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_cg_ssor",
                // Iterative Precondition CG solver with ssor preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionCGTag tag;
                    b = blaze::iterative::solve(A, b, tag, "SSOR");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_cg_incompleteCholesky",
                // Iterative Precondition CG solver with incomplete Cholesky factorization
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionCGTag tag;
                    b = blaze::iterative::solve(
                        A, b, tag, "incomplete_Cholesky");
                    return arg_type{std::move(b)};
                }},
            {"iterative_solver_cg_symmetric_gauss_seidel",
                // Iterative Precondition CG solver with Symmetric Gauss Seidel
                // preconditioner
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](args_type&& args) -> arg_type {
                    storage2d_type A{args[0].matrix()};
                    storage1d_type b{args[1].vector()};
                    blaze::iterative::PreconditionCGTag tag;
                    b = blaze::iterative::solve(
                        A, b, tag, "Symmetric_Gauss_Seidel");
                    return arg_type{std::move(b)};
                }}
#endif
        };
        return lin_solver[name];
    }

    linear_solver::vector_function_ptr_uln
    linear_solver::get_lin_solver_map_uln(std::string const& name) const
    {
        static std::map<std::string, vector_function_ptr_uln> lin_solver = {
            {"linear_solver_ldlt",
                // Linear solver based on LDLT decomposition of a symmetric
                // indefinite matrix
                [](arg_type&& arg_0, arg_type&& arg_1,
                    primitive_argument_type&& arg_2) -> arg_type {
                    std::string ul = extract_string_value_strict(arg_2);
                    if (ul != "L" && ul != "U")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linear_solver::eval",
                            util::generate_error_message(
                                "the linear_solver primitive requires for "
                                "the third argument to be either 'L' or 'U'"));
                    }
                    storage2d_type A{blaze::trans(arg_0.matrix())};
                    storage1d_type b{arg_1.vector()};
                    const std::unique_ptr<int[]> ipiv(new int[b.size()]);
                    blaze::sysv(A, b, ul == "L" ? 'L' : 'U', ipiv.get());
                    return arg_type{std::move(b)};
                }},
            {"linear_solver_cholesky",
                // Linear solver based on cholesky(LLH) decomposition of a
                // positive definite matrix
                [](arg_type&& arg_0, arg_type&& arg_1,
                    primitive_argument_type&& arg_2) -> arg_type {
                    std::string ul =
                        extract_string_value_strict(std::move(arg_2));
                    if (ul != "L" && ul != "U")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linear_solver::eval",
                            util::generate_error_message(
                                "the linear_solver primitive requires for "
                                "the third argument to be either 'L' or 'U'"));
                    }
                    storage2d_type A{blaze::trans(arg_0.matrix())};
                    storage1d_type b{arg_1.vector()};
                    blaze::posv(A, b, ul == "L" ? 'L' : 'U');
                    return arg_type{std::move(b)};
                }},
#ifdef PHYLANX_HAVE_BLAZE_ITERATIVE
            {"iterative_solver_lanczos",
                // Iterative Lanczos solver for eigenvalues
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](arg_type&& arg_0, arg_type&& arg_1,
                    primitive_argument_type&& arg_2) -> arg_type {
                    std::int64_t n =
                        extract_scalar_integer_value_strict(std::move(arg_2));
                    storage2d_type A{arg_0.matrix()};
                    storage1d_type b{arg_1.vector()};
                    storage1d_type x;
                    blaze::iterative::LanczosTag tag;
                    x = blaze::iterative::solve(A, b, tag, n);
                    return arg_type{std::move(x)};
                }},
            {"iterative_solver_arnoldi",
                // Iterative Arnoldi solver for eigenvalues
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](arg_type&& arg_0, arg_type&& arg_1,
                    primitive_argument_type&& arg_2) -> arg_type {
                    std::int64_t n =
                        extract_scalar_integer_value_strict(std::move(arg_2));
                    storage2d_type A{arg_0.matrix()};
                    storage1d_type b{arg_1.vector()};
                    storage1d_type x;
                    blaze::iterative::ArnoldiTag tag;
                    x = blaze::iterative::solve(A, b, tag, n);
                    return arg_type{std::move(x)};
                }},
            {"iterative_solver_gmres",
                // Iterative GMRES solver
                // Note: Relies on BlazeIterative library and
                // need to be explicitly enabled
                [](arg_type&& arg_0, arg_type&& arg_1,
                    primitive_argument_type&& arg_2) -> arg_type {
                    std::int64_t n =
                        extract_scalar_integer_value_strict(std::move(arg_2));
                    storage2d_type A{arg_0.matrix()};
                    storage1d_type b{arg_1.vector()};
                    storage1d_type x;
                    blaze::iterative::GMRESTag tag;
                    x = blaze::iterative::solve(A, b, tag, n);
                    return arg_type{std::move(x)};
                }}
#endif
        };
        return lin_solver[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    linear_solver::linear_solver(primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        std::string func_name = extract_function_name(name);

        func_ = get_lin_solver_map(func_name);
        func_uln_ = get_lin_solver_map_uln(func_name);

        HPX_ASSERT(func_ != nullptr || func_uln_ != nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type linear_solver::calculate_linear_solver(
        args_type&& op) const
    {
        if (func_ == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver::eval",
                generate_error_message(
                    "this linear_solver primitive requires exactly three "
                    "operands"));
        }
        return primitive_argument_type{func_(std::move(op))};
    }

    primitive_argument_type linear_solver::calculate_linear_solver(
        arg_type && lhs, arg_type && rhs, primitive_argument_type&& uln) const
    {
        if (func_uln_ == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver::eval",
                util::generate_error_message(
                    "this linear_solver primitive requires exactly two operands",
                    name_, codename_));
        }

            return primitive_argument_type{
                    func_uln_(std::move(lhs), std::move(rhs), std::move(uln))};
    }

    hpx::future<primitive_argument_type> linear_solver::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver::eval",
                generate_error_message("the linear_solver primitive requires "
                                       "exactly two operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]) ||
            (operands.size() == 3 && !valid(operands[2])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "linear_solver_operation::eval",
                generate_error_message(
                    "the linear_solver primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 3)
        {
            return hpx::dataflow(hpx::launch::sync,
                hpx::util::unwrapping([this_ = std::move(this_)](arg_type&& lhs,
                                          arg_type&& rhs,
                                          primitive_argument_type&& uln)
                                          -> primitive_argument_type {
                    if (lhs.num_dimensions() != 2 || rhs.num_dimensions() != 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linear_solver::eval",
                            this_->generate_error_message(
                                "the linear_solver primitive requires "
                                "that first operand to be a matrix and "
                                "the second operand to be a vector"));
                    }

                    return this_->calculate_linear_solver(
                        std::move(lhs), std::move(rhs), std::move(uln));
                }),
                numeric_operand(operands[0], args, name_, codename_, ctx),
                numeric_operand(operands[1], args, name_, codename_, ctx),
                value_operand(operands[2], args, name_, codename_, ctx));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](args_type&& args)
            -> primitive_argument_type
            {
                if (args[0].num_dimensions() != 2 ||
                    args[1].num_dimensions() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "linear_solver_operation::eval",
                        util::generate_error_message(
                            "the linear_solver_operation primitive "
                            "requires "
                            "that first operand to be a matrix and "
                            "the second "
                            "operand to be a vector",
                            this_->name_, this_->codename_));
                }

                return this_->calculate_linear_solver(std::move(args));
            }),
            detail::map_operands(operands, functional::numeric_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

