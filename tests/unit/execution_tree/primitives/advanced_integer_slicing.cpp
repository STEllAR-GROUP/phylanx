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

void test_integer_slicing(char const* code, char const* expected)
{
    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));
    auto exprected_result = phylanx::execution_tree::extract_integer_value(
        compile_and_run(expected));

    HPX_TEST_EQ(result, exprected_result);
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_1d_0d()
{
    // direct array based indexing
    test_integer_slicing("slice(hstack(42), 0, nil)", "42");
    test_integer_slicing("slice(hstack(42), -1, nil)", "42");

    test_integer_slicing("slice(hstack(42, 43), 0, nil)", "42");
    test_integer_slicing("slice(hstack(42, 43), -1, nil)", "43");

    // indexing using a list of arrays
    test_integer_slicing("slice(hstack(42), list(0), nil)", "42");
    test_integer_slicing("slice(hstack(42), list(-1), nil)", "42");

    test_integer_slicing("slice(hstack(42, 43), list(0), nil)", "42");
    test_integer_slicing("slice(hstack(42, 43), list(-1), nil)", "43");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_1d_1d()
{
    // direct array based indexing
    test_integer_slicing(
        "slice(hstack(42), hstack(0), nil)", "hstack(42)");
    test_integer_slicing(
        "slice(hstack(42), hstack(-1), nil)", "hstack(42)");

    test_integer_slicing(
        "slice(hstack(42), hstack(0, 0), nil)", "hstack(42, 42)");
    test_integer_slicing(
        "slice(hstack(42), hstack(-1, 0), nil)", "hstack(42, 42)");

    test_integer_slicing(
        "slice(hstack(42, 43), hstack(0), nil)", "hstack(42)");
    test_integer_slicing(
        "slice(hstack(42, 43), hstack(-1), nil)", "hstack(43)");

    test_integer_slicing(
        "slice(hstack(42, 43), hstack(0, 1, 0), nil)", "hstack(42, 43, 42)");
    test_integer_slicing(
        "slice(hstack(42, 43), hstack(-2, -1, 0), nil)", "hstack(42, 43, 42)");

    // indexing using a list of arrays
    test_integer_slicing(
        "slice(hstack(42), list(hstack(0)), nil)", "hstack(42)");
    test_integer_slicing(
        "slice(hstack(42), list(hstack(-1)), nil)", "hstack(42)");

    test_integer_slicing(
        "slice(hstack(42), list(hstack(0, 0)), nil)", "hstack(42, 42)");
    test_integer_slicing(
        "slice(hstack(42), list(hstack(-1, 0)), nil)", "hstack(42, 42)");

    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(0)), nil)", "hstack(42)");
    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(-1)), nil)", "hstack(43)");

    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(0, 1, 0)), nil)",
        "hstack(42, 43, 42)");
    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(-2, -1, 0)), nil)",
        "hstack(42, 43, 42)");
}

///////////////////////////////////////////////////////////////////////////////
void test_integer_slicing_1d_2d()
{
    // direct array based indexing
    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(0)), nil)",
        "hstack(vstack(42))");
    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(-1)), nil)",
        "hstack(vstack(42))");

    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(0, 0)), nil)",
        "hstack(vstack(42, 42))");
    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(-1, 0)), nil)",
        "hstack(vstack(42, 42))");

    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(0), vstack(0)), nil)",
        "hstack(vstack(42), vstack(42))");
    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(-1), vstack(0)), nil)",
        "hstack(vstack(42), vstack(42))");

    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(0, 0), vstack(0, 0)), nil)",
        "hstack(vstack(42, 42), vstack(42, 42))");
    test_integer_slicing(
        "slice(hstack(42), hstack(vstack(-1, 0), vstack(0, -1)), nil)",
        "hstack(vstack(42, 42), vstack(42, 42))");

    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(0)), nil)",
        "hstack(vstack(42))");
    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(-1)), nil)",
        "hstack(vstack(43))");

    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(0, 1)), nil)",
        "hstack(vstack(42, 43))");
    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(-1, 0)), nil)",
        "hstack(vstack(43, 42))");

    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(0), vstack(1)), nil)",
        "hstack(vstack(42), vstack(43))");
    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(-1), vstack(0)), nil)",
        "hstack(vstack(43), vstack(42))");

    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(0, 1, 0), vstack(0, 0, 1)), nil)",
        "hstack(vstack(42, 43, 42), vstack(42, 42, 43))");
    test_integer_slicing(
        "slice(hstack(42, 43), hstack(vstack(-2, -1, 0), vstack(0, -2, -1)), nil)",
        "hstack(vstack(42, 43, 42), vstack(42, 42, 43))");

    // indexing using a list of arrays
    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(0))), nil)",
        "hstack(vstack(42))");
    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(-1))), nil)",
        "hstack(vstack(42))");

    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(0, 0))), nil)",
        "hstack(vstack(42, 42))");
    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(-1, 0))), nil)",
        "hstack(vstack(42, 42))");

    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(0), vstack(0))), nil)",
        "hstack(vstack(42), vstack(42))");
    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(-1), vstack(0))), nil)",
        "hstack(vstack(42), vstack(42))");

    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(0, 0), vstack(0, 0))), nil)",
        "hstack(vstack(42, 42), vstack(42, 42))");
    test_integer_slicing(
        "slice(hstack(42), list(hstack(vstack(-1, 0), vstack(0, -1))), nil)",
        "hstack(vstack(42, 42), vstack(42, 42))");

    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(0))), nil)",
        "hstack(vstack(42))");
    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(-1))), nil)",
        "hstack(vstack(43))");

    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(0, 1))), nil)",
        "hstack(vstack(42, 43))");
    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(-1, 0))), nil)",
        "hstack(vstack(43, 42))");

    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(0), vstack(1))), nil)",
        "hstack(vstack(42), vstack(43))");
    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(-1), vstack(0))), nil)",
        "hstack(vstack(43), vstack(42))");

    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(0, 1, 0), "
            "vstack(0, 0, 1))), nil)",
        "hstack(vstack(42, 43, 42), vstack(42, 42, 43))");
    test_integer_slicing(
        "slice(hstack(42, 43), list(hstack(vstack(-2, -1, 0), "
            "vstack(0, -2, -1))), nil)",
        "hstack(vstack(42, 43, 42), vstack(42, 42, 43))");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_integer_slicing_1d_0d();   // use 0d value as index for 1d array
    test_integer_slicing_1d_1d();   // use 1d value as index for 1d array
    test_integer_slicing_1d_2d();   // use 2d value as index for 1d array

    return hpx::util::report_errors();
}
