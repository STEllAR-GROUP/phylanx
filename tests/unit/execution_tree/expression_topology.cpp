//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <string>

void test_expressiontree_topology(char const* name,
    char const* codestr, char const* newick_expected,
    char const* dot_expected)
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(codestr), snippets);
    auto topology = snippets.program_.get_expression_topology();

    std::string newick_tree =
        phylanx::execution_tree::newick_tree(name, topology);
    HPX_TEST_EQ(newick_tree, std::string(newick_expected));

    std::string dot_tree = phylanx::execution_tree::dot_tree(name, topology);
    HPX_TEST_EQ(dot_tree, std::string(dot_expected));
}

int main(int argc, char* argv[])
{
    // <image url="$(ItemDir)/images/expression_topology_test1.dot.png" />
    //
    test_expressiontree_topology("test1",
        "define(x, 42)",
            "((/phylanx$0/variable$0$x/0$1$8) "
                "/phylanx$0/define-variable$0$x/0$1$8) test1;",
            "graph \"test1\" {\n"
            "    \"/phylanx$0/define-variable$0$x/0$1$8\" -- "
                    "\"/phylanx$0/variable$0$x/0$1$8\";\n"
            "    \"/phylanx$0/variable$0$x/0$1$8\";\n"
            "}\n");

    // <image url="$(ItemDir)/images/expression_topology_test2.dot.png" />
    //
    test_expressiontree_topology("test2",
        "block(define(x, 42), define(y, x))",
        "(((/phylanx$0/variable$0$x/0$1$14) "
            "/phylanx$0/define-variable$0$x/0$1$14,"
            "((/phylanx$0/access-variable$0$x/"
                "0$1$32) /phylanx$0/variable$1$y/0$1$29) "
            "/phylanx$0/define-variable$1$y/0$1$29) "
                "/phylanx$0/block$0/0$1$1) test2;",
        "graph \"test2\" {\n"
        "    \"/phylanx$0/block$0/0$1$1\" -- "
                "\"/phylanx$0/define-variable$0$x/0$1$14\";\n"
        "    \"/phylanx$0/define-variable$0$x/0$1$14\" -- "
                "\"/phylanx$0/variable$0$x/0$1$14\";\n"
        "    \"/phylanx$0/variable$0$x/0$1$14\";\n"
        "    \"/phylanx$0/block$0/0$1$1\" -- "
                "\"/phylanx$0/define-variable$1$y/0$1$29\";\n"
        "    \"/phylanx$0/define-variable$1$y/0$1$29\" -- "
                "\"/phylanx$0/variable$1$y/0$1$29\";\n"
        "    \"/phylanx$0/variable$1$y/0$1$29\" -- "
                "\"/phylanx$0/access-variable$0$x/0$1$32\";\n"
        "    \"/phylanx$0/access-variable$0$x/0$1$32\" -- "
                "\"/phylanx$0/variable$0$x/0$1$14\";\n"
        "    \"/phylanx$0/variable$0$x/0$1$14\";\n"
        "}\n");

    return hpx::util::report_errors();
}

