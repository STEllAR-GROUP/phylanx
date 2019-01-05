// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

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
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_where_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}

///////////////////////////////////////////////////////////////////////////////
void test_one_argument_where()
{
    // test 0d data (scalars)
    test_where_operation("where(false)", "list(hstack__int())");
    test_where_operation("where(0)", "list(hstack__int())");
    test_where_operation("where(0.)", "list(hstack__int())");
    test_where_operation("where(true)", "list(hstack(0))");
    test_where_operation("where(1)", "list(hstack(0))");
    test_where_operation("where(1.)", "list(hstack(0))");

    // test 1d data (vectors)
    test_where_operation("where(hstack())", "list(hstack__int())");

    test_where_operation("where(hstack(false))", "list(hstack__int())");
    test_where_operation("where(hstack(0))", "list(hstack__int())");
    test_where_operation("where(hstack(0.))", "list(hstack__int())");

    test_where_operation("where(hstack(true))", "list(hstack(0))");
    test_where_operation("where(hstack(1))", "list(hstack(0))");
    test_where_operation("where(hstack(1.))", "list(hstack(0))");

    test_where_operation(
        "where(hstack(false, true, false, true))", "list(hstack(1, 3))");
    test_where_operation("where(hstack(0, 1, 2, 0))", "list(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1., 0., 42., 43.))", "list(hstack(0, 2, 3))");

    // test 2d data (matrix)
    test_where_operation(
        "where(vstack(hstack()))", "list(hstack__int(), hstack__int())");
    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)))",
        "list(hstack__int(), hstack__int())");

    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(2, 0)))",
        "list(hstack(0, 1), hstack(1, 0))");
    test_where_operation(
        "where(vstack(hstack(0., 1.), hstack(2., 0.)))",
        "list(hstack(0, 1), hstack(1, 0))");
}

void test_three_argument_where_0d()
{
    ///////////////////////////////////////////////////////////////////////////
    // test scalar result
    test_where_operation("where(false, 1, 2)", "2");
    test_where_operation("where(true, 1, 2)", "1");

    ///////////////////////////////////////////////////////////////////////////
    // test vector result
    test_where_operation("where(false, hstack(1), hstack(2))", "hstack(2)");
    test_where_operation("where(true, hstack(1), hstack(2))", "hstack(1)");

    test_where_operation(
        "where(false, hstack(1, 2), hstack(2, 3))", "hstack(2, 3)");
    test_where_operation(
        "where(true, hstack(1, 2), hstack(2, 3))", "hstack(1, 2)");

    // test vector result with broadcasting a scalar
    test_where_operation("where(false, hstack(1), 2)", "hstack(2)");
    test_where_operation("where(true, hstack(1), 2)", "hstack(1)");
    test_where_operation("where(false, 1, hstack(2))", "hstack(2)");
    test_where_operation("where(true, 1, hstack(2))", "hstack(1)");

    test_where_operation("where(false, hstack(1, 2), 2)", "hstack(2, 2)");
    test_where_operation("where(true, hstack(1, 2), 2)", "hstack(1, 2)");
    test_where_operation("where(false, 1, hstack(2, 3))", "hstack(2, 3)");
    test_where_operation("where(true, 1, hstack(2, 3))", "hstack(1, 1)");

    // test vector result with broadcasting a single element vector
    test_where_operation(
        "where(false, hstack(1, 2), hstack(2))", "hstack(2, 2)");
    test_where_operation(
        "where(true, hstack(1, 2), hstack(2))", "hstack(1, 2)");
    test_where_operation(
        "where(false, hstack(2), hstack(2, 3))", "hstack(2, 3)");
    test_where_operation(
        "where(true, hstack(2), hstack(2, 3))", "hstack(2, 2)");

    ///////////////////////////////////////////////////////////////////////////
    // test matrix result
    test_where_operation(
        R"(where(false,
            vstack(hstack(1), hstack(2)), vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(2), hstack(1))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1), hstack(2)), vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(1), hstack(2))");

    // test matrix result with broadcasting a scalar
    test_where_operation(
        R"(where(false,
            vstack(hstack(1), hstack(2)), 3
          ))", "vstack(hstack(3), hstack(3))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1), hstack(2)), 3
          ))", "vstack(hstack(1), hstack(2))");
    test_where_operation(
        R"(where(false,
            3, vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(2), hstack(1))");
    test_where_operation(
        R"(where(true,
            3, vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(3), hstack(3))");

    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2), hstack(2, 3)), 3
          ))", "vstack(hstack(3, 3), hstack(3, 3))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2), hstack(2, 3)), 3
          ))", "vstack(hstack(1, 2), hstack(2, 3))");
    test_where_operation(
        R"(where(false,
            3, vstack(hstack(1, 2), hstack(2, 3))
          ))", "vstack(hstack(1, 2), hstack(2, 3))");
    test_where_operation(
        R"(where(true,
            3, vstack(hstack(1, 2), hstack(2, 3))
          ))", "vstack(hstack(3, 3), hstack(3, 3))");

    // test matrix result with broadcasting a single element vector
    test_where_operation(
        R"(where(false,
            vstack(hstack(1), hstack(2)), hstack(3)
          ))", "vstack(hstack(3), hstack(3))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1), hstack(2)), hstack(3)
          ))", "vstack(hstack(1), hstack(2))");
    test_where_operation(
        R"(where(false,
            hstack(3), vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(2), hstack(1))");
    test_where_operation(
        R"(where(true,
            hstack(3), vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(3), hstack(3))");

    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2), hstack(2, 3)), hstack(3)
          ))", "vstack(hstack(3, 3), hstack(3, 3))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2), hstack(2, 3)), hstack(3)
          ))", "vstack(hstack(1, 2), hstack(2, 3))");
    test_where_operation(
        R"(where(false,
            hstack(3), vstack(hstack(1, 2), hstack(2, 3))
          ))", "vstack(hstack(1, 2), hstack(2, 3))");
    test_where_operation(
        R"(where(true,
            hstack(3), vstack(hstack(1, 2), hstack(2, 3))
          ))", "vstack(hstack(3, 3), hstack(3, 3))");

    // test matrix result with broadcasting a vector
    test_where_operation(
        R"(where(false,
            vstack(hstack(1), hstack(2)), hstack(3, 4)
          ))", "vstack(hstack(3, 4), hstack(3, 4))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1), hstack(2)), hstack(3, 4)
          ))", "vstack(hstack(1, 1), hstack(2, 2))");
    test_where_operation(
        R"(where(false,
            hstack(3, 4), vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(2, 2), hstack(1, 1))");
    test_where_operation(
        R"(where(true,
            hstack(3, 4), vstack(hstack(2), hstack(1))
          ))", "vstack(hstack(3, 4), hstack(3, 4))");

    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2)), hstack(3, 4)
          ))", "vstack(hstack(3, 4))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2)), hstack(3, 4)
          ))", "vstack(hstack(1, 2))");
    test_where_operation(
        R"(where(false,
            hstack(3, 4), vstack(hstack(2, 1))
          ))", "vstack(hstack(2, 1))");
    test_where_operation(
        R"(where(true,
            hstack(3, 4), vstack(hstack(2, 1))
          ))", "vstack(hstack(3, 4))");

    // test matrix result with broadcasting a matrix
    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2)), vstack(hstack(3))
          ))", "vstack(hstack(3, 3))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2)), vstack(hstack(3))
          ))", "vstack(hstack(1, 2))");
    test_where_operation(
        R"(where(false,
            vstack(hstack(3)), vstack(hstack(2, 1))
          ))", "vstack(hstack(2, 1))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(3)), vstack(hstack(2, 1))
          ))", "vstack(hstack(3, 3))");

    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2)), vstack(hstack(3, 4))
          ))", "vstack(hstack(3, 4))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2)), vstack(hstack(3, 4))
          ))", "vstack(hstack(1, 2))");
    test_where_operation(
        R"(where(false,
            vstack(hstack(3, 4)), vstack(hstack(2, 1))
          ))", "vstack(hstack(2, 1))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(3, 4)), vstack(hstack(2, 1))
          ))", "vstack(hstack(3, 4))");

    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2), hstack(3, 4)), vstack(hstack(5, 6))
          ))", "vstack(hstack(5, 6), hstack(5, 6))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2), hstack(3, 4)), vstack(hstack(5, 6))
          ))", "vstack(hstack(1, 2), hstack(3, 4))");
    test_where_operation(
        R"(where(false,
            vstack(hstack(5, 6)), vstack(hstack(1, 2), hstack(3, 4))
          ))", "vstack(hstack(1, 2), hstack(3, 4))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(5, 6)), vstack(hstack(1, 2), hstack(3, 4))
          ))", "vstack(hstack(5, 6), hstack(5, 6))");

    test_where_operation(
        R"(where(false,
            vstack(hstack(1, 2), hstack(3, 4)), vstack(hstack(5), hstack(6))
          ))", "vstack(hstack(5, 5), hstack(6, 6))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(1, 2), hstack(3, 4)), vstack(hstack(5), hstack(6))
          ))", "vstack(hstack(1, 2), hstack(3, 4))");
    test_where_operation(
        R"(where(false,
            vstack(hstack(5), hstack(6)), vstack(hstack(1, 2), hstack(3, 4))
          ))", "vstack(hstack(1, 2), hstack(3, 4))");
    test_where_operation(
        R"(where(true,
            vstack(hstack(5), hstack(6)), vstack(hstack(1, 2), hstack(3, 4))
          ))", "vstack(hstack(5, 5), hstack(6, 6))");
}

void test_three_argument_where_1d()
{
    ///////////////////////////////////////////////////////////////////////////
    // test scalar right hand sides
    test_where_operation("where(hstack(0, 0), 1, 2)", "hstack(2, 2)");
    test_where_operation("where(hstack(0, 1), 1, 2)", "hstack(2, 1)");
    test_where_operation("where(hstack(1, 0), 1, 2)", "hstack(1, 2)");
    test_where_operation("where(hstack(1, 1), 1, 2)", "hstack(1, 1)");

    test_where_operation("where(hstack(0, 0), hstack(1), 2)", "hstack(2, 2)");
    test_where_operation("where(hstack(0, 1), hstack(1), 2)", "hstack(2, 1)");
    test_where_operation("where(hstack(1, 0), 1, hstack(2))", "hstack(1, 2)");
    test_where_operation("where(hstack(1, 1), 1, hstack(2))", "hstack(1, 1)");

    test_where_operation("where(hstack(0, 0), hstack(1, 2), 3)", "hstack(3, 3)");
    test_where_operation("where(hstack(0, 1), hstack(1, 2), 3)", "hstack(3, 2)");
    test_where_operation("where(hstack(1, 0), hstack(1, 2), 3)", "hstack(1, 3)");
    test_where_operation("where(hstack(1, 1), hstack(1, 2), 3)", "hstack(1, 2)");

    test_where_operation("where(hstack(0, 0), 1, hstack(2, 3))", "hstack(2, 3)");
    test_where_operation("where(hstack(0, 1), 1, hstack(2, 3))", "hstack(2, 1)");
    test_where_operation("where(hstack(1, 0), 1, hstack(2, 3))", "hstack(1, 3)");
    test_where_operation("where(hstack(1, 1), 1, hstack(2, 3))", "hstack(1, 1)");

    ///////////////////////////////////////////////////////////////////////////
    // test vector right hand sides
    test_where_operation(
        "where(hstack(0, 0), hstack(1), hstack(2))", "hstack(2, 2)");
    test_where_operation(
        "where(hstack(0, 1), hstack(1), hstack(2))", "hstack(2, 1)");
    test_where_operation(
        "where(hstack(1, 0), hstack(1), hstack(2))", "hstack(1, 2)");
    test_where_operation(
        "where(hstack(1, 1), hstack(1), hstack(2))", "hstack(1, 1)");

    test_where_operation(
        "where(hstack(0, 0), hstack(1, 2), hstack(3))", "hstack(3, 3)");
    test_where_operation(
        "where(hstack(0, 1), hstack(1, 2), hstack(3))", "hstack(3, 2)");
    test_where_operation(
        "where(hstack(1, 0), hstack(1, 2), hstack(3))", "hstack(1, 3)");
    test_where_operation(
        "where(hstack(1, 1), hstack(1, 2), hstack(3))", "hstack(1, 2)");

    test_where_operation(
        "where(hstack(0, 0), hstack(1), hstack(2, 3))", "hstack(2, 3)");
    test_where_operation(
        "where(hstack(0, 1), hstack(1), hstack(2, 3))", "hstack(2, 1)");
    test_where_operation(
        "where(hstack(1, 0), hstack(1), hstack(2, 3))", "hstack(1, 3)");
    test_where_operation(
        "where(hstack(1, 1), hstack(1), hstack(2, 3))", "hstack(1, 1)");

    test_where_operation(
        "where(hstack(0, 0), hstack(1, 2), hstack(3, 4))", "hstack(3, 4)");
    test_where_operation(
        "where(hstack(0, 1), hstack(1, 2), hstack(3, 4))", "hstack(3, 2)");
    test_where_operation(
        "where(hstack(1, 0), hstack(1, 2), hstack(3, 4))", "hstack(1, 4)");
    test_where_operation(
        "where(hstack(1, 1), hstack(1, 2), hstack(3, 4))", "hstack(1, 2)");

    test_where_operation(
        "where(hstack(0, 0), hstack(1, 4), hstack(2, 3))", "hstack(2, 3)");
    test_where_operation(
        "where(hstack(0, 1), hstack(1, 4), hstack(2, 3))", "hstack(2, 4)");
    test_where_operation(
        "where(hstack(1, 0), hstack(1, 4), hstack(2, 3))", "hstack(1, 3)");
    test_where_operation(
        "where(hstack(1, 1), hstack(1, 4), hstack(2, 3))", "hstack(1, 4)");

    ///////////////////////////////////////////////////////////////////////////
    // test matrix right hand sides
    test_where_operation(
        "where(hstack(0, 0), vstack(hstack(1)), 2)",
        "vstack(hstack(2, 2))");
    test_where_operation(
        "where(hstack(0, 1), vstack(hstack(1)), 2)",
        "vstack(hstack(2, 1))");
    test_where_operation(
        "where(hstack(1, 0), vstack(hstack(1)), 2)",
        "vstack(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1, 1), vstack(hstack(1)), 2)",
        "vstack(hstack(1, 1))");

    test_where_operation(
        "where(hstack(0, 0), 2, vstack(hstack(1)))",
        "vstack(hstack(1, 1))");
    test_where_operation(
        "where(hstack(0, 1), 2, vstack(hstack(1)))",
        "vstack(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1, 0), 2, vstack(hstack(1)))",
        "vstack(hstack(2, 1))");
    test_where_operation(
        "where(hstack(1, 1), 2, vstack(hstack(1)))",
        "vstack(hstack(2, 2))");

    test_where_operation(
        "where(hstack(0, 0), vstack(hstack(1)), hstack(2))",
        "vstack(hstack(2, 2))");
    test_where_operation(
        "where(hstack(0, 1), vstack(hstack(1)), hstack(2))",
        "vstack(hstack(2, 1))");
    test_where_operation(
        "where(hstack(1, 0), vstack(hstack(1)), hstack(2))",
        "vstack(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1, 1), vstack(hstack(1)), hstack(2))",
        "vstack(hstack(1, 1))");

    test_where_operation(
        "where(hstack(0, 0), hstack(2), vstack(hstack(1)))",
        "vstack(hstack(1, 1))");
    test_where_operation(
        "where(hstack(0, 1), hstack(2), vstack(hstack(1)))",
        "vstack(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1, 0), hstack(2), vstack(hstack(1)))",
        "vstack(hstack(2, 1))");
    test_where_operation(
        "where(hstack(1, 1), hstack(2), vstack(hstack(1)))",
        "vstack(hstack(2, 2))");

    test_where_operation(
        "where(hstack(0, 0), vstack(hstack(1)), vstack(hstack(2)))",
        "vstack(hstack(2, 2))");
    test_where_operation(
        "where(hstack(0, 1), vstack(hstack(1)), vstack(hstack(2)))",
        "vstack(hstack(2, 1))");
    test_where_operation(
        "where(hstack(1, 0), vstack(hstack(1)), vstack(hstack(2)))",
        "vstack(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1, 1), vstack(hstack(1)), vstack(hstack(2)))",
        "vstack(hstack(1, 1))");

    test_where_operation(
        "where(hstack(0, 0), vstack(hstack(2)), vstack(hstack(1)))",
        "vstack(hstack(1, 1))");
    test_where_operation(
        "where(hstack(0, 1), vstack(hstack(2)), vstack(hstack(1)))",
        "vstack(hstack(1, 2))");
    test_where_operation(
        "where(hstack(1, 0), vstack(hstack(2)), vstack(hstack(1)))",
        "vstack(hstack(2, 1))");
    test_where_operation(
        "where(hstack(1, 1), vstack(hstack(2)), vstack(hstack(1)))",
        "vstack(hstack(2, 2))");

    test_where_operation(
        "where(hstack(0, 0), vstack(hstack(1), hstack(2)), vstack(hstack(3)))",
        "vstack(hstack(3, 3), hstack(3, 3))");
    test_where_operation(
        "where(hstack(0, 1), vstack(hstack(1), hstack(2)), vstack(hstack(3)))",
        "vstack(hstack(3, 1), hstack(3, 2))");
    test_where_operation(
        "where(hstack(1, 0), vstack(hstack(1), hstack(2)), vstack(hstack(3)))",
        "vstack(hstack(1, 3), hstack(2, 3))");
    test_where_operation(
        "where(hstack(1, 1), vstack(hstack(1), hstack(2)), vstack(hstack(3)))",
        "vstack(hstack(1, 1), hstack(2, 2))");

    test_where_operation(
        "where(hstack(0, 0), vstack(hstack(1, 2)), vstack(hstack(3)))",
        "vstack(hstack(3, 3))");
    test_where_operation(
        "where(hstack(0, 1), vstack(hstack(1, 2)), vstack(hstack(3)))",
        "vstack(hstack(3, 2))");
    test_where_operation(
        "where(hstack(1, 0), vstack(hstack(1, 2)), vstack(hstack(3)))",
        "vstack(hstack(1, 3))");
    test_where_operation(
        "where(hstack(1, 1), vstack(hstack(1, 2)), vstack(hstack(3)))",
        "vstack(hstack(1, 2))");
}

void test_three_argument_where_2d()
{
    ///////////////////////////////////////////////////////////////////////////
    // test scalar right hand sides
    test_where_operation("where(vstack(hstack(0, 0), hstack(0, 0)), 1, 2)",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation("where(vstack(hstack(0, 1), hstack(1, 0)), 1, 2)",
        "vstack(hstack(2, 1), hstack(1, 2))");
    test_where_operation("where(vstack(hstack(1, 0), hstack(0, 1)), 1, 2)",
        "vstack(hstack(1, 2), hstack(2, 1))");
    test_where_operation("where(vstack(hstack(1, 1), hstack(1, 1)), 1, 2)",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), hstack(1), 2)",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), hstack(1), 2)",
        "vstack(hstack(2, 1), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), hstack(1), 2)",
        "vstack(hstack(1, 2), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), hstack(1), 2)",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), 1, hstack(2))",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), 1, hstack(2))",
        "vstack(hstack(2, 1), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), 1, hstack(2))",
        "vstack(hstack(1, 2), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), 1, hstack(2))",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), vstack(hstack(1)), 2)",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), vstack(hstack(1)), 2)",
        "vstack(hstack(2, 1), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), vstack(hstack(1)), 2)",
        "vstack(hstack(1, 2), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), vstack(hstack(1)), 2)",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), 1, vstack(hstack(2)))",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), 1, vstack(hstack(2)))",
        "vstack(hstack(2, 1), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), 1, vstack(hstack(2)))",
        "vstack(hstack(1, 2), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), 1, vstack(hstack(2)))",
        "vstack(hstack(1, 1), hstack(1, 1))");

    ///////////////////////////////////////////////////////////////////////////
    // test vector right hand sides
    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), hstack(1), hstack(2))",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), hstack(1), hstack(2))",
        "vstack(hstack(2, 1), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), hstack(1), hstack(2))",
        "vstack(hstack(1, 2), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), hstack(1), hstack(2))",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), hstack(1, 3), hstack(2))",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), hstack(1, 3), hstack(2))",
        "vstack(hstack(2, 3), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), hstack(1, 3), hstack(2))",
        "vstack(hstack(1, 2), hstack(2, 3))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), hstack(1, 3), hstack(2))",
        "vstack(hstack(1, 3), hstack(1, 3))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), hstack(1), hstack(2, 3))",
        "vstack(hstack(2, 3), hstack(2, 3))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), hstack(1), hstack(2, 3))",
        "vstack(hstack(2, 1), hstack(1, 3))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), hstack(1), hstack(2, 3))",
        "vstack(hstack(1, 3), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), hstack(1), hstack(2, 3))",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), "
            "vstack(hstack(1)), hstack(2, 3))",
        "vstack(hstack(2, 3), hstack(2, 3))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), "
            "vstack(hstack(1)), hstack(2, 3))",
        "vstack(hstack(2, 1), hstack(1, 3))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), "
            "vstack(hstack(1)), hstack(2, 3))",
        "vstack(hstack(1, 3), hstack(2, 1))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), "
            "vstack(hstack(1)), hstack(2, 3))",
        "vstack(hstack(1, 1), hstack(1, 1))");

    test_where_operation(
        "where(vstack(hstack(0, 0), hstack(0, 0)), "
            "hstack(1, 3), vstack(hstack(2)))",
        "vstack(hstack(2, 2), hstack(2, 2))");
    test_where_operation(
        "where(vstack(hstack(0, 1), hstack(1, 0)), "
            "hstack(1, 3), vstack(hstack(2)))",
        "vstack(hstack(2, 3), hstack(1, 2))");
    test_where_operation(
        "where(vstack(hstack(1, 0), hstack(0, 1)), "
            "hstack(1, 3), vstack(hstack(2)))",
        "vstack(hstack(1, 2), hstack(2, 3))");
    test_where_operation(
        "where(vstack(hstack(1, 1), hstack(1, 1)), "
            "hstack(1, 3), vstack(hstack(2)))",
        "vstack(hstack(1, 3), hstack(1, 3))");
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_one_argument_where();
    test_three_argument_where_0d();
    test_three_argument_where_1d();
    test_three_argument_where_2d();

    return hpx::util::report_errors();
}
