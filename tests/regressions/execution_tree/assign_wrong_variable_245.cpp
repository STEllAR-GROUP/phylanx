//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #245: PhySL Modifies Untouched Variable

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/iostream.hpp>
#include <hpx/modules/testing.hpp>

#include <sstream>
#include <string>

std::string const add_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 + A0 ),
    store(Result, A0 + A1 ),
    store(Result, A0 + A2 ),
    store(Result, A1 + A0 ),
    store(Result, A1 + A1 ),
    store(Result, A1 + A2 ),
    store(Result, A2 + A0 ),
    store(Result, A2 + A1 ),
    store(Result, A2 + A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R
)))";

void test_add()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(add_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const sub_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 - A0 ),
    store(Result, A0 - A1 ),
    store(Result, A0 - A2 ),
    store(Result, A1 - A0 ),
    store(Result, A1 - A1 ),
    store(Result, A1 - A2 ),
    store(Result, A2 - A0 ),
    store(Result, A2 - A1 ),
    store(Result, A2 - A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R))
)";

void test_sub()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(sub_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const mul_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 * A0 ),
    store(Result, A0 * A1 ),
    store(Result, A0 * A2 ),
    store(Result, A1 * A0 ),
    store(Result, A1 * A1 ),
    store(Result, A1 * A2 ),
    store(Result, A2 * A0 ),
    store(Result, A2 * A1 ),
    store(Result, A2 * A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),

    R))
)";

void test_mul()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(mul_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const div_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 / A0 ),
    store(Result, A0 / A1 ),
    store(Result, A0 / A2 ),
    store(Result, A1 / A0 ),
    store(Result, A1 / A1 ),
    store(Result, A1 / A2 ),
    store(Result, A2 / A0 ),
    store(Result, A2 / A1 ),
    store(Result, A2 / A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),

    R))
)";

void test_div()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(div_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const equal_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 == A0 ),
    store(Result, A0 == A1 ),
    store(Result, A0 == A2 ),
    store(Result, A1 == A0 ),
    store(Result, A1 == A1 ),
    store(Result, A1 == A2 ),
    store(Result, A2 == A0 ),
    store(Result, A2 == A1 ),
    store(Result, A2 == A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),

    R))
)";

void test_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(equal_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const not_equal_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 != A0 ),
    store(Result, A0 != A1 ),
    store(Result, A0 != A2 ),
    store(Result, A1 != A0 ),
    store(Result, A1 != A1 ),
    store(Result, A1 != A2 ),
    store(Result, A2 != A0 ),
    store(Result, A2 != A1 ),
    store(Result, A2 != A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R))
)";

void test_not_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile(not_equal_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const greater_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 > A0 ),
    store(Result, A0 > A1 ),
    store(Result, A0 > A2 ),
    store(Result, A1 > A0 ),
    store(Result, A1 > A1 ),
    store(Result, A1 > A2 ),
    store(Result, A2 > A0 ),
    store(Result, A2 > A1 ),
    store(Result, A2 > A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R))
)";

void test_greater()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(greater_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const greater_equal_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 >= A0 ),
    store(Result, A0 >= A1 ),
    store(Result, A0 >= A2 ),
    store(Result, A1 >= A0 ),
    store(Result, A1 >= A1 ),
    store(Result, A1 >= A2 ),
    store(Result, A2 >= A0 ),
    store(Result, A2 >= A1 ),
    store(Result, A2 >= A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R))
)";

void test_greater_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile(greater_equal_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const less_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 < A0 ),
    store(Result, A0 < A1 ),
    store(Result, A0 < A2 ),
    store(Result, A1 < A0 ),
    store(Result, A1 < A1 ),
    store(Result, A1 < A2 ),
    store(Result, A2 < A0 ),
    store(Result, A2 < A1 ),
    store(Result, A2 < A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R))
)";

void test_less()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(less_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const less_equal_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2, 3]),
    define(A2, [[3, 4, 5], [2, 7, 9]]),
    define(Result, 0),
    define(R, true),
    store(Result, A0 <= A0 ),
    store(Result, A0 <= A1 ),
    store(Result, A0 <= A2 ),
    store(Result, A1 <= A0 ),
    store(Result, A1 <= A1 ),
    store(Result, A1 <= A2 ),
    store(Result, A2 <= A0 ),
    store(Result, A2 <= A1 ),
    store(Result, A2 <= A2 ),
    if((any(A1!=[1, 2, 3]) || any(A2!=[[3, 4, 5], [2, 7, 9]])), store(R,false)),
    R))
)";

void test_less_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile(less_equal_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const dot_code = R"(define(test, block(
    define(A0, 3),
    define(A1, [1, 2]),
    define(A2, [[3, 4], [2, 7]]),
    define(Result, 0),
    define(R, true),
    store(Result, dot(A0 , A0)),
    store(Result, dot(A0 , A1)),
    store(Result, dot(A0 , A2)),
    store(Result, dot(A1 , A0)),
    store(Result, dot(A1 , A1)),
    store(Result, dot(A1 , A2)),
    store(Result, dot(A2 , A0)),
    store(Result, dot(A2 , A1)),
    store(Result, dot(A2 , A2)),
    if((any(A1!=[1, 2]) || any(A2!=[[3, 4], [2, 7]])), store(R,false)),
    R))
)";

void test_dot()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(dot_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const and_code = R"(define(test, block(
    define(A0, 0),
    define(A1, [0, 2]),
    define(A2, [[2, 3], [0, 4]]),
    define(Result, 0),
    define(R, true),
    store(Result, (A0 && A0)),
    store(Result, (A0 && A1)),
    store(Result, (A0 && A2)),
    store(Result, (A1 && A0)),
    store(Result, (A1 && A1)),
    store(Result, (A1 && A2)),
    store(Result, (A2 && A0)),
    store(Result, (A2 && A1)),
    store(Result, (A2 && A2)),
    if((any(A1!=[0, 2]) || any(A2!=[[2, 3], [0, 4]])), store(R,false)),
    R))
)";

void test_and()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(and_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const or_code = R"(define(test, block(
    define(A0, 0.0),
    define(A1, [0, 2]),
    define(A2, [[2, 3], [0, 4]]),
    define(Result, 0),
    define(R, true),
    store(Result, (A0 || A0)),
    store(Result, (A0 || A1)),
    store(Result, (A0 || A2)),
    store(Result, (A1 || A0)),
    store(Result, (A1 || A1)),
    store(Result, (A1 || A2)),
    store(Result, (A2 || A0)),
    store(Result, (A2 || A1)),
    store(Result, (A2 || A2)),
    if((any(A1!=[0, 2]) || any(A2!=[[2, 3], [0, 4]])), store(R,false)),
    R))
)";

void test_or()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(or_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const linear_solver_code = R"(define(test, block(
    define(A, [[2,-1,0],[-1,2,-1],[0,-1,1]]),
    define(b, [0, 0, 1]),
    define(R, true),
    define(Result, linear_solver_lu(A, b)),
    store(Result, linear_solver_ldlt(A, b)),
    store(Result, linear_solver_ldlt(A, b, "L")),
    store(Result, linear_solver_ldlt(A, b, "U")),
    store(Result, linear_solver_cholesky(A, b)),
    store(Result, linear_solver_cholesky(A, b, "L")),
    store(Result, linear_solver_cholesky(A, b, "U")),
    if((any(A!=[[2,-1,0],[-1,2,-1],[0,-1,1]]) || any(b!=[0, 0, 1])), store(R,false)),
    R))
)";

void test_linear_solver()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile(linear_solver_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()()), 1);
}

std::string const decomposition_code = R"(block(
    define(A, [[10, -10, 0], [-3, 15, 6], [5, 7, 5]]),
    define(R, true),
    define(Result, lu((A))),
    if((any(A!=[[10, -10, 0], [-3, 15, 6], [5, 7, 5]])), store(R, false)),
    R
))";

void test_decomposition()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code =
        phylanx::execution_tree::compile(decomposition_code, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_scalar_boolean_value(f()), 1);
}

int main(int argc, char* argv[])
{
    test_add();
    test_sub();
    test_mul();
    test_div();
    test_equal();
    test_not_equal();
    test_greater();
    test_greater_equal();
    test_less();
    test_less_equal();
    test_dot();
    test_and();
    test_or();
    test_linear_solver();
    test_decomposition();

    return hpx::util::report_errors();
}
