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
void test_cannon_product_2()
{
    switch (hpx::get_locality_id())
    {
    case 0: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_2_1",
                    list("args",
                        list("locality", 0, 9),
                        list("tile", list("columns", 0, 2), list("rows", 0, 3)))),
                annotate_d([[0], [1]], "test2d2d_2_2",
                    list("args",
                        list("locality", 0, 9),
                        list("tile", list("columns", 0, 1), list("rows", 0, 2))))
            )
        )",
            R"(
            annotate_d([[10], [20], [30]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 0, 9),
                    list("tile", list("columns", 0, 1), list("rows", 0, 3))))
        )");
        break;
    }
    case 1: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1, 0], [2, 0], [3, 0]], "test2d2d_2_1",
                    list("args",
                        list("locality", 1, 9),
                        list("tile", list("columns", 2, 4), list("rows", 0, 3)))),
                annotate_d([[0], [2]], "test2d2d_2_2",
                    list("args",
                        list("locality", 1, 9),
                        list("tile", list("columns", 1, 2), list("rows", 0, 2))))
            )
        )",
            R"(
            annotate_d([[14], [28], [42]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 1, 9),
                    list("tile", list("columns", 1, 2), list("rows", 0, 3))))
        )");
        break;
    }
    case 2: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[1, 1], [2, 2], [3, 3]], "test2d2d_2_1",
                    list("args",
                        list("locality", 2, 9),
                        list("tile", list("columns", 4, 6), list("rows", 0, 3)))),
                annotate_d([[0], [3]], "test2d2d_2_2",
                    list("args",
                        list("locality", 2, 9),
                        list("tile", list("columns", 2, 3), list("rows", 0, 2))))
            )
        )",
            R"(
            annotate_d([[18], [36], [54]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 2, 9),
                    list("tile", list("columns", 2, 3), list("rows", 0, 3))))
        )");
        break;
    }
    case 3: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_2_1",
                    list("args",
                        list("locality", 3, 9),
                        list("tile", list("columns", 0, 2), list("rows", 3, 6)))),
                annotate_d([[1], [0]], "test2d2d_2_2",
                    list("args",
                        list("locality", 3, 9),
                        list("tile", list("columns", 0, 1), list("rows", 2, 4))))
            )
        )",
            R"(
            annotate_d([[36], [45], [54]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 3, 9),
                    list("tile", list("columns", 0, 1), list("rows", 3, 6))))
        )");
        break;
    }
    case 4: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[0, 1], [0, 2], [0, 3]], "test2d2d_2_1",
                    list("args",
                        list("locality", 4, 9),
                        list("tile", list("columns", 2, 4), list("rows", 3, 6)))),
                annotate_d([[2], [0]], "test2d2d_2_2",
                    list("args",
                        list("locality", 4, 9),
                        list("tile", list("columns", 1, 2), list("rows", 2, 4))))
            )
        )",
            R"(
            annotate_d([[48], [60], [72]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 4, 9),
                    list("tile", list("columns", 1, 2), list("rows", 3, 6))))
        )");
        break;
    }
    case 5: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[4, 4], [5, 5], [6, 6]], "test2d2d_2_1",
                    list("args",
                        list("locality", 5, 9),
                        list("tile", list("columns", 4, 6), list("rows", 3, 6)))),
                annotate_d([[3], [6]], "test2d2d_2_2",
                    list("args",
                        list("locality", 5, 9),
                        list("tile", list("columns", 2, 3), list("rows", 2, 4))))
            )
        )",
            R"(
            annotate_d([[66], [87], [108]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 5, 9),
                    list("tile", list("columns", 2, 3), list("rows", 3, 6))))
        )");
        break;
    }
    case 6: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[7, 0], [8, 8], [9, 0]], "test2d2d_2_1",
                    list("args",
                        list("locality", 6, 9),
                        list("tile", list("columns", 0, 2), list("rows", 6, 9)))),
                annotate_d([[4], [4]], "test2d2d_2_2",
                    list("args",
                        list("locality", 6, 9),
                        list("tile", list("columns", 0, 1), list("rows", 4, 6))))
            )
        )",
            R"(
            annotate_d([[35], [72], [36]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 6, 9),
                    list("tile", list("columns", 0, 1), list("rows", 6, 9))))
        )");
        break;
    }
    case 7: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[7, 0], [0, 0], [0, 0]], "test2d2d_2_1",
                    list("args",
                        list("locality", 7, 9),
                        list("tile", list("columns", 2, 4), list("rows", 6, 9)))),
                annotate_d([[5], [5]], "test2d2d_2_2",
                    list("args",
                        list("locality", 7, 9),
                        list("tile", list("columns", 1, 2), list("rows", 4, 6))))
            )
        )",
            R"(
            annotate_d([[49], [96], [45]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 7, 9),
                    list("tile", list("columns", 1, 2), list("rows", 6, 9))))
        )");
        break;
    }
    case 8: {
        test_cannon_product("test2d2d_2", R"(
            cannon_product(
                annotate_d([[7, 0], [8, 8], [0, 9]], "test2d2d_2_1",
                    list("args",
                        list("locality", 8, 9),
                        list("tile", list("columns", 4, 6), list("rows", 6, 9)))),
                annotate_d([[6], [6]], "test2d2d_2_2",
                    list("args",
                        list("locality", 8, 9),
                        list("tile", list("columns", 2, 3), list("rows", 4, 6))))
            )
        )",
            R"(
            annotate_d([[63], [120], [54]],
                "test2d2d_2_1/1",
                list("args",
                    list("locality", 8, 9),
                    list("tile", list("columns", 2, 3), list("rows", 6, 9))))
        )");
        break;
    }
    }
}

////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_cannon_product_2();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
