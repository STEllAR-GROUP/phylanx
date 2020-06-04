// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
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

void test_generic_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
void test_sqrt_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_generic_d_operation("test_sqrt_2loc1d_0", R"(
            sqrt(annotate_d([0, 1764, 1, 169], "array_1",
                list("tile", list("columns", 2, 6))))
        )", R"(
            annotate_d([0., 42., 1., 13.], "array_1/1",
                list("tile", list("columns", 2, 6)))
        )");
    }
    else
    {
        test_generic_d_operation("test_sqrt_2loc1d_0", R"(
            sqrt(annotate_d([16, 4], "array_1",
                list("tile", list("columns", 0, 2))))
        )", R"(
            annotate_d([4., 2.], "array_1/1",
                list("tile", list("columns", 0, 2)))
        )");
    }
}

void test_square_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_generic_d_operation("test_square_2loc1d_0", R"(
            square(annotate_d([4., 2.], "array_2",
                list("tile", list("rows", 0, 2))))
        )", R"(
            annotate_d([16., 4.], "array_2/1",
                list("tile", list("rows", 0, 2)))
        )");
    }
    else
    {
        test_generic_d_operation("test_square_2loc1d_0", R"(
            square(annotate_d([0., 42., 1., 13.], "array_2",
                list("tile", list("rows", 2, 6))))
        )", R"(
            annotate_d([0., 1764., 1., 169.], "array_2/1",
                list("tile", list("rows", 2, 6)))
        )");
    }
}

/////////////////////////////////////////////////////////////////////////////////
void test_sqrt_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_generic_d_operation("test_sqrt_2loc2d_0", R"(
            sqrt(annotate_d([[0, 1764], [1, 169]], "array_3",
                list("tile", list("columns", 0, 2), list("rows", 0, 2))))
        )", R"(
            annotate_d([[0., 42.], [1., 13.]], "array_3/1",
                list("tile", list("columns", 0, 2), list("rows", 0, 2)))
        )");
    }
    else
    {
        test_generic_d_operation("test_sqrt_2loc2d_0", R"(
            sqrt(annotate_d([[16], [4]], "array_3",
                list("tile", list("columns", 2, 3), list("rows", 0, 2))))
        )", R"(
            annotate_d([[4.], [2.]], "array_3/1",
                list("tile", list("columns", 2, 3), list("rows", 0, 2)))
        )");
    }
}

void test_square_2d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_generic_d_operation("test_square_2loc2d_0", R"(
            square(annotate_d([[1, 2], [4, 5]], "array_4",
                list("tile", list("columns", 1, 3), list("rows", 0, 2))))
        )", R"(
            annotate_d([[1, 4], [16, 25]], "array_4/1",
                list("tile", list("columns", 1, 3), list("rows", 0, 2)))
        )");
    }
    else
    {
        test_generic_d_operation("test_square_2loc2d_0", R"(
            square(annotate_d([[0], [3]], "array_4",
                list("tile", list("columns", 0, 1), list("rows", 0, 2))))
        )", R"(
            annotate_d([[0], [9]], "array_4/1",
                list("tile", list("columns", 0, 1), list("rows", 0, 2)))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_sqrt_1d_0();
    test_square_1d_0();

    test_sqrt_2d_0();
    test_square_2d_0();

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

