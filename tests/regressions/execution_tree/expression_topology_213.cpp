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
    test_expressiontree_topology("fact",
        "block("
            "define(fact, arg0, if(arg0 <= 1, 1, arg0 * fact(arg0 - 1))),"
            "fact(10)"
        ")",
        "((((((/phylanx/access-argument$0$arg0/0$28) /phylanx/__le$0/0$28,"
            "(/phylanx/access-argument$1$arg0/0$42,"
            "(/phylanx/define-function$0$fact/0$13) "
            "/phylanx/call-function$0$fact/0$49) /phylanx/__mul$0/0$42) "
            "/phylanx/if$0/0$25) /phylanx/define-function$0$fact/0$13) "
            "/phylanx/call-function$0$fact/0$66) /phylanx/block$0/0$0) fact;",
        "graph fact {\n"
        "    \"/phylanx/block$0/0$0\" -- "
                "\"/phylanx/call-function$0$fact/0$66\";\n"
        "    \"/phylanx/call-function$0$fact/0$66\" -- "
                "\"/phylanx/define-function$0$fact/0$13\";\n"
        "    \"/phylanx/define-function$0$fact/0$13\" -- "
                "\"/phylanx/if$0/0$25\";\n"
        "    \"/phylanx/if$0/0$25\" -- \"/phylanx/__le$0/0$28\";\n"
        "    \"/phylanx/__le$0/0$28\" -- "
                "\"/phylanx/access-argument$0$arg0/0$28\";\n"
        "    \"/phylanx/access-argument$0$arg0/0$28\";\n"
        "    \"/phylanx/if$0/0$25\" -- \"/phylanx/__mul$0/0$42\";\n"
        "    \"/phylanx/__mul$0/0$42\" -- "
                "\"/phylanx/access-argument$1$arg0/0$42\";\n"
        "    \"/phylanx/access-argument$1$arg0/0$42\";\n"
        "    \"/phylanx/__mul$0/0$42\" -- "
                "\"/phylanx/call-function$0$fact/0$49\";\n"
        "    \"/phylanx/call-function$0$fact/0$49\" -- "
                "\"/phylanx/define-function$0$fact/0$13\";\n"
        "    \"/phylanx/define-function$0$fact/0$13\";\n"
        "}\n");

    return hpx::util::report_errors();
}

