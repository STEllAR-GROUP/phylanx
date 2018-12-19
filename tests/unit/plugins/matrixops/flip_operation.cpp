// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>
#include <utility>

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

///////////////////////////////////////////////////////////////////////////////
void test_flip_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

void test_flip_int()
{
    blaze::DynamicMatrix<std::int64_t> subject{{13, 42}, {22, 43}};
    phylanx::execution_tree::primitive arg0 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(subject));
    phylanx::execution_tree::primitive arg1 =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(-1));

    phylanx::execution_tree::primitive flip =
        phylanx::execution_tree::primitives::create_flip_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(arg0), std::move(arg1)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        flip.eval();

    blaze::DynamicMatrix<std::int64_t> expected{{42, 13}, {43, 22}};

    HPX_TEST_EQ(phylanx::ir::node_data<std::int64_t>(std::move(expected)),
        phylanx::execution_tree::extract_integer_value(f.get()));
}
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_flip_operation("flip([13., 42., 33.], 0)", "hstack(33., 42., 13.)");
    test_flip_operation("flip([13., 42., 33.], -1)", "hstack(33., 42., 13.)");
    test_flip_operation("flip([[13., 42., 33.],[101., 12., 65.]],  0)",
        "vstack(hstack(101., 12., 65.), hstack(13., 42., 33.))");
    test_flip_operation("flip([[13., 42.],[22., 43.],[54., 41.]], -2)",
        "vstack(hstack(54., 41.), hstack(22., 43.), hstack(13., 42.))");
    test_flip_operation("flip([[13., 42., 33.],[101., 12., 65.]],  1)",
        "vstack(hstack(33., 42., 13.), hstack(65., 12., 101.))");
    test_flip_operation("flip([[13., 42.],[22., 43.],[54., 41.]], -1)",
        "vstack(hstack(42., 13.), hstack(43., 22.), hstack(41., 54.))");
    test_flip_int();

    return hpx::util::report_errors();
}
