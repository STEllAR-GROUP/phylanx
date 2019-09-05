//   Copyright (c) 2018 Shahrzad Shirzad
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

void test_all_operation_0d_true()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(true));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_0d_false()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(false));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_0d_double_true()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(3.0));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_0d_double_false()
{
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(v.nonZeros() == v.size(),
        phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_1d()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(v.nonZeros() == v.size(),
        phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_1d_double_true()
{
    blaze::Rand<blaze::DynamicVector<int>> gen{};
    blaze::DynamicVector<std::uint8_t> v = gen.generate(1007UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_1d_numpy_false()
{
    blaze::DynamicVector<std::uint8_t> v{true, false, true};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(v));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_2d()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(m.nonZeros() == m.rows() * m.columns(),
        phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_2d_true()
{
    blaze::Rand<blaze::DynamicMatrix<int>> gen{};
    blaze::DynamicMatrix<std::uint8_t> m = gen.generate(101UL, 101UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(m.nonZeros() == m.rows() * m.columns(),
        phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
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
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_2d_numpy_true()
{
    blaze::DynamicMatrix<std::uint8_t> m{{true, true, true}, {true, true, true}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_2d_numpy_false()
{
    blaze::DynamicMatrix<std::uint8_t> m{{true, true, true}, {false, true, false}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d()
{
    blaze::Rand<blaze::DynamicTensor<int>> gen{};
    blaze::DynamicTensor<std::uint8_t> m =
        gen.generate(10UL, 101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(m.nonZeros() == m.rows() * m.columns(),
        phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_true()
{
    blaze::Rand<blaze::DynamicTensor<int>> gen{};
    blaze::DynamicTensor<std::uint8_t> m =
        gen.generate(10UL, 101UL, 101UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_double()
{
    blaze::Rand<blaze::DynamicTensor<int>> gen{};
    blaze::DynamicTensor<double> m = gen.generate(10UL, 101UL, 101UL, 0, 1);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(m.nonZeros() == m.rows() * m.columns(),
        phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_double_true()
{
    blaze::Rand<blaze::DynamicTensor<int>> gen{};
    blaze::DynamicTensor<double> m = gen.generate(10UL, 101UL, 101UL, 1, 2);

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_double_numpy_false()
{
    blaze::DynamicTensor<double> m{{{1.0, 2.0, 3.0}, {0.0, 1.0, 0.0}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_double_numpy_true()
{
    blaze::DynamicTensor<double> m{{{1.0, 2.0, 3.0}, {4.0, 1.0, 6.0}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_numpy_true()
{
    blaze::DynamicTensor<std::uint8_t> m{
        {{true, true, true}, {true, true, true}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(1, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

void test_all_operation_3d_numpy_false()
{
    blaze::DynamicTensor<std::uint8_t> m{
        {{true, true, true}, {false, true, false}}};

    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(m));

    phylanx::execution_tree::primitive all =
        phylanx::execution_tree::primitives::create_all_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(arg1)});

    phylanx::execution_tree::primitive_argument_type f = all.eval().get();

    HPX_TEST_EQ(0, phylanx::execution_tree::extract_scalar_boolean_value(f));
}

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

void test_operation(std::string const& code, std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_all_operation_4d()
{
    test_operation(
        R"(all([[[[1.,2.], [4.,1.], [3.,4.]],[[3.,6.], [2.,-2.], [1.,1.]]],
            [[[1.,2.], [4.,1.], [3.,2.]],[[0. ,6.], [-2.,6.], [1.,1.]]]]))",
        R"(astype(0 ,"bool"))");
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

    test_all_operation_3d();
    test_all_operation_3d_true();
    test_all_operation_3d_double();
    test_all_operation_3d_double_true();
    test_all_operation_3d_double_numpy_false();
    test_all_operation_3d_double_numpy_true();
    test_all_operation_3d_numpy_true();
    test_all_operation_3d_numpy_false();

    test_all_operation_4d();

    return hpx::util::report_errors();
}
