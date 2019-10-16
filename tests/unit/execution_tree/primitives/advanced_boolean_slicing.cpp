// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

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

void test_boolean_slicing(char const* code, char const* expected)
{
    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));
    auto exprected_result = phylanx::execution_tree::extract_integer_value(
        compile_and_run(expected));

    HPX_TEST_EQ(result, exprected_result);
}

///////////////////////////////////////////////////////////////////////////////
// direct array based indexing
void test_boolean_slicing_1d_0d()
{
    test_boolean_slicing("slice([42], true, nil)", "[42]");
    test_boolean_slicing("slice([42], false, nil)", "[]");

    test_boolean_slicing(
        "slice([42, 43], true, nil)", "[42, 43]");
    test_boolean_slicing(
        "slice([42, 43], false, nil)", "[]");
}

// indexing using a list of arrays
void test_boolean_slicing_1d_0d_list()
{
    test_boolean_slicing(
        "slice([42], list(true), nil)", "[42]");
    test_boolean_slicing(
        "slice([42], list(false), nil)", "[]");

    test_boolean_slicing(
        "slice([42, 43], list(true), nil)", "[42, 43]");
    test_boolean_slicing("slice([42, 43], list(false), nil)", "[]");
}

///////////////////////////////////////////////////////////////////////////////
void test_boolean_slicing_1d_1d()
{
    // direct array based indexing
    test_boolean_slicing(
        "slice([42], hstack(list(true)), nil)", "[42]");
    test_boolean_slicing(
        "slice([42], hstack(list(false)), nil)", "[]");

    test_boolean_slicing(
        "slice([42, 43], hstack(list(true, true)), nil)", "[42, 43]");
    test_boolean_slicing(
        "slice([42, 43], hstack(list(false, true)), nil)", "[43]");
}

void test_boolean_slicing_1d_1d_list()
{
    // direct array based indexing
    test_boolean_slicing(
        "slice([42], list(hstack(list(true))), nil)", "[42]");
    test_boolean_slicing(
        "slice([42], list(hstack(list(false))), nil)", "[]");

    test_boolean_slicing("slice([42, 43], list(hstack(list(true, true))), nil)",
        "[42, 43]");
    test_boolean_slicing(
        "slice([42, 43], list(hstack(list(false, true))), nil)", "[43]");
}

///////////////////////////////////////////////////////////////////////////////
void test_boolean_slicing_1d_0d_store()
{
    // direct array based indexing
    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, true, nil), 43),
        v
    ))", "[43]");

    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, false, nil), 43),
        v
    ))", "[42]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, true, nil), 43),
        v
    ))", "[43, 43]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, false, nil), 43),
        v
    ))", "[42, 42]");
}

void test_boolean_slicing_1d_0d_list_store()
{
    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, list(true), nil), 43),
        v
    ))", "[43]");

    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, list(false), nil), 43),
        v
    ))", "[42]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, list(true), nil), 43),
        v
    ))", "[43, 43]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, list(false), nil), 43),
        v
    ))", "[42, 42]");
}

void test_boolean_slicing_1d_1d_store()
{
    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, hstack(list(true)), nil), 43),
        v
    ))", "[43]");

    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, hstack(list(false)), nil), 43),
        v
    ))", "[42]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, hstack(list(true, true)), nil), 43),
        v
    ))", "[43, 43]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, hstack(list(false, true)), nil), 43),
        v
    ))", "[42, 43]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, hstack(list(false, false)), nil), 43),
        v
    ))", "[42, 42]");
}

void test_boolean_slicing_1d_1d_list_store()
{
    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, list(hstack(list(true))), nil), 43),
        v
    ))", "[43]");

    test_boolean_slicing(R"(block(
        define(v, [42]),
        store(slice(v, list(hstack(list(false))), nil), 43),
        v
    ))", "[42]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, list(hstack(list(true, true))), nil), 43),
        v
    ))", "[43, 43]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, list(hstack(list(false, true))), nil), 43),
        v
    ))", "[42, 43]");

    test_boolean_slicing(R"(block(
        define(v, [42, 42]),
        store(slice(v, list(hstack(list(false, false))), nil), 43),
        v
    ))", "[42, 42]");
}

///////////////////////////////////////////////////////////////////////////////
// void test_boolean_slicing_2d_0d()
// {
//     // direct array based indexing
//     test_boolean_slicing(
//         "slice([[42, 43]], 0, nil)", "[42]");
//     test_boolean_slicing(
//         "slice([[42], vstack(43]), 0, nil)", "[42, 43]");
//
//     test_boolean_slicing(
//         "slice([[42, 43]], nil, 0)", "[[42]]");
// }

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_boolean_slicing_1d_0d();   // use 0d value as index for 1d array
    test_boolean_slicing_1d_0d_list();

    test_boolean_slicing_1d_1d();   // use 1d value as index for 1d array
    test_boolean_slicing_1d_1d_list();

    test_boolean_slicing_1d_0d_store();   // use 0d value as index for 1d array
    test_boolean_slicing_1d_0d_list_store();

    test_boolean_slicing_1d_1d_store();   // use 1d value as index for 1d array
    test_boolean_slicing_1d_1d_list_store();

//     test_boolean_slicing_2d_0d();   // use 0d value as index for 2d array

    return hpx::util::report_errors();
}
