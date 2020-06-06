// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);
    return code.run().arg_;
}

void test_expand_dims_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_expand_dims_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_3loc1d_0", R"(
            expand_dims(
                annotate_d([1, 2, 3, 4], "array_0",
                list("tile", list("columns", 2, 6)))
            , 0)
        )", R"(
            annotate_d([[1, 2, 3, 4]], "array_0_expanded/1",
                    list("tile", list("columns", 2, 6), list("rows", 0, 1)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_expand_dims_operation("test_expand_dims_3loc1d_0", R"(
            expand_dims(
                annotate_d([-1, 0], "array_0",
                list("tile", list("columns", 6, 8)))
            , 0)
        )", R"(
            annotate_d([[-1, 0]], "array_0_expanded/1",
                    list("tile", list("columns", 6, 8), list("rows", 0, 1)))
        )");
    }
    else
    {
        test_expand_dims_operation("test_expand_dims_3loc1d_0", R"(
            expand_dims(
                annotate_d([5, 42], "array_0",
                list("tile", list("columns", 0, 2)))
            , 0)
        )", R"(
            annotate_d([[5, 42]], "array_0_expanded/1",
                    list("tile", list("columns", 0, 2), list("rows", 0, 1)))
        )");
    }
}

void test_expand_dims_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_expand_dims_operation("test_expand_dims_3loc1d_1", R"(
            expand_dims(
                annotate_d([1, 2, 3, 4], "array_1",
                list("tile", list("columns", 2, 6)))
            , 1)
        )", R"(
            annotate_d([[1], [2], [3], [4]], "array_1_expanded/1",
                list("tile", list("columns", 0, 1), list("rows", 2, 6)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_expand_dims_operation("test_expand_dims_3loc1d_1", R"(
            expand_dims(
                annotate_d([-1, 0], "array_1",
                list("tile", list("columns", 6, 8)))
            , 1)
        )", R"(
            annotate_d([[-1], [0]], "array_1_expanded/1",
                list("tile", list("columns", 0, 1), list("rows", 6, 8)))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_expand_dims_operation("test_expand_dims_3loc1d_1", R"(
            expand_dims(
                annotate_d([5, 42], "array_1",
                list("tile", list("columns", 0, 2)))
            , 1)
        )", R"(
            annotate_d([[5], [42]], "array_1_expanded/1",
                list("tile", list("columns", 0, 1), list("rows", 0, 2)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_expand_dims_1d_0();
    test_expand_dims_1d_1();

    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(argc, argv, cfg);
}

