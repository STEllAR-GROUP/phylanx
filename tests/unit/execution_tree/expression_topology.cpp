//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>

void test_expressiontree_topology(char const* name,
    char const* code, char const* newick_expected,
    char const* dot_expected)
{
    phylanx::execution_tree::compiler::function_list snippets;

    phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(code), snippets);

    auto topology = snippets.get_expression_topology();
    std::string newick_tree =
        phylanx::execution_tree::newick_tree(name, topology);

    HPX_TEST_EQ(newick_tree, std::string(newick_expected));

    std::string dot_tree = phylanx::execution_tree::dot_tree(name, topology);

    HPX_TEST_EQ(dot_tree, std::string(dot_expected));
}

int main(int argc, char* argv[])
{
    test_expressiontree_topology("test1",
        "define(x, 42)",
            "(/phylanx/define-variable$0$x/0$1$8) test1;",
            "graph test1 {\n"
            "    \"/phylanx/define-variable$0$x/0$1$8\";\n"
            "}\n");

    test_expressiontree_topology("test2",
        "block(define(x, 42), define(y, x))",
            "((/phylanx/define-variable$0$x/0$1$14,"
                "(/phylanx/access-variable$0$x/0$1$32) "
                "/phylanx/define-variable$1$y/0$1$29) "
                "/phylanx/block$0/0$1$1) test2;",
            "graph test2 {\n"
            "    \"/phylanx/block$0/0$1$1\" -- "
                    "\"/phylanx/define-variable$0$x/0$1$14\";\n"
            "    \"/phylanx/define-variable$0$x/0$1$14\";\n"
            "    \"/phylanx/block$0/0$1$1\" -- "
                    "\"/phylanx/define-variable$1$y/0$1$29\";\n"
            "    \"/phylanx/define-variable$1$y/0$1$29\" -- "
                    "\"/phylanx/access-variable$0$x/0$1$32\";\n"
            "    \"/phylanx/access-variable$0$x/0$1$32\";\n"
            "}\n");

    return hpx::util::report_errors();
}

