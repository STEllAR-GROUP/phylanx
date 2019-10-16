// Copyright (c) 2019 Maxwell Reeser
// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);
    return code.run();
}

void test_annotation_equality_0()
{
    std::string annotation_0 = R"(
            annotate_d([[91, 91]], "test2d2d_4_1/1",
                list("tile", list("columns", 0, 2), list("rows", 0, 1)))
        )";
    std::string annotation_1 = R"(
            annotate_d([[91, 91]], "test2d2d_4_1/1",
                list("tile", list("rows", 0, 1), list("columns", 0, 2)))
        )";

    HPX_TEST_EQ(compile_and_run("annotation_0", annotation_0),
        compile_and_run("annotation_1", annotation_1));
}

int main(int argc, char* argv[])
{
    test_annotation_equality_0();

    return hpx::util::report_errors();
}
