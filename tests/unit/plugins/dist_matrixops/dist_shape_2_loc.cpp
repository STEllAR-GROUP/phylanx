// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

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

void test_shape_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_shape_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_shape_d_operation("test_shape_2loc1d_0", R"(
            shape(annotate_d([42.0, 13.0], "full_array_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2)))))
        )", "list(5)");
    }
    else
    {
        test_shape_d_operation("test_shape_2loc1d_0", R"(
            shape(annotate_d([33.0, 42.0, 43.0], "full_array_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 5)))))
        )", "list(5)");
    }
}

void test_shape_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_shape_d_operation("test_shape_2loc1d_1", R"(
            shape(annotate_d([1, 2, 3], "array",
                list("tile", list("rows", 2, 5))))
        )", "list(5)");
    }
    else
    {
        test_shape_d_operation("test_shape_2loc1d_1", R"(
            shape(annotate_d([1, 2], "array",
                list("tile", list("rows", 0, 2))))
        )", "list(5)");
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_shape_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_shape_d_operation("test_shape_2loc2d_0", R"(
            shape(annotate_d([[-1,-2,-3], [1, 2, 3]], "array",
                list("tile", list("rows", 1, 3), list("columns", 0, 3))))
        )", "list(3, 3)");
    }
    else
    {
        test_shape_d_operation("test_shape_2loc2d_0", R"(
            shape(annotate_d([[11, 12, 33]], "array",
                list("tile", list("rows", 0, 1), list("columns", 0, 3))))
        )", "list(3, 3)");
    }
}

void test_shape_2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_shape_d_operation("test_shape_2loc2d_1", R"(
            shape(annotate_d([[-1,-2], [-3, 1], [2, 3]], "array",
                list("tile", list("rows", 0, 3), list("columns", 3, 5))))
        )", "list(3, 5)");
    }
    else
    {
        test_shape_d_operation("test_shape_2loc2d_1", R"(
            shape(annotate_d([[1, 2, 3], [4, 5, 6], [7, 8, 9]], "array",
                list("tile", list("rows", 0, 3), list("columns", 0, 3))))
        )", "list(3, 5)");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_shape_1d_0();
    //test_shape_1d_1();

    //test_shape_2d_0();
    //test_shape_2d_1();

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

