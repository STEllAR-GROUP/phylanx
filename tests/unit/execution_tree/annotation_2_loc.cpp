// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/distributed/iostream.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

void test_equality(std::string const& name, std::string const& lhs_codestr,
    std::string const& rhs_codestr)
{
    HPX_TEST_EQ(
        compile_and_run(name, lhs_codestr), compile_and_run(name, rhs_codestr));
}

void test_non_equality(std::string const& name, std::string const& lhs_codestr,
    std::string const& rhs_codestr)
{
    HPX_TEST_NEQ(
        compile_and_run(name, lhs_codestr), compile_and_run(name, rhs_codestr));
}

void test_annotation_0()
{
    if (hpx::get_locality_id() == 0)
    {
        std::string annotation_0 = R"(
            annotate_d([[1, 2]], "test_annotation_0_1",
                list("tile", list("columns", 0, 2), list("rows", 0, 1)))
        )";
        std::string annotation_1 = R"(
            annotate_d([[1, 2]], "test_annotation_0_1",
                list("tile", list("rows", 0, 1), list("columns", 0, 2)))
        )";
        test_equality("test_annotation_0", annotation_0, annotation_1);
    }
    else
    {
        std::string annotation_0 = R"(
            annotate_d([[3, 4]], "test_annotation_0_1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )";
        std::string annotation_1 = R"(
            annotate_d([[3, 4]], "test_annotation_0_1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )";
        test_equality("test_annotation_0", annotation_0, annotation_1);
    }
}


void test_annotation_1()
{
    if (hpx::get_locality_id() == 0)
    {
        std::string annotation_0 = R"(
            annotate_d([[1, 2]], "test_annotation_1_1",
                list("tile", list("columns", 0, 2), list("rows", 0, 1)))
        )";
        std::string annotation_1 = R"(
            annotate_d([[3, 4]], "test_annotation_1_1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )";

        test_non_equality("test_annotation_1", annotation_0, annotation_1);
    }
    else
    {
        std::string annotation_0 = R"(
            annotate_d([[3, 4]], "test_annotation_1_1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )";
        std::string annotation_1 = R"(
            annotate_d([[1, 2]], "test_annotation_1_1",
                list("tile", list("rows", 0, 1), list("columns", 0, 2)))
        )";

        test_non_equality("test_annotation_1", annotation_0, annotation_1);
    }
}

void test_annotation_2()
{
    if (hpx::get_locality_id() == 0)
    {
        std::string annotation_0 = R"(
            annotate_d([1, 2], "test_annotation_2_1",
                list("tile", list("columns", 0, 2)))
        )";
        std::string annotation_1 = R"(
            annotate_d([[1, 2]], "test_annotation_2_1",
                list("tile", list("rows", 0, 1), list("columns", 0, 2)))
        )";
        test_non_equality("test_annotation_2", annotation_0, annotation_1);
    }
    else
    {
        std::string annotation_0 = R"(
            annotate_d([3, 4], "test_annotation_2_1",
                list("tile", list("columns", 2, 4)))
        )";
        std::string annotation_1 = R"(
            annotate_d([[3, 4]], "test_annotation_2_1",
                list("tile", list("columns", 2, 4), list("rows", 0, 1)))
        )";
        test_non_equality("test_annotation_2", annotation_0, annotation_1);
    }
}

void test_annotation_3()
{
    if (hpx::get_locality_id() == 0)
    {
        std::string annotation_0 = R"(
            annotate_d([[]], "test_annotation_3_1",
                list("tile", list("columns", 0, 0), list("rows", 0, 0)))
        )";

        compile_and_run("test_annotation_3", annotation_0);
    }
    else
    {
        std::string annotation_0 = R"(
            annotate_d([[0, 1, 2], [3, 4, 5]], "test_annotation_3_1",
                list("tile", list("columns", 0, 3), list("rows", 0, 2)))
        )";

        compile_and_run("test_annotation_3", annotation_0);
    }
}

void test_annotation_4()
{
    // having overlapped spans
    if (hpx::get_locality_id() == 0)
    {
        std::string annotation_0 = R"(
            annotate_d([1, 2], "test_annotation_4_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("rows", 0, 2))))
        )";

        compile_and_run("test_annotation_4", annotation_0);
    }
    else
    {
        std::string annotation_0 = R"(
            annotate_d([2, 3, 4], "test_annotation_4_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("rows", 1, 4))))
        )";

        compile_and_run("test_annotation_4", annotation_0);
    }
}

void test_annotation_5()
{
    // having overlapped spans
    if (hpx::get_locality_id() == 0)
    {
        std::string annotation_0 = R"(
            annotate_d([[1, 2], [-1, -2]], "test_annotation_5_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("rows", 0, 2), list("columns", 0, 2))))
        )";

        compile_and_run("test_annotation_4", annotation_0);
    }
    else
    {
        std::string annotation_0 = R"(
            annotate_d([[-1, -2], [11, 12]], "test_annotation_5_1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("rows", 1, 3), list("columns", 0, 2))))
        )";

        compile_and_run("test_annotation_5", annotation_0);
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_annotation_0();
    test_annotation_1();
    test_annotation_2();
    test_annotation_3();
    test_annotation_4();
    test_annotation_5();

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
