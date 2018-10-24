// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    return code.run();
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
    test_boolean_slicing("slice(hstack(42), true, nil)", "hstack(42)");
    test_boolean_slicing("slice(hstack(42), false, nil)", "hstack()");

    test_boolean_slicing(
        "slice(hstack(42, 43), true, nil)", "hstack(42, 43)");
    test_boolean_slicing(
        "slice(hstack(42, 43), false, nil)", "hstack()");
}

// indexing using a list of arrays
void test_boolean_slicing_1d_0d_list()
{
    test_boolean_slicing(
        "slice(hstack(42), list(true), nil)", "hstack(42)");
    test_boolean_slicing(
        "slice(hstack(42), list(false), nil)", "hstack()");

    test_boolean_slicing(
        "slice(hstack(42, 43), list(true), nil)", "hstack(42, 43)");
    test_boolean_slicing("slice(hstack(42, 43), list(false), nil)", "hstack()");
}

///////////////////////////////////////////////////////////////////////////////
void test_boolean_slicing_1d_1d()
{
    // direct array based indexing
    test_boolean_slicing(
        "slice(hstack(42), hstack(true), nil)", "hstack(42)");
    test_boolean_slicing(
        "slice(hstack(42), hstack(false), nil)", "hstack()");

    test_boolean_slicing(
        "slice(hstack(42, 43), hstack(true, true), nil)", "hstack(42, 43)");
    test_boolean_slicing(
        "slice(hstack(42, 43), hstack(false, true), nil)", "hstack(43)");
}

void test_boolean_slicing_1d_1d_list()
{
    // direct array based indexing
    test_boolean_slicing(
        "slice(hstack(42), list(hstack(true)), nil)", "hstack(42)");
    test_boolean_slicing(
        "slice(hstack(42), list(hstack(false)), nil)", "hstack()");

    test_boolean_slicing("slice(hstack(42, 43), list(hstack(true, true)), nil)",
        "hstack(42, 43)");
    test_boolean_slicing(
        "slice(hstack(42, 43), list(hstack(false, true)), nil)", "hstack(43)");
}

///////////////////////////////////////////////////////////////////////////////
void test_boolean_slicing_1d_0d_store()
{
    // direct array based indexing
    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, true, nil), 43),
        v
    ))", "hstack(43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, false, nil), 43),
        v
    ))", "hstack(42)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, true, nil), 43),
        v
    ))", "hstack(43, 43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, false, nil), 43),
        v
    ))", "hstack(42, 42)");
}

void test_boolean_slicing_1d_0d_list_store()
{
    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, list(true), nil), 43),
        v
    ))", "hstack(43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, list(false), nil), 43),
        v
    ))", "hstack(42)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, list(true), nil), 43),
        v
    ))", "hstack(43, 43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, list(false), nil), 43),
        v
    ))", "hstack(42, 42)");
}

void test_boolean_slicing_1d_1d_store()
{
    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, hstack(true), nil), 43),
        v
    ))", "hstack(43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, hstack(false), nil), 43),
        v
    ))", "hstack(42)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, hstack(true, true), nil), 43),
        v
    ))", "hstack(43, 43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, hstack(false, true), nil), 43),
        v
    ))", "hstack(42, 43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, hstack(false, false), nil), 43),
        v
    ))", "hstack(42, 42)");
}

void test_boolean_slicing_1d_1d_list_store()
{
    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, list(hstack(true)), nil), 43),
        v
    ))", "hstack(43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42)),
        store(slice(v, list(hstack(false)), nil), 43),
        v
    ))", "hstack(42)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, list(hstack(true, true)), nil), 43),
        v
    ))", "hstack(43, 43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, list(hstack(false, true)), nil), 43),
        v
    ))", "hstack(42, 43)");

    test_boolean_slicing(R"(block(
        define(v, hstack(42, 42)),
        store(slice(v, list(hstack(false, false)), nil), 43),
        v
    ))", "hstack(42, 42)");
}

///////////////////////////////////////////////////////////////////////////////
// void test_boolean_slicing_2d_0d()
// {
//     // direct array based indexing
//     test_boolean_slicing(
//         "slice(hstack(vstack(42, 43)), 0, nil)", "hstack(42)");
//     test_boolean_slicing(
//         "slice(hstack(vstack(42), vstack(43)), 0, nil)", "hstack(42, 43)");
//
//     test_boolean_slicing(
//         "slice(hstack(vstack(42, 43)), nil, 0)", "hstack(vstack(42))");
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
