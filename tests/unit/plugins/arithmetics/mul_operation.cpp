//   Copyright (c) 2017 Hartmut Kaiser
//   Copyright (c) 2017 Alireza Kheirkhahan
//   Copyright (c) 2017 Parsa Amini
//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <iostream>
#include <utility>
#include <vector>

void test_mul_operation_0d()
{
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_mul_operation_0d_lit()
{
    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(7.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

void test_mul_operation_0d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = 6.0 * v;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = 6.0 * v;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d1d_numpy()
{
    blaze::DynamicVector<double> v{4.0, 5.0, 6.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected{24.0, 30.0, 36.0};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101ul, 101ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = 6.0 * m;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101ul, 101ul);

    phylanx::ir::node_data<double> lhs(6.0);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_0d2d_numpy()
{
    blaze::DynamicMatrix<double> m{{4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected{
        {24.0, 30.0, 36.0}, {42.0, 48.0, 54.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d0d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = v * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d0d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007ul);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected = v * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d0d_numpy()
{
    blaze::DynamicVector<double> v{4.0, 5.0, 6.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected{24.0, 30.0, 36.0};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d1d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007ul);
    blaze::DynamicVector<double> v2 = gen.generate(1007ul);

    blaze::DynamicVector<double> expected = v1 * v2;

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d1d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v1 = gen.generate(1007ul);
    blaze::DynamicVector<double> v2 = gen.generate(1007ul);

    blaze::DynamicVector<double> expected = v1 * v2;

    phylanx::ir::node_data<double> lhs(v1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d1d_numpy()
{
    blaze::DynamicVector<double> v1{4.0, 5.0, 6.0};
    blaze::DynamicVector<double> v2{7.0, 8.0, 9.0};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected{28.0, 40.0, 54.0};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d1d_numpy_1()
{
    blaze::DynamicVector<double> v1{4.0, 5.0, 6.0};
    blaze::DynamicVector<double> v2{7.0, 8.0, 9.0};
    blaze::DynamicVector<double> v3{1.0, 3.0, 2.0};

    phylanx::execution_tree::primitive ops1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v1));

    phylanx::execution_tree::primitive ops2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v2));

    phylanx::execution_tree::primitive ops3 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v3));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(ops1), std::move(ops2), std::move(ops3)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicVector<double> expected{28.0, 120.0, 108.0};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d2d()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        div.eval();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::trans(v) * blaze::row(m, i);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d2d_lit()
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> v = gen.generate(104UL);

    blaze::Rand<blaze::DynamicMatrix<double>> mat_gen{};
    blaze::DynamicMatrix<double> m = mat_gen.generate(101UL, 104UL);

    phylanx::ir::node_data<double> lhs(v);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        div.eval();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::trans(v) * blaze::row(m, i);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_1d2d_numpy()
{
    blaze::DynamicVector<double> v{1.0, 2.0, 3.0};
    blaze::DynamicMatrix<double> m{{4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        div.eval();

    blaze::DynamicMatrix<double> expected{{4.0, 10.0, 18.0}, {7.0, 16.0, 27.0}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d0d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42ul, 42ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d0d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(42ul, 42ul);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m * 6.0;
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d0d_numpy()
{
    blaze::DynamicMatrix<double> m{{4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(6.0));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected{
        {24.0, 30.0, 36.0}, {42.0, 48.0, 54.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d1d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen1{};
    blaze::DynamicMatrix<double> m = gen1.generate(107ul, 42ul);
    blaze::Rand<blaze::DynamicVector<double>> gen2{};
    blaze::DynamicVector<double> v = gen2.generate(42ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
        std::move(lhs), std::move(rhs)
    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) * blaze::trans(v);
    }

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d1d_numpy()
{
    blaze::DynamicVector<double> v{1.0, 2.0, 3.0};
    blaze::DynamicMatrix<double> m{{4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive div =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        div.eval();
    blaze::DynamicMatrix<double> expected{{4.0, 10.0, 18.0}, {7.0, 16.0, 27.0}};

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d1d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen1{};
    blaze::DynamicMatrix<double> m = gen1.generate(107ul, 42ul);
    blaze::Rand<blaze::DynamicVector<double>> gen2{};
    blaze::DynamicVector<double> v = gen2.generate(42ul);

    phylanx::ir::node_data<double> lhs(m);

    phylanx::execution_tree::primitive rhs =
            phylanx::execution_tree::primitives::create_variable(
                    hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive mul =
            phylanx::execution_tree::primitives::create_mul_operation(
                    hpx::find_here(),
                    phylanx::execution_tree::primitive_arguments_type{
                            std::move(lhs), std::move(rhs)
                    });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
            mul.eval();

    blaze::DynamicMatrix<double> expected(m.rows(), m.columns());
    for (size_t i = 0UL; i < m.rows(); ++i)
    {
        blaze::row(expected, i) = blaze::row(m, i) * blaze::trans(v);
    }

    HPX_TEST_EQ(
            phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42ul, 42ul);
    blaze::DynamicMatrix<double> m2 = gen.generate(42ul, 42ul);

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m1 % m2;
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d_lit()
{
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m1 = gen.generate(42ul, 42ul);
    blaze::DynamicMatrix<double> m2 = gen.generate(42ul, 42ul);

    phylanx::ir::node_data<double> lhs(m1);

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)
            });

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected = m1 % m2;

    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d_numpy()
{
    blaze::DynamicMatrix<double> m1{{4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    blaze::DynamicMatrix<double> m2{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};

    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive rhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(lhs), std::move(rhs)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected{
        {4.0, 10.0, 18.0}, {28.0, 40.0, 54.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

void test_mul_operation_2d_numpy_1()
{
    blaze::DynamicMatrix<double> m1{{4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}};
    blaze::DynamicMatrix<double> m2{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    blaze::DynamicMatrix<double> m3{{2.0, 0.0, 3.0}, {1.0, 3.0, 7.0}};

    phylanx::execution_tree::primitive ops1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m1));

    phylanx::execution_tree::primitive ops2 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m2));
    phylanx::execution_tree::primitive ops3 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m3));

    phylanx::execution_tree::primitive mul =
        phylanx::execution_tree::primitives::create_mul_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(ops1), std::move(ops2), std::move(ops3)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        mul.eval();

    blaze::DynamicMatrix<double> expected{
        {8.0, 0.0, 54.0}, {28.0, 120.0, 378.0}};
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
        phylanx::execution_tree::extract_numeric_value(f.get()));
}

int main(int argc, char* argv[])
{
    test_mul_operation_0d();
    test_mul_operation_0d_lit();

    test_mul_operation_0d1d();
    test_mul_operation_0d1d_lit();
    test_mul_operation_0d1d_numpy();

    test_mul_operation_0d2d();
    test_mul_operation_0d2d_lit();
    test_mul_operation_0d2d_numpy();

    test_mul_operation_1d0d();
    test_mul_operation_1d0d_lit();
    test_mul_operation_1d0d_numpy();

    test_mul_operation_1d1d();
    test_mul_operation_1d1d_lit();
    test_mul_operation_1d1d_numpy();
    test_mul_operation_1d1d_numpy_1();

    test_mul_operation_1d2d();
    test_mul_operation_1d2d_lit();
    test_mul_operation_1d2d_numpy();

    test_mul_operation_2d0d();
    test_mul_operation_2d0d_lit();
    test_mul_operation_2d0d_numpy();

    test_mul_operation_2d1d();
    test_mul_operation_2d1d_lit();
    test_mul_operation_2d1d_numpy();

    test_mul_operation_2d();
    test_mul_operation_2d_lit();
    test_mul_operation_2d_numpy();
    test_mul_operation_2d_numpy_1();

    return hpx::util::report_errors();
}

