//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
void test_car_cdr_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_car_cdr_operation("car( list(1, 2, 3) )", "1");
    test_car_cdr_operation("cdr( list(1, 2, 3) )", "list(2, 3)");

    test_car_cdr_operation("caar( list(list(1, 2), list(3, 4)) )", "1");
    test_car_cdr_operation(
        "cadr( list(list(1, 2), list(3, 4)) )", "list(3, 4)");
    test_car_cdr_operation("cdar( list(list(1, 2), list(3, 4)) )", "list(2)");
    test_car_cdr_operation("cddr( list(list(1, 2), list(3, 4)) )", "list()");

    test_car_cdr_operation("caaar( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "1");
    test_car_cdr_operation("caadr( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "list(list(3), 4)");
    test_car_cdr_operation("cadar( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "2");
    test_car_cdr_operation("caddr( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "7");
    test_car_cdr_operation("cdaar( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "list()");
    test_car_cdr_operation("cdadr( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "list(list(5), 6)");
    test_car_cdr_operation("cddar( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "list()");
    test_car_cdr_operation("cdddr( list( list(list(1), 2), list( list(list(3), "
                           "4), list(5), 6), 7 ) )", "list()");

    return hpx::util::report_errors();
}
