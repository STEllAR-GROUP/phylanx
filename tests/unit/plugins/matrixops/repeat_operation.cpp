// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>

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
void test_repeat_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}


///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_repeat_operation("repeat(42., 4)", "[42.,42.,42.,42.]");
    test_repeat_operation("repeat(42., [4], 0)", "[42.,42.,42.,42.]");
    test_repeat_operation(
        "repeat([42, 13, 33], 3, -1)", "[42, 42, 42, 13, 13, 13, 33, 33, 33]");
    test_repeat_operation("repeat([42, 13, 33], [3])",
        "[42, 42, 42, 13, 13, 13, 33, 33, 33]");
    test_repeat_operation(
        "repeat([42, 13, 33], [3, 0, 2], 0)", "[42, 42, 42, 33, 33]");
    test_repeat_operation(
        "repeat([42, 13, 33], [0, 3, 0], -1)", "[13, 13, 13]");
    test_repeat_operation("repeat([[42, 13, 33],[5, 44, 6]], [3, 2], 0)",
        "[[42, 13, 33],[42, 13, 33],[42, 13, 33],[5, 44, 6],[5, 44, 6]]");
    test_repeat_operation("repeat([[42, 13, 33],[5, 44, 6]], [2, 0, 3], 1)",
        "[[42, 42, 33, 33, 33],[ 5,  5,  6,  6,  6]]");
    test_repeat_operation(
        "repeat([[42, 13, 33],[5, 44, 6]], [0, 1, 3, 2, 2, 0])",
        "[13, 33, 33, 33,  5,  5, 44, 44]");
    test_repeat_operation("repeat([[42, 13, 33],[5, 44, 6]], 2)",
        "[42, 42, 13, 13, 33, 33,  5,  5, 44, 44,  6,  6]");

    //axis 0
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2, 0)",
        "[[[42, 13],[33,  4]], [[42, 13],[33,  4]], [[ 5, 44],[ 6, 23]], "
        "[[ 5, 44],[ 6, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2, -3)",
        "[[[42, 13],[33,  4]], [[42, 13],[33,  4]], [[ 5, 44],[ 6, 23]], "
        "[[ 5, 44],[ 6, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13, 5],[33, 4, 23]]], [3], 0)",
        "[[[42, 13,  5],[33,  4, 23]], [[42, 13,  5],[33,  4, 23]],"
        "[[42, 13,  5],[33,  4, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], [1, 2], 0)",
        "[[[42, 13],[33,  4]], [[ 5, 44],[ 6, 23]], [[ 5, 44],[ 6, 23]]]");

    //axis 1
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2, 1)",
        "[[[42, 13],[42, 13],[33,  4],[33,  4]], [[ 5, 44],[ 5, 44],[6, 23] "
        ",[ 6, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2, -2)",
        "[[[42, 13],[42, 13],[33,  4],[33,  4]], [[ 5, 44],[ 5, 44],[6, 23] "
        ",[ 6, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13, 5],[33, 4, 23]]], [3], 1)",
        "[[[42, 13,  5],[42, 13,  5],[42, 13,  5],[33,  4, 23],[33,  4, 23], "
        "[33,  4, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13, 5],[33, 4, 23]]], [2,1], 1)",
        "[[[42, 13,  5],[42, 13,  5],[33,  4, 23]]]");


    //axis 2
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2, 2)",
        "[[[42, 42, 13, 13],[33, 33,  4,  4]],[[ 5,  5, 44, 44],"
        "[ 6,  6, 23, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2, -1)",
        "[[[42, 42, 13, 13],[33, 33,  4,  4]],[[ 5,  5, 44, 44],"
        "[ 6,  6, 23, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13, 5],[33, 4, 23]]], [3], 2)",
        "[[[42, 42, 42, 13, 13, 13,  5,  5,  5],"
        "[33, 33, 33,  4,  4,  4, 23, 23, 23]]]");
    test_repeat_operation(
        "repeat([[[42, 13, 5],[33, 4, 23]]], [2, 0, 1], 2)",
        "[[[42, 42,  5], [33, 33, 23]]]");

    //flatten
    test_repeat_operation("repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], 2)",
        "[42, 42, 13, 13, 33, 33,  4,  4,  5,  5, 44, 44,  6,  6, 23, 23]");
    test_repeat_operation("repeat([[[42, 13],[33, 4]],[[5, 44],[6, 23]]], "
                          "[0, 2, 3, 0, 1, 0, 0, 2])",
                          "[13, 13, 33, 33, 33,  5, 23, 23]");

    return hpx::util::report_errors();
}
