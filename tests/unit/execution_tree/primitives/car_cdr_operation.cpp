//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    return phylanx::execution_tree::compile(code, snippets, env);
}

///////////////////////////////////////////////////////////////////////////////
void test_car_cdr_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile(code)(), compile(expected_str)());
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_car_cdr_operation("car( '(1, 2, 3) )", "1");
    test_car_cdr_operation("cdr( '(1, 2, 3) )", "'(2, 3)");

    test_car_cdr_operation("caar( '('(1, 2), '(3, 4)) )", "1");
    test_car_cdr_operation("cadr( '('(1, 2), '(3, 4)) )", "'(3, 4)");
    test_car_cdr_operation("cdar( '('(1, 2), '(3, 4)) )", "'(2)");
    test_car_cdr_operation("cddr( '('(1, 2), '(3, 4)) )", "'()");

    test_car_cdr_operation(
        "caaar( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "1");
    test_car_cdr_operation(
        "caadr( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "'('(3), 4)");
    test_car_cdr_operation(
        "cadar( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "2");
    test_car_cdr_operation(
        "caddr( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "7");
    test_car_cdr_operation(
        "cdaar( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "'()");
    test_car_cdr_operation(
        "cdadr( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "'('(5), 6)");
    test_car_cdr_operation(
        "cddar( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "'()");
    test_car_cdr_operation(
        "cdddr( '( '('(1), 2), '( '('(3), 4), '(5), 6), 7 ) )", "'()");

    return hpx::util::report_errors();
}
