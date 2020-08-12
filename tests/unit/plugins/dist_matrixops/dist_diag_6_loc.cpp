//   Copyright (c) 2020 Hartmut Kaiser
//   Copyright (c) 2020 Nanmiao Wu
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/iostream.hpp>
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

///////////////////////////////////////////////////////////////////////////////
void test_diag_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

////////////////////////////////////////////////////////////////////////////////
void test_diag_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([1], "my_diag1d_0",
                    list("tile", list("columns", 0, 1))
                ),
                2, "sym", 0, 6
            )
        )", R"(
            annotate_d([[0, 0, 1], [0, 0, 0], [0, 0, 0], [0, 0, 0],
                [0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 0, 6),
                    list("tile", list("columns", 0, 3),
                    list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([2, 3], "my_diag1d_0",
                    list("tile", list("columns", 1, 3))
                ),
                2, "sym", 1, 6
            )
        )", R"(
            annotate_d([[0, 0, 0], [2, 0, 0], [0, 3, 0], [0, 0, 4],
                [0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 1, 6),
                    list("tile", list("columns", 3, 6),
                    list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([4], "my_diag1d_0",
                    list("tile", list("columns", 3, 4))
                ),
                2, "sym", 2, 6
            )
        )", R"(
            annotate_d([[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0],
                [5, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 2, 6),
                    list("tile", list("columns", 6, 9),
                    list("rows", 0, 5))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([5], "my_diag1d_0",
                    list("tile", list("columns", 4, 5))
                ),
                2, "sym", 3, 6
            )
        )", R"(
            annotate_d([[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 3, 6),
                    list("tile", list("columns", 0, 3),
                    list("rows", 5, 9))))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([6], "my_diag1d_0",
                    list("tile", list("columns", 5, 6))
                ),
                2, "sym", 4, 6
            )
        )", R"(
            annotate_d([[0, 0, 0], [0, 0, 0], [0, 0, 0], [0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 4, 6),
                    list("tile", list("columns", 3, 6),
                    list("rows", 5, 9))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([7], "my_diag1d_0",
                    list("tile", list("columns", 6, 7))
                ),
                2, "sym", 5, 6
            )
        )", R"(
            annotate_d([[0, 6, 0], [0, 0, 7], [0, 0, 0], [0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 5, 6),
                    list("tile", list("columns", 6, 9),
                    list("rows", 5, 9))))
        )");

    }
}


////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_diag_1d_0();

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
