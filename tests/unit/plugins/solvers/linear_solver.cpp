// Copyright (c) 2017-2018 Shahrzad Shirzad
//               2018 Patrick Diehl
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <cstdint>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
void test_linear_solver_lu_PhySL()
{
    std::string const code = R"(block(
        define(a, [[3,1,-1],[2,-1,1],[-1,3,-2]]),
        define(b, [2, 3, -1]),
        linear_solver_lu(a, b))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_ldlt_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        linear_solver_ldlt(a, b))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_ldlt_l_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        linear_solver_ldlt(a, b, "L"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_ldlt_u_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        linear_solver_ldlt(a, b, "U"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cholesky_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        linear_solver_cholesky(a, b))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cholesky_l_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        linear_solver_cholesky(a, b, "L"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cholesky_u_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        linear_solver_cholesky(a, b, "U"))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cg_jacobi_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        iterative_solver_cg_jacobi(a, b))
    )";

    auto result =
            phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
                phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cg_ssor_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        iterative_solver_cg_ssor(a, b))
    )";

    auto result =
            phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
                phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cg_incompleteCholesky_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        iterative_solver_cg_incompleteCholesky(a, b))
    )";

    auto result =
            phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
                phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_cg_symmetric_gauss_seidel_PhySL()
{
    std::string const code = R"(block(
        define(a, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
        define(b, [0, 0, 1]),
        iterative_solver_cg_symmetric_gauss_seidel(a, b))
    )";

    auto result =
            phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
                phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_lanczos_PhySL()
{
    std::string const code = R"(block(
         define(a, [[0, 0, 0, 0, 0, 0],[0, 1, 0, 0, 0, 0],[0, 0, 2, 0, 0, 0],
              [0, 0, 0, 3, 0, 0],[0, 0, 0, 0, 4, 0],[0, 0, 0, 0, 0, 100000]]),
        define(b, [1, 1, 1, 1, 1, 1]),
        define(n, 6),
        iterative_solver_lanczos(a, b, n))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{100000, 0, 4, 1, 2, 3}));
}

void test_linear_solver_arnoldi_PhySL()
{
    std::string const code = R"(block(
        define(a, [[0, 0, 0, 0, 0, 0],[0, 1, 0, 0, 0, 0],[0, 0, 2, 0, 0, 0],
              [0, 0, 0, 3, 0, 0],[0, 0, 0, 0, 4, 0],[0, 0, 0, 0, 0, 100000]]),
        define(b, [1, 1, 1, 1, 1, 1]),
        define(n, 6),
        iterative_solver_arnoldi(a, b, n))
    )";

    auto result =
            phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
                phylanx::ir::node_data<double>(
                        blaze::DynamicVector<double>{100000, 0, 4, 1, 2, 3}));
}

void test_linear_solver_gmres_PhySL()
{
    std::string const code = R"(block(
        define(a, [[0.46, 0.60, 0.74, 0.61, 0.85],[0.56, 0.31, 0.80, 0.94, 0.76],
              [0.41, 0.19, 0.15, 0.33, 0.06],[0.03, 0.92, 0.15, 0.56, 0.08],
              [0.09, 0.06, 0.69, 0.42, 0.96]]),
        define(b, [1.788, 1.891, 0.458, 0.818, 1.53]),
        define(n, 5),
        iterative_solver_gmres(a, b, n))
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result,
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{0.1, 0.3, 0.5, 0.7, 0.9}));
}

void test_linear_solver_lu(std::string const& func_name)
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicMatrix<double>{
                {3, 1, -1}, {2, -1, 1}, {-1, 3, -2}}});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{
                blaze::DynamicVector<double>{2, 3, -1}});

    phylanx::execution_tree::primitive linear_solver =
        phylanx::execution_tree::primitives::create_linear_solver(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linear_solver.eval();
    HPX_TEST_EQ(f.get(),
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver(std::string const& func_name)
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicMatrix<double>{
                {2, -1, 0}, {-1, 2, -1}, {0, -1, 1}}});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{
                blaze::DynamicVector<double>{0, 0, 1}});

    phylanx::execution_tree::primitive linear_solver =
        phylanx::execution_tree::primitives::create_linear_solver(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linear_solver.eval();
    HPX_TEST_EQ(f.get(),
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_l(std::string const& func_name)
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicMatrix<double>{
                {2, -1, 0}, {-1, 2, -1}, {0, -1, 1}}});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{
                blaze::DynamicVector<double>{0, 0, 1}});

    phylanx::execution_tree::primitive ul =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), std::string("L"));

    phylanx::execution_tree::primitive linear_solver =
        phylanx::execution_tree::primitives::create_linear_solver(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs), std::move(ul)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linear_solver.eval();
    HPX_TEST_EQ(f.get(),
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_u(std::string const& func_name)
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicMatrix<double>{
                {2, -1, 0}, {-1, 2, -1}, {0, -1, 1}}});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{
                blaze::DynamicVector<double>{0, 0, 1}});

    phylanx::execution_tree::primitive ul =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), std::string("U"));

    phylanx::execution_tree::primitive linear_solver =
        phylanx::execution_tree::primitives::create_linear_solver(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs), std::move(ul)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linear_solver.eval();
    HPX_TEST_EQ(f.get(),
        phylanx::ir::node_data<double>(blaze::DynamicVector<double>{1, 2, 3}));
}

void test_linear_solver_n(std::string const& func_name)
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{
                blaze::DynamicMatrix<double>{{0, 0, 0, 0, 0, 0},
                    {0, 1, 0, 0, 0, 0}, {0, 0, 2, 0, 0, 0}, {0, 0, 0, 3, 0, 0},
                    {0, 0, 0, 0, 4, 0}, {0, 0, 0, 0, 0, 100000}}});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{
                blaze::DynamicVector<double>{1, 1, 1, 1, 1, 1}});

    phylanx::execution_tree::primitive n =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(6));

    phylanx::execution_tree::primitive linear_solver =
        phylanx::execution_tree::primitives::create_linear_solver(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs), std::move(n)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linear_solver.eval();
    HPX_TEST_EQ(f.get(),
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{100000, 0, 4, 1, 2, 3}));
}

void test_linear_solver_m(std::string const& func_name)
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicMatrix<double>{
                {0.46, 0.60, 0.74, 0.61, 0.85}, {0.56, 0.31, 0.80, 0.94, 0.76},
                {0.41, 0.19, 0.15, 0.33, 0.06}, {0.03, 0.92, 0.15, 0.56, 0.08},
                {0.09, 0.06, 0.69, 0.42, 0.96}}});

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(hpx::find_here(),
            phylanx::ir::node_data<double>{blaze::DynamicVector<double>{
                1.788, 1.891, 0.458, 0.818, 1.53}});

    phylanx::execution_tree::primitive n =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));

    phylanx::execution_tree::primitive linear_solver =
        phylanx::execution_tree::primitives::create_linear_solver(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs), std::move(n)},
            func_name);

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linear_solver.eval();
    HPX_TEST_EQ(f.get(),
        phylanx::ir::node_data<double>(
            blaze::DynamicVector<double>{0.1, 0.3, 0.5, 0.7, 0.9}));
}

int main()
{
    test_linear_solver_lu_PhySL();

    test_linear_solver_ldlt_u_PhySL();
    test_linear_solver_ldlt_l_PhySL();
    test_linear_solver_ldlt_PhySL();

    test_linear_solver_cholesky_PhySL();
    test_linear_solver_cholesky_u_PhySL();
    test_linear_solver_cholesky_l_PhySL();

#ifdef PHYLANX_HAVE_BLAZE_ITERATIVE
    test_linear_solver_cg_jacobi_PhySL();
    test_linear_solver_cg_ssor_PhySL();
    test_linear_solver_cg_incompleteCholesky_PhySL();
    test_linear_solver_cg_symmetric_gauss_seidel_PhySL();
    test_linear_solver_lanczos_PhySL();
    test_linear_solver_arnoldi_PhySL();
    test_linear_solver_gmres_PhySL();
#endif

    test_linear_solver_lu("linear_solver_lu");

    test_linear_solver("linear_solver_ldlt");
    test_linear_solver_l("linear_solver_ldlt");
    test_linear_solver_u("linear_solver_ldlt");

    test_linear_solver("linear_solver_cholesky");
    test_linear_solver_l("linear_solver_cholesky");
    test_linear_solver_u("linear_solver_cholesky");
#ifdef PHYLANX_HAVE_BLAZE_ITERATIVE
    test_linear_solver("iterative_solver_conjugate_gradient");
    test_linear_solver("iterative_solver_bicgstab");
    test_linear_solver("iterative_solver_bicgstab_lu");
    test_linear_solver("iterative_solver_bicgstab_rq");
    test_linear_solver("iterative_solver_bicgstab_qr");
    test_linear_solver("iterative_solver_bicgstab_cholesky");
    test_linear_solver("iterative_solver_cg_jacobi");
    test_linear_solver("iterative_solver_cg_ssor");
    test_linear_solver("iterative_solver_cg_incompleteCholesky");
    test_linear_solver("iterative_solver_cg_symmetric_gauss_seidel");
    test_linear_solver_n("iterative_solver_lanczos");
    test_linear_solver_n("iterative_solver_arnoldi");
    test_linear_solver_m("iterative_solver_gmres");
#endif
    return hpx::util::report_errors();
}
