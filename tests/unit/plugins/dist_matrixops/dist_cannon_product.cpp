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
void test_cannon_product(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type cannon_result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(cannon_result, comparison);
}

////////////////////////////////////////////////////////////////////////////////

void test_cannon_product_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_cannon_product("test2d2d_0", R"(
            cannon_product(
                annotate_d([[1], [2], [3]], "test2d2d_0_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 1), list("rows", 0, 3)))),
                annotate_d([[1, 2, 3]], "test2d2d_0_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 3), list("rows", 0, 1))))
            )
        )",
            R"(
            annotate_d([[1, 2, 3], [4, 8, 12], [6, 12, 18]],
                "test2d2d_0_1/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_cannon_product("test2d2d_0", R"(
            cannon_product(
                annotate_d([[0], [2], [3]], "test2d2d_0_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 1, 2), list("rows", 0, 3)))),
                annotate_d([[4, 0, 6]], "test2d2d_0_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 3, 6), list("rows", 0, 1))))
            )
        )",
            R"(
            annotate_d([[4, 0, 6], [16, 10, 24], [24, 15, 36]],
                "test2d2d_0_1/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 3, 6), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_cannon_product("test2d2d_0", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "test2d2d_0_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 1), list("rows", 3, 6)))),
                annotate_d([[1, 2, 3]], "test2d2d_0_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 3), list("rows", 1, 2))))
            )
        )",
            R"(
            annotate_d([[8, 16, 24], [10, 20, 30], [6, 12, 18]],
                "test2d2d_0_1/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 3), list("rows", 3, 6))))
        )");
    }
    else
    {
        test_cannon_product("test2d2d_0", R"(
            cannon_product(
                annotate_d([[4], [5], [0]], "test2d2d_0_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 3, 6)))),
                annotate_d([[4, 5, 6]], "test2d2d_0_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 3, 6), list("rows", 1, 2))))
            )
        )",
            R"(
            annotate_d([[32, 20, 48], [40, 25, 60], [24, 0, 36]],
                "test2d2d_0_1/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 3, 6), list("rows", 3, 6))))
        )");
    }
}

void test_cannon_product_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_cannon_product("test2d2d_1", R"(
            cannon_product(
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_1_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 2), list("rows", 0, 3)))),
                annotate_d([[0, 0, 0],[1, 2, 3]], "test2d2d_1_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 3), list("rows", 0, 2))))
            )
        )",
            R"(
            annotate_d([[2, 4, 6], [4, 8, 12], [6, 12, 18]],
                "test2d2d_1_1/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 3), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_cannon_product("test2d2d_1", R"(
            cannon_product(
                annotate_d([[1, 0], [2, 0], [3, 0]], "test2d2d_1_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 2, 4), list("rows", 0, 3)))),
                annotate_d([[4, 5, 6], [4, 5, 6]], "test2d2d_1_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 3, 6), list("rows", 0, 2))))
            )
        )",
            R"(
            annotate_d([[12, 15, 18], [24, 30, 36], [36, 45, 54]],
                "test2d2d_1_1/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 3, 6), list("rows", 0, 3))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_cannon_product("test2d2d_1", R"(
            cannon_product(
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_1_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 2), list("rows", 3, 6)))),
                annotate_d([[1, 2, 3], [1, 2, 0]], "test2d2d_1_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 3), list("rows", 2, 4))))
            )
        )",
            R"(
            annotate_d([[5, 10, 12], [7, 14, 15], [9, 18, 18]],
                "test2d2d_1_1/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 3), list("rows", 3, 6))))
        )");
    }
    else
    {
        test_cannon_product("test2d2d_1", R"(
            cannon_product(
                annotate_d([[0, 1], [0, 2], [0, 3]], "test2d2d_1_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 2, 4), list("rows", 3, 6)))),
                annotate_d([[4, 5, 6], [0, 0, 6]], "test2d2d_1_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 3, 6), list("rows", 2, 4))))
            )
        )",
            R"(
            annotate_d([[32, 40, 54], [40, 50, 72], [48, 60, 90]],
                "test2d2d_1_1/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 3, 6), list("rows", 3, 6))))
        )");
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_cannon_product_0();
    test_cannon_product_1();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
