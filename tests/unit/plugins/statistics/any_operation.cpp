//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

void test_any_operation_0d_true()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(true, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_0d_false()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_0d_double_true()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_0d_double_false()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(
        v.nonZeros() != 0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_false()
{
    blaze::DynamicVector<std::uint8_t> v(1007UL, 0);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();
    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_double()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<double> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(
        v.nonZeros() != 0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_double_false()
{
    blaze::DynamicVector<double> v(1007UL, 0.0);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();
    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_numpy_false()
{
    blaze::DynamicVector<double> v{false, false, false};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_numpy_true()
{
    blaze::DynamicVector<double> v{true, false, false};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_double_numpy_false()
{
    blaze::DynamicVector<double> v{0.0, 0.0, 0.0};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_1d_double_numpy_true()
{
    blaze::DynamicVector<double> v{1.0, 0.0, 0.0};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(
        m.nonZeros() != 0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_false()
{
    blaze::DynamicMatrix<std::uint8_t> m(101UL, 101UL, 0);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_double()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(
        m.nonZeros() != 0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_double_false()
{
    blaze::DynamicMatrix<double> m(101UL, 101UL, 0.0);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_numpy_false()
{
    blaze::DynamicMatrix<double> m{
        {false, false, false}, {false, false, false}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_numpy_true()
{
    blaze::DynamicMatrix<double> m{{true, false, true}, {false, true, true}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_double_numpy_false()
{
    blaze::DynamicMatrix<double> m{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_2d_double_numpy_true()
{
    blaze::DynamicMatrix<double> m{{1.0, 0.0, 3.0}, {0.0, 1.0, 6.0}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_any_operation_3d()
{
    blaze::Rand<blaze::DynamicTensor<int>> gen{};
    blaze::DynamicTensor<std::uint8_t> m =
        gen.generate(10UL, 101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(m.nonZeros() != 0,
        phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_false()
{
    blaze::DynamicTensor<std::uint8_t> m(10UL, 101UL, 101UL, 0);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_double()
{
    blaze::Rand<blaze::DynamicTensor<int>> gen{};
    blaze::DynamicTensor<double> m = gen.generate(10UL, 101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(m.nonZeros() != 0,
        phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_double_false()
{
    blaze::DynamicTensor<double> m(10UL, 101UL, 101UL, 0.0);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_numpy_false()
{
    blaze::DynamicTensor<double> m{
        {{false, false, false}, {false, false, false}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_numpy_true()
{
    blaze::DynamicTensor<double> m{{{true, false, true}, {false, true, true}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_double_numpy_false()
{
    blaze::DynamicTensor<double> m{{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_any_operation_3d_double_numpy_true()
{
    blaze::DynamicTensor<double> m{{{1.0, 0.0, 3.0}, {0.0, 1.0, 6.0}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive any =
        phylanx::execution_tree::primitives::create_any_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = any.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}
#endif

int main(int argc, char* argv[])
{
    test_any_operation_0d_true();
    test_any_operation_0d_false();
    test_any_operation_0d_double_true();
    test_any_operation_0d_double_false();

    test_any_operation_1d();
    test_any_operation_1d_false();
    test_any_operation_1d_double();
    test_any_operation_1d_double_false();
    test_any_operation_1d_numpy_false();
    test_any_operation_1d_numpy_true();
    test_any_operation_1d_double_numpy_false();
    test_any_operation_1d_double_numpy_true();

    test_any_operation_2d();
    test_any_operation_2d_false();
    test_any_operation_2d_double();
    test_any_operation_2d_double_false();
    test_any_operation_2d_numpy_false();
    test_any_operation_2d_numpy_true();
    test_any_operation_2d_double_numpy_false();
    test_any_operation_2d_double_numpy_true();

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_any_operation_3d();
    test_any_operation_3d_false();
    test_any_operation_3d_double();
    test_any_operation_3d_double_false();
    test_any_operation_3d_numpy_false();
    test_any_operation_3d_numpy_true();
    test_any_operation_3d_double_numpy_false();
    test_any_operation_3d_double_numpy_true();
#endif
    return hpx::util::report_errors();
}
