// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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

void test_0d_keep_dims_false_init()
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
    phylanx::execution_tree::primitive init =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.0));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0),
                std::move(arg1), std::move(arg2), std::move(init)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{84.};

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

void test_1d_keep_dims_true_init()
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
    phylanx::execution_tree::primitive init =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.5));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1), std::move(arg2), std::move(init)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{-162.};

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

void test_2d_axis1_keep_dims_true_init()
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
    phylanx::execution_tree::primitive init =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(2.));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0),
                std::move(arg1), std::move(arg2), std::move(init)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<double> expected{{108.}, {12.}, {2.}};

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

    blaze::DynamicMatrix<double> expected{{54.}, {6.}, {1.}};

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

void test_2d_axis1_keep_dims_false_init()
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
    phylanx::execution_tree::primitive init =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0),
                std::move(arg1), std::move(arg2), std::move(init)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicVector<double> expected{162., 18., 3.};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axes(std::int64_t axis1, std::int64_t axis2)
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::execution_tree::primitive_arguments_type{
                    phylanx::ir::node_data<std::int64_t>(axis1),
                    phylanx::ir::node_data<std::int64_t>(axis2)
                }
            });

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = 324.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_2d_axes_keep_dims_true(std::int64_t axis1, std::int64_t axis2)
{
    blaze::DynamicMatrix<double> subject{{6., 9.}, {2., 3.}, {-1., -1.}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::execution_tree::primitive_arguments_type{
                    phylanx::ir::node_data<std::int64_t>(axis1),
                    phylanx::ir::node_data<std::int64_t>(axis2)
                }
            });
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

    blaze::DynamicMatrix<double> expected({{324.}});

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = -116640.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_int()
{
    blaze::DynamicTensor<std::int64_t> subject{
        {{6, 9}, {2, 3}, {-1, -1}}, {{3, 4}, {1, 5}, {-2, 3}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    std::int64_t expected = -116640;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis0()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicMatrix<double> expected{{18., 36.}, {2., 15.}, {2., -3.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis0_int()
{
    blaze::DynamicTensor<std::int64_t> subject{
        {{6, 9}, {2, 3}, {-1, -1}}, {{3, 4}, {1, 5}, {-2, 3}}};
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

    blaze::DynamicMatrix<std::int64_t> expected{{18, 36}, {2, 15}, {2, -3}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_3d_axis1()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicMatrix<double> expected{{-12., -27.}, {-6., 60.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis1_int()
{
    blaze::DynamicTensor<std::int64_t> subject{
        {{6, 9}, {2, 3}, {-1, -1}}, {{3, 4}, {1, 5}, {-2, 3}}};
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

    blaze::DynamicMatrix<std::int64_t> expected{{-12, -27}, {-6, 60}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_3d_axis2()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<double> expected{{54., 6., 1.}, {12., 5., -6.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis2_int()
{
    blaze::DynamicTensor<std::int64_t> subject{
        {{6, 9}, {2, 3}, {-1, -1}}, {{3, 4}, {1, 5}, {-2, 3}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<std::int64_t> expected{{54, 6, 1}, {12, 5, -6}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}

void test_3d_keep_dims_true()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicTensor<double> expected{{{-116640.}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_keep_dims_false()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    double expected = -116640.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis0_keep_dims_true()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicTensor<double> expected{{{18., 36.}, {2., 15.}, {2., -3.}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis1_keep_dims_true()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicTensor<double> expected{{{-12., -27.}}, {{-6., 60.}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis2_keep_dims_true()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));
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

    blaze::DynamicTensor<double> expected{
        {{54.}, {6.}, {1.}}, {{12.}, {5.}, {-6.}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis0_keep_dims_false()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicMatrix<double> expected{{18., 36.}, {2., 15.}, {2., -3.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis1_keep_dims_false()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
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

    blaze::DynamicMatrix<double> expected{{-12., -27.}, {-6., 60.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis2_keep_dims_false()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));
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

    blaze::DynamicMatrix<double> expected{{54., 6., 1.}, {12., 5., -6.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axis2_keep_dims_false_init()
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));
    phylanx::execution_tree::primitive arg2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));
    phylanx::execution_tree::primitive init =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(2));

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg0),
                std::move(arg1), std::move(arg2), std::move(init)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    blaze::DynamicMatrix<double> expected{{108., 12., 2.}, {24., 10., -12.}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axes(std::int64_t axis1, std::int64_t axis2,
    blaze::DynamicVector<double> expected)
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::execution_tree::primitive_arguments_type{
                    phylanx::ir::node_data<std::int64_t>(axis1),
                    phylanx::ir::node_data<std::int64_t>(axis2)
                }
            });

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axes(std::int64_t axis1, std::int64_t axis2, std::int64_t axis3)
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::execution_tree::primitive_arguments_type{
                    phylanx::ir::node_data<std::int64_t>(axis1),
                    phylanx::ir::node_data<std::int64_t>(axis2),
                    phylanx::ir::node_data<std::int64_t>(axis3)
                }
            });

    phylanx::execution_tree::primitive prod =
        phylanx::execution_tree::primitives::create_prod_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        prod.eval();

    double expected = -116640.;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axes_keep_dims_true(std::int64_t axis1, std::int64_t axis2,
    blaze::DynamicTensor<double> expected)
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::execution_tree::primitive_arguments_type{
                    phylanx::ir::node_data<std::int64_t>(axis1),
                    phylanx::ir::node_data<std::int64_t>(axis2)
                }
            });
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

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_3d_axes_keep_dims_true(
    std::int64_t axis1, std::int64_t axis2, std::int64_t axis3)
{
    blaze::DynamicTensor<double> subject{
        {{6., 9.}, {2., 3.}, {-1., -1.}}, {{3., 4.}, {1., 5.}, {-2., 3.}}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{
                phylanx::execution_tree::primitive_arguments_type{
                    phylanx::ir::node_data<std::int64_t>(axis1),
                    phylanx::ir::node_data<std::int64_t>(axis2),
                    phylanx::ir::node_data<std::int64_t>(axis3)
                }
            });
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

    blaze::DynamicTensor<double> expected{{{-116640.}}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_0d();
    test_0d_keep_dims_true();
    test_0d_keep_dims_false();
    test_0d_keep_dims_false_init();

    test_1d();
    test_1d_keep_dims_true();
    test_1d_keep_dims_true_init();
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
    test_2d_axis1_keep_dims_false_init();
    test_2d_int();
    test_2d_axis0_int();
    test_2d_axis1_int();

    test_2d_axes(0, 1);
    test_2d_axes(1, 0);

    test_2d_axes_keep_dims_true(0, 1);
    test_2d_axes_keep_dims_true(1, 0);

    test_3d();
    test_3d_axis0();
    test_3d_axis1();
    test_3d_axis2();
    test_3d_keep_dims_true();
    test_3d_keep_dims_false();
    test_3d_axis0_keep_dims_true();
    test_3d_axis1_keep_dims_true();
    test_3d_axis2_keep_dims_true();
    test_3d_axis0_keep_dims_false();
    test_3d_axis1_keep_dims_false();
    test_3d_axis2_keep_dims_false();
    test_3d_axis2_keep_dims_false_init();
    test_3d_int();
    test_3d_axis0_int();
    test_3d_axis1_int();
    test_3d_axis2_int();

    test_3d_axes(0, 1, blaze::DynamicVector<double>({72., -1620.}));
    test_3d_axes(1, 0, blaze::DynamicVector<double>({72., -1620.}));
    test_3d_axes(0, 2, blaze::DynamicVector<double>({648., 30., -6.}));
    test_3d_axes(2, 0, blaze::DynamicVector<double>({648., 30., -6.}));
    test_3d_axes(1, 2, blaze::DynamicVector<double>({324., -360.}));
    test_3d_axes(2, 1, blaze::DynamicVector<double>({324., -360.}));

    test_3d_axes_keep_dims_true(
        0, 1, blaze::DynamicTensor<double>({{{72., -1620.}}}));
    test_3d_axes_keep_dims_true(
        1, 0, blaze::DynamicTensor<double>({{{72., -1620.}}}));
    test_3d_axes_keep_dims_true(
        0, 2, blaze::DynamicTensor<double>({{{648.}, {30.}, {-6.}}}));
    test_3d_axes_keep_dims_true(
        2, 0, blaze::DynamicTensor<double>({{{648.}, {30.}, {-6.}}}));
    test_3d_axes_keep_dims_true(
        1, 2, blaze::DynamicTensor<double>({{{324.}}, {{-360.}}}));
    test_3d_axes_keep_dims_true(
        2, 1, blaze::DynamicTensor<double>({{{324.}}, {{-360.}}}));

    test_3d_axes(0, 1, 2);
    test_3d_axes(1, 2, 0);
    test_3d_axes(2, 0, 1);
    test_3d_axes(0, 2, 1);
    test_3d_axes(2, 1, 0);
    test_3d_axes(1, 0, 2);

    test_3d_axes_keep_dims_true(0, 1, 2);
    test_3d_axes_keep_dims_true(1, 2, 0);
    test_3d_axes_keep_dims_true(2, 0, 1);
    test_3d_axes_keep_dims_true(0, 2, 1);
    test_3d_axes_keep_dims_true(2, 1, 0);
    test_3d_axes_keep_dims_true(1, 0, 2);

    return hpx::util::report_errors();
}
