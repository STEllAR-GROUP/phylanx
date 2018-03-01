//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <utility>
#include <vector>

void test_all_operation_0d_true()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(true));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_0d_false()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(false));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_0d_double_true()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_0d_double_false()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(v.nonZeros() == v.size(),
        phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<bool> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(v.nonZeros() == v.size(),
        phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_double_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<bool> v = gen.generate(1007UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_numpy_false()
{
    blaze::DynamicVector<bool> v{true, false, true};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_numpy_true()
{
    blaze::DynamicVector<double> v{true, true, true};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_double_numpy_false()
{
    blaze::DynamicVector<double> v{1.0, 0.0, 3.0};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_1d_double_numpy_true()
{
    blaze::DynamicVector<double> v{1.0, 2.0, 3.0};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<bool> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(m.nonZeros() == m.rows() * m.columns(),
        phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<bool> m = gen.generate(101UL, 101UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_double()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(m.nonZeros() == m.rows() * m.columns(),
        phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_double_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_double_numpy_false()
{
    blaze::DynamicMatrix<double> m{{1.0, 2.0, 3.0}, {0.0, 1.0, 0.0}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_double_numpy_true()
{
    blaze::DynamicMatrix<double> m{{1.0, 2.0, 3.0}, {4.0, 1.0, 6.0}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_numpy_true()
{
    blaze::DynamicMatrix<bool> m{{true, true, true}, {true, true, true}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_boolean_value(f));
}

void test_all_operation_2d_numpy_false()
{
    blaze::DynamicMatrix<bool> m{{true, true, true}, {false, true, false}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<bool>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_boolean_value(f));
}

int main(int argc, char* argv[])
{
    test_all_operation_0d_true();
    test_all_operation_0d_false();
    test_all_operation_0d_double_true();
    test_all_operation_0d_double_false();

    test_all_operation_1d();
    test_all_operation_1d_true();
    test_all_operation_1d_double();
    test_all_operation_1d_double_true();
    test_all_operation_1d_numpy_false();
    test_all_operation_1d_numpy_true();
    test_all_operation_1d_double_numpy_false();
    test_all_operation_1d_double_numpy_true();

    test_all_operation_2d();
    test_all_operation_2d_true();
    test_all_operation_2d_double();
    test_all_operation_2d_double_true();
    test_all_operation_2d_double_numpy_false();
    test_all_operation_2d_double_numpy_true();
    test_all_operation_2d_numpy_true();
    test_all_operation_2d_numpy_false();

    return hpx::util::report_errors();
}
