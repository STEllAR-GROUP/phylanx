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
    hpx::cout << cannon_result << " : " << comparison << hpx::endl;
    HPX_TEST_EQ(cannon_result, comparison);
}

////////////////////////////////////////////////////////////////////////////////

void test_cannon_product_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_cannon_product("test2d2d_0", R"(
            cannon_product(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_0_1",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1))),
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_0_2",
                    list("tile", list("columns", 0, 2), list("rows", 3, 6)))
            )
        )",
            R"(
            annotate_d([[91, 91]], "test2d2d_0_1/1",
                list("tile", list("rows", 0, 1), list("columns", 0, 2)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_cannon_product("test2d2d_0", R"(
            cannon_product(
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_0_1",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2))),
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_0_2",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3)))
            )
        )",
            R"(
            annotate_d([[91, 91]], "test2d2d_0_1/1",
                list("tile", list("columns", 0, 2), list("rows", 1, 2)))
        )");
    }
}

void test_cannon_product_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_cannon_product("test2d2d_1", R"(
            cannon_product(
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_1_1",
                    list("tile", list("columns", 0, 2), list("rows", 3, 6))),
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_1_2",
                    list("tile", list("columns", 0, 6), list("rows", 0, 1)))
                
            )
        )",
            R"(
            annotate_d([[8, 16, 24, 32, 40, 48], [10, 20, 30, 40, 50, 60], [12, 24, 36, 48, 60, 72]],
                "test2d2d_1_1/1",
                list("tile", list("rows", 3, 6), list("columns", 0, 6)))
        )");
    }
    else if (hpx::get_locality_id() == 0)
    {
        test_cannon_product("test2d2d_1", R"(
            cannon_product(
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_1_1",
                    list("tile", list("columns", 0, 2), list("rows", 0, 3))),
                annotate_d([[1, 2, 3, 4, 5, 6]], "test2d2d_1_2",
                    list("tile", list("columns", 0, 6), list("rows", 1, 2)))
            )
        )",
            R"(
            annotate_d([[2, 4, 6, 8, 10, 12], [4, 8, 12, 16, 20, 24], [6, 12, 18, 24, 30, 36]],
                "test2d2d_1_1/1",
                list("tile", list("columns", 0, 6), list("rows", 0, 3)))
        )");
    }
}

void test_cannon_product_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1], [2], [3]], "test2d2d_2_1",
                    list("tile", list("columns", 0, 1), list("rows", 0, 3))),
                annotate_d([[1, 2, 3]], "test2d2d_2_2",
                    list("tile", list("columns", 0, 3), list("rows", 0, 1)))
                
            )
        )",
            R"(
            annotate_d([[2, 4, 6], [4, 8, 12], [6, 12, 18][8, 16, 24, 32, 40, 48], [10, 20, 30, 40, 50, 60], [12, 24, 36, 48, 60, 72]],
                "test2d2d_2_1/1",
                list("tile", list("rows", 0, 3), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1], [2], [3]], "test2d2d_2_1",
                    list("tile", list("columns", 1, 2), list("rows", 0, 3))),
                annotate_d([[4, 5, 6]], "test2d2d_2_2",
                    list("tile", list("columns", 3, 6), list("rows", 0, 1)))
            )
        )",
            R"(
            annotate_d([[8, 10, 12], [16, 20, 24], [24, 30, 36]],
                "test2d2d_2_1/1",
                list("tile", list("columns", 3, 6), list("rows", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "test2d2d_2_1",
                    list("tile", list("columns", 0, 1), list("rows", 3, 6))),
                annotate_d([[1, 2, 3]], "test2d2d_2_2",
                    list("tile", list("columns", 0, 3), list("rows", 1, 2)))
            )
        )",
            R"(
            annotate_d([[8, 16, 24], [10, 20, 30], [12, 24, 36]],
                "test2d2d_2_1/1",
                list("tile", list("columns", 0, 3), list("rows", 3, 6)))
        )");
    }
    else
    {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "test2d2d_2_1",
                    list("tile", list("columns", 1, 2), list("rows", 3, 6))),
                annotate_d([[4, 5, 6]], "test2d2d_2_2",
                    list("tile", list("columns", 3, 6), list("rows", 1, 2)))
            )
        )",
            R"(
            annotate_d([[32, 40, 48], [40, 50, 60], [48, 60, 72]],
                "test2d2d_2_1/1",
                list("tile", list("columns", 3, 6), list("rows", 3, 6)))
        )");
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_cannon_product_0();
    test_cannon_product_1();
    test_cannon_product_2();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
