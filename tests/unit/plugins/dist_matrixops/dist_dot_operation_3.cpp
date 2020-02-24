//   Copyright (c) 2017-2019 Hartmut Kaiser
//   Copyright (c) 2019 Bita Hasheminezhad
//   Copyright (c) 2019 Maxwell Reeser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
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
void test_dot_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    hpx::cout << "Start: " << name << hpx::endl;
    HPX_TEST_EQ(
        compile_and_run(name, code), compile_and_run(name, expected_str));
    hpx::cout << "End: " << name << hpx::endl << hpx::flush;
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_2d2d_6()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_6", R"(
            dot_d(
                annotate_d([[1], [2], [3]], "test2d2d_6_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 1), list("rows", 0, 3)))),
                annotate_d([[1, 2, 3]], "test2d2d_6_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 3), list("rows", 0, 1))))
            )
        )",
            R"(
            annotate_d([[2, 4, 6, 8, 10, 12], [4, 8, 12, 16, 20, 24]],
                "test2d2d_6_1/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 6), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_dot_operation("test2d2d_6", R"(
            dot_d(
                annotate_d([[1], [2], [3]], "test2d2d_6_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 1, 2), list("rows", 0, 3)))),
                annotate_d([[4, 5, 6]], "test2d2d_6_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 3, 6), list("rows", 0, 1))))
            )
        )",
            R"(
            annotate_d([[6, 12, 18, 24, 30, 36], [8, 16, 24, 32, 40, 48]],
                "test2d2d_6_1/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 0, 6), list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_dot_operation("test2d2d_6", R"(
            dot_d(
                annotate_d([[4], [5], [6]], "test2d2d_6_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 1), list("rows", 3, 6)))),
                annotate_d([[1, 2, 3]], "test2d2d_6_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 3), list("rows", 1, 2))))
            )
        )",
            R"(
            annotate_d([[10, 20, 30, 40, 50, 60]],
                "test2d2d_6_1/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 6), list("rows", 4, 5))))
        )");
    }
    else
    {
        test_dot_operation("test2d2d_6", R"(
            dot_d(
                annotate_d([[4], [5], [6]], "test2d2d_6_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 3, 6)))),
                annotate_d([[4, 5, 6]], "test2d2d_6_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 3, 6), list("rows", 1, 2))))
            )
        )",
            R"(
            annotate_d([[12, 24, 36, 48, 60, 72]],
                "test2d2d_6_1/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 0, 6), list("rows", 5, 6))))
        )");
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_dot_2d2d_6();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(argc, argv, cfg);
}
