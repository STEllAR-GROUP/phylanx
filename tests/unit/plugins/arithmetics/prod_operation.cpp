// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

#include <blaze/Math.h>

void test_0d()
{
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{42.};

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_0d_keep_dims_true()
{
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{42.};

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_0d_keep_dims_false()
{
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{42.};

    HPX_TEST_EQ(
        expected, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_1d()
{
    blaze::DynamicVector<double> subject{6., 9., 2., 3., -1.};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = -324.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_1d_keep_dims_true()
{
    blaze::DynamicVector<double> subject{6., 9., 2., 3., -1.};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{-324.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_1d_keep_dims_false()
{
    blaze::DynamicVector<double> subject{6., 9., 2., 3., -1.};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = -324.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = 324.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_int()
{
    blaze::DynamicMatrix<std::int64_t> subject{{6, 9}, {2, 3}, {-1, -1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    std::int64_t expected = 324;

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_2d_axis0()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{-12., -27.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis0_int()
{
    blaze::DynamicMatrix<std::int64_t> subject{{6, 9}, {2, 3}, {-1, -1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<std::int64_t> expected{-12, -27};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_2d_axis1()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{54., 6., 1.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis1_int()
{
    blaze::DynamicMatrix<std::int64_t> subject{{6, 9}, {2, 3}, {-1, -1}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<std::int64_t> expected{54, 6, 1};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_2d_keep_dims_true()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<double> expected{{324.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_keep_dims_false()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ast::nil{});
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = 324.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis0_keep_dims_true()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<double> expected{{-12., -27.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis1_keep_dims_true()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<double> expected{{54., 6., 1.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis0_keep_dims_false()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(0));
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{-12., -27.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axis1_keep_dims_false()
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(1));
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{54., 6., 1.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_0d();
    test_1d();
    test_1d_keep_dims_true();
    test_1d_keep_dims_false();
    test_2d();
    test_2d_axis0();
    test_2d_axis1();
    test_2d_keep_dims_true();
    test_2d_keep_dims_false();
    test_2d_axis0_keep_dims_true();
    test_2d_axis1_keep_dims_true();
    test_2d_axis0_keep_dims_false();
    test_2d_axis1_keep_dims_false();
    test_2d_int();
    test_2d_axis0_int();
    test_2d_axis1_int();

    return hpx::util::report_errors();
}
