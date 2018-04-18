//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #245: PhySL Modifies Untouched Variable

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    R))
)";

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

void test_add()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(add_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_sub()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(sub_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_mul()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(mul_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_div()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(div_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(equal_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_not_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(not_equal_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_greater()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(greater_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_greater_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(greater_equal_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_less()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(less_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_less_equal()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(less_equal_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_dot()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(dot_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_and()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(and_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
}

void test_or()
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(or_code, snippets);

    HPX_TEST_EQ(phylanx::execution_tree::extract_boolean_value(f()), 1);
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

    return hpx::util::report_errors();
}
