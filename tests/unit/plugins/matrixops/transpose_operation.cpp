//   Copyright (c) 2017-2018 Hartmut Kaiser
//   Copyright (c) 2019 Bita Hasheminezhad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

void test_transpose_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(5.0));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    HPX_TEST_EQ(
        5.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_transpose_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(5.0);

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    HPX_TEST_EQ(
        5.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_transpose_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicVector<double> expected(v);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicVector<double> expected(v);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_1d_axes0d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive axes =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs),std::move(axes)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicVector<double> expected(v);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_1d_axes1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive axes =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-1));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(axes)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicVector<double> expected(v);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicMatrix<double> expected = blaze::trans(m);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_2d_nil()
{
    blaze::DynamicMatrix<std::int64_t> subject{{1, 2, 3},
                                               {3, 4, 1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});

    phylanx::execution_tree::primitive l2_normalize =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        l2_normalize.eval();

    blaze::DynamicMatrix<std::int64_t> expected{{1, 3},{2, 4},{3, 1}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_transpose_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicMatrix<double> expected = blaze::trans(m);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_2d_axes()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    blaze::DynamicVector<std::int64_t> v{1, 0};
    phylanx::execution_tree::primitive axes =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(v));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(axes)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicMatrix<double> expected = blaze::trans(m);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_transpose_operation_2d_axes_nochange()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42UL, 42UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    blaze::DynamicVector<std::int64_t> v{-2, -1};
    phylanx::execution_tree::primitive axes =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(v));

    phylanx::execution_tree::primitive transpose =
        phylanx::execution_tree::primitives::create_transpose_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                phylanx::execution_tree::primitive_argument_type{
                    std::move(lhs)},
                phylanx::execution_tree::primitive_argument_type{
                    std::move(axes)}});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        transpose.eval();

    blaze::DynamicMatrix<double> expected(m);

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

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
void test_transpose_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_transpose_operation_0d();
    test_transpose_operation_0d_lit();

    test_transpose_operation_1d();
    test_transpose_operation_1d_lit();
    test_transpose_operation_1d_axes0d();
    test_transpose_operation_1d_axes1d();

    test_transpose_operation_2d();
    test_transpose_operation_2d_nil();
    test_transpose_operation_2d_lit();
    test_transpose_operation_2d_axes();
    test_transpose_operation_2d_axes_nochange();

    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]]])", "[[[1], [4]],[[2], [5]],[[3], [6]]]");
    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
        "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [2, 1, 0])",
        "[[[ 1,  7, 13, 19],[ 4, 10, 16, 22]],[[ 2,  8, 14, 20],"
        "[ 5, 11, 17, 23]],[[ 3,  9, 15, 21],[ 6, 12, 18, 24]]]");
    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
        "[[13,14,15],[16,17,18]],[[19,20,21],[22,23,24]]], make_list(1, 0, 2))",
        "[[[ 1,  2,  3],[ 7,  8,  9],[13, 14, 15],[19, 20, 21]],"
        "[[ 4,  5,  6],[10, 11, 12],[16, 17, 18],[22, 23, 24]]]");
    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
        "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [0, 2, 1])",
        "[[[ 1,  4],[ 2,  5],[ 3,  6]], [[ 7, 10],[ 8, 11],[ 9, 12]],"
        "[[13, 16],[14, 17],[15, 18]], [[19, 22],[20, 23],[21, 24]]]");
    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
        "[[13,14,15],[16,17,18]],[[19,20,21],[22,23,24]]], make_list(1, 2, 0))",
        "[[[ 1,  7, 13, 19],[ 2,  8, 14, 20],[ 3,  9, 15, 21]],"
        "[[ 4, 10, 16, 22],[ 5, 11, 17, 23],[ 6, 12, 18, 24]]]");
    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
        "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [1, -1, -3])",
        "[[[ 1,  7, 13, 19],[ 2,  8, 14, 20],[ 3,  9, 15, 21]],"
        "[[ 4, 10, 16, 22],[ 5, 11, 17, 23],[ 6, 12, 18, 24]]]");
    test_transpose_operation(
        "transpose([[[1,2,3],[4,5,6]], [[7,8,9],[10,11,12]],"
        "[[13,14,15],[16,17,18]], [[19,20,21],[22,23,24]]], [2, 0, 1])",
        "[[[ 1,  4],[ 7, 10],[13, 16],[19, 22]],"
        "[[ 2,  5],[ 8, 11],[14, 17],[20, 23]],"
        "[[ 3,  6],[ 9, 12],[15, 18],[21, 24]]]");
    test_transpose_operation(
        "transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
        "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]],"
        "[[[-1,-2,-3,-4],[-5,-6,-7,-8],[-9,-10,-11,-12]],"
        "[[-13,-14,-15,-16],[-17,-18,-19,-20],[-21,-22,-23,-24]]]])",
        "[[[[  1,  -1],[ 13, -13]], [[  5,  -5],[ 17, -17]],"
        "[[  9,  -9],[ 21, -21]]],[[[  2,  -2], [ 14, -14]],"
        "[[  6,  -6],[ 18, -18]], [[ 10, -10], [ 22, -22]]],"
        "[[[  3,  -3],[ 15, -15]], [[  7,  -7], [ 19, -19]],"
        "[[ 11, -11],[ 23, -23]]],[[[  4,  -4], [ 16, -16]],"
        "[[  8,  -8],[ 20, -20]], [[ 12, -12], [ 24, -24]]]]");
    test_transpose_operation(
        "transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
        "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]],"
        "[[[-1,-2,-3,-4],[-5,-6,-7,-8],[-9,-10,-11,-12]],"
        "[[-13,-14,-15,-16],[-17,-18,-19,-20],[-21,-22,-23,-24]]]],"
        "make_list(0,1,3,2))",
        "[[[[  1,   5,   9], [  2,   6,  10],  [  3,   7,  11],"
        "[  4,   8,  12]],[[ 13,  17,  21],  [ 14,  18,  22],"
        "[ 15,  19,  23], [ 16,  20,  24]]],[[[ -1,  -5,  -9],"
        "[ -2,  -6, -10], [ -3,  -7, -11],  [ -4,  -8, -12]],"
        "[[-13, -17, -21], [-14, -18, -22],  [-15, -19, -23],"
        "[-16, -20, -24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(0,2,1,3))",
        "[[[[ 1,  2,  3,  4], [13, 14, 15, 16]],"
        "[[ 5,  6,  7,  8], [17, 18, 19, 20]],"
        "[[ 9, 10, 11, 12], [21, 22, 23, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(0,2,3,1))",
        "[[[[ 1, 13], [ 2, 14], [ 3, 15], [ 4, 16]],"
        "[[ 5, 17], [ 6, 18], [ 7, 19], [ 8, 20]],"
        "[[ 9, 21], [10, 22], [11, 23], [12, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(0,3,1,2))",
        "[[[[ 1,  5,  9],[13, 17, 21]],"
        "[[ 2,  6, 10],[14, 18, 22]],"
        "[[ 3,  7, 11],[15, 19, 23]],"
        "[[ 4,  8, 12],[16, 20, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(0,3,2,1))",
        "[[[[ 1, 13], [ 5, 17], [ 9, 21]],"
        "[[ 2, 14], [ 6, 18], [10, 22]],"
        "[[ 3, 15], [ 7, 19], [11, 23]],"
        "[[ 4, 16], [ 8, 20], [12, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,0,2,3))",
        "[[[[ 1,  2,  3,  4],[ 5,  6,  7,  8],[ 9, 10, 11, 12]]],"
        "[[[13, 14, 15, 16],[17, 18, 19, 20],[21, 22, 23, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,0,3,2))",
        "[[[[ 1,  5,  9], [ 2,  6, 10],[ 3,  7, 11],[ 4,  8, 12]]],"
        "[[[13, 17, 21],[14, 18, 22],[15, 19, 23],[16, 20, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,0,3,2))",
        "[[[[ 1,  5,  9], [ 2,  6, 10],[ 3,  7, 11],[ 4,  8, 12]]],"
        "[[[13, 17, 21],[14, 18, 22],[15, 19, 23],[16, 20, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,2,0,3))",
        "[[[[ 1,  2,  3,  4]],[[ 5,  6,  7,  8]], [[ 9, 10, 11, 12]]],"
        "[[[13, 14, 15, 16]], [[17, 18, 19, 20]], [[21, 22, 23, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,2,3,0))",
        "[[[[ 1], [ 2], [ 3], [ 4]],"
        "[[ 5], [ 6], [ 7], [ 8]],"
        "[[ 9], [10], [11], [12]]],"
        "[[[13], [14], [15], [16]],"
        "[[17], [18], [19], [20]],"
        "[[21], [22], [23], [24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,3,0,2))",
        "[[[[ 1,  5,  9]],[[ 2,  6, 10]],[[ 3,  7, 11]],[[ 4,  8, 12]]],"
        "[[[13, 17, 21]],[[14, 18, 22]],[[15, 19, 23]],[[16, 20, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(1,3,2,0))",
        "[[[[ 1],[ 5],[ 9]],[[ 2],[ 6],[10]],"
        "[[ 3],[ 7],[11]],[[ 4],[ 8],[12]]],"
        "[[[13],[17],[21]],[[14],[18],[22]],"
        "[[15],[19],[23]],[[16],[20],[24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(2,0,1,3))",
        "[[[[ 1,  2,  3,  4], [13, 14, 15, 16]]],"
        "[[[ 5,  6,  7,  8], [17, 18, 19, 20]]],"
        "[[[ 9, 10, 11, 12], [21, 22, 23, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(2,0,3,1))",
        "[[[[ 1, 13], [ 2, 14], [ 3, 15], [ 4, 16]]],"
        "[[[ 5, 17], [ 6, 18], [ 7, 19], [ 8, 20]]],"
        "[[[ 9, 21], [10, 22], [11, 23], [12, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(2,1,0,3))",
        "[[[[ 1,  2,  3,  4]],[[13, 14, 15, 16]]],"
        "[[[ 5,  6,  7,  8]],[[17, 18, 19, 20]]],"
        "[[[ 9, 10, 11, 12]],[[21, 22, 23, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(2,1,3,0))",
        "[[[[ 1],[ 2],[ 3],[ 4]],[[13],[14],[15],[16]]],"
        "[[[ 5],[ 6],[ 7],[ 8]],[[17],[18],[19],[20]]],"
        "[[[ 9],[10],[11],[12]],[[21],[22],[23],[24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(2,3,0,1))",
        "[[[[ 1, 13]], [[ 2, 14]], [[ 3, 15]], [[ 4, 16]]],"
        "[[[ 5, 17]], [[ 6, 18]], [[ 7, 19]], [[ 8, 20]]],"
        "[[[ 9, 21]], [[10, 22]], [[11, 23]], [[12, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(2,3,1,0))",
        "[[[[ 1],[13]],[[ 2],[14]],[[ 3],[15]],[[ 4],[16]]],"
        "[[[ 5],[17]],[[ 6],[18]],[[ 7],[19]],[[ 8],[20]]],"
        "[[[ 9],[21]],[[10],[22]],[[11],[23]],[[12],[24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(3,0,1,2))",
        "[[[[ 1,  5,  9],[13, 17, 21]]],"
        "[[[ 2,  6, 10],[14, 18, 22]]],"
        "[[[ 3,  7, 11],[15, 19, 23]]],"
        "[[[ 4,  8, 12],[16, 20, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(3,0,2,1))",
        "[[[[ 1, 13],[ 5, 17],[ 9, 21]]],"
        "[[[ 2, 14],[ 6, 18],[10, 22]]],"
        "[[[ 3, 15],[ 7, 19],[11, 23]]],"
        "[[[ 4, 16],[ 8, 20],[12, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(3,1,0,2))",
        "[[[[ 1,  5,  9]],[[13, 17, 21]]],"
        "[[[ 2,  6, 10]],[[14, 18, 22]]],"
        "[[[ 3,  7, 11]],[[15, 19, 23]]],"
        "[[[ 4,  8, 12]],[[16, 20, 24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(3,1,2,0))",
        "[[[[ 1], [ 5], [ 9]], [[13], [17], [21]]],"
        "[[[ 2], [ 6], [10]], [[14], [18], [22]]],"
        "[[[ 3], [ 7], [11]], [[15], [19], [23]]],"
        "[[[ 4], [ 8], [12]], [[16], [20], [24]]]]");
    test_transpose_operation("transpose([[[[1,2,3,4],[5,6,7,8],[9,10,11,12]],"
                             "[[13,14,15,16],[17,18,19,20],[21,22,23,24]]]],"
                             "make_list(3,2,0,1))",
        "[[[[ 1, 13]], [[ 5, 17]], [[ 9, 21]]],"
        "[[[ 2, 14]], [[ 6, 18]], [[10, 22]]],"
        "[[[ 3, 15]], [[ 7, 19]], [[11, 23]]],"
        "[[[ 4, 16]], [[ 8, 20]], [[12, 24]]]]");

    return hpx::util::report_errors();
}
