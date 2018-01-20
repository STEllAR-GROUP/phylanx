//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_expressiontree_topology(char const* name,
    char const* code, char const* expected)
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto f = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(code), snippets);

    auto topology = f.get_expression_topology();
    std::string tree = phylanx::execution_tree::newick_tree(name, topology);

    HPX_TEST_EQ(tree, std::string(expected));
}

int main(int argc, char* argv[])
{
    test_expressiontree_topology("test1",
        "define(x, 42)",
            "(/phylanx/define-variable#0#x/0#7) test1;");

    test_expressiontree_topology("test2",
        "block(define(x, 42), define(y, x))",
        "((/phylanx/define-variable#0#x/0#13,"
            "(/phylanx/variable#0#x/0#31) "
                "/phylanx/define-variable#1#y/0#28) "
            "/phylanx/block#0/0#0) test2;");

    return hpx::util::report_errors();
}

