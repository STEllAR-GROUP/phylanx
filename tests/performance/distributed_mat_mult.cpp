// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Maxwell Reeser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
void execution_time(std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& compiled_code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);

    hpx::cout << "Locality: " << hpx::get_locality_id()
              << "  Starting execution for: " << name << hpx::endl;
    std::uint64_t t = hpx::util::high_resolution_clock::now();
    compiled_code.run().arg_;
    t = hpx::util::high_resolution_clock::now() - t;

    hpx::cout << "       " << (t / 1e6) << " ms.\n" << hpx::flush;
}

////////////////////////////////////////////////////////////////////////////////

void test_cannon_product_0()
{
    if (hpx::get_locality_id() == 0)
    {
        execution_time("perftest_cannon_0", R"(
            cannon_product(
                annotate_d([[1], [2], [3]], "perftest_cannon_0_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 1), list("rows", 0, 3)))),
                annotate_d([[1, 2, 3]], "perftest_cannon_0_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 3), list("rows", 0, 1))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_cannon_0", R"(
            cannon_product(
                annotate_d([[0], [2], [3]], "perftest_cannon_0_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 1, 2), list("rows", 0, 3)))),
                annotate_d([[4, 0, 6]], "perftest_cannon_0_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 3, 6), list("rows", 0, 1))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        execution_time("perftest_cannon_0", R"(
            cannon_product(
                annotate_d([[4], [5], [6]], "perftest_cannon_0_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 1), list("rows", 3, 6)))),
                annotate_d([[1, 2, 3]], "perftest_cannon_0_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 3), list("rows", 1, 2))))
            )
        )");
    }
    else
    {
        execution_time("perftest_cannon_0", R"(
            cannon_product(
                annotate_d([[4], [5], [0]], "perftest_cannon_0_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 3, 6)))),
                annotate_d([[4, 5, 6]], "perftest_cannon_0_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 3, 6), list("rows", 1, 2))))
            )
        )");
    }
}

void test_cannon_product_1()
{
    if (hpx::get_locality_id() == 0)
    {
        execution_time("perftest_cannon_1", R"(
            cannon_product(
                annotate_d(random(list(100, 100)), "perftest_cannon_1_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 100),
                            list("rows", 0, 100)))),
                annotate_d(random(list(100, 100)), "perftest_cannon_1_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 100),
                            list("rows", 0, 100))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_cannon_1", R"(
            cannon_product(
                annotate_d(random(list(100, 100)), "perftest_cannon_1_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 100, 200),
                            list("rows", 0, 100)))),
                annotate_d(random(list(100, 100)), "perftest_cannon_1_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 100, 200),
                            list("rows", 0, 100))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        execution_time("perftest_cannon_1", R"(
            cannon_product(
                annotate_d(random(list(100, 100)), "perftest_cannon_1_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 100),
                            list("rows", 100, 200)))),
                annotate_d(random(list(100, 100)), "perftest_cannon_1_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 100),
                            list("rows", 100, 200))))
            )
        )");
    }
    else
    {
        execution_time("perftest_cannon_1", R"(
            cannon_product(
                annotate_d(random(list(100, 100)), "perftest_cannon_1_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 100, 200),
                            list("rows", 100, 200)))),
                annotate_d(random(list(100, 100)), "perftest_cannon_1_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 100, 200),
                            list("rows", 100, 200))))
            )
        )");
    }
}

void test_cannon_product_2()
{
    if (hpx::get_locality_id() == 0)
    {
        execution_time("perftest_cannon_2", R"(
            cannon_product(
                annotate_d(random(list(300, 300)), "perftest_cannon_2_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 300),
                            list("rows", 0, 300)))),
                annotate_d(random(list(300, 300)), "perftest_cannon_2_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 300),
                            list("rows", 0, 300))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        execution_time("perftest_cannon_2", R"(
            cannon_product(
                annotate_d(random(list(300, 300)), "perftest_cannon_2_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 300, 600),
                            list("rows", 0, 300)))),
                annotate_d(random(list(300, 300)), "perftest_cannon_2_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 300, 600),
                            list("rows", 0, 300))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        execution_time("perftest_cannon_2", R"(
            cannon_product(
                annotate_d(random(list(300, 300)), "perftest_cannon_2_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 300),
                            list("rows", 300, 600)))),
                annotate_d(random(list(300, 300)), "perftest_cannon_2_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 300),
                            list("rows", 300, 600))))
            )
        )");
    }
    else
    {
        execution_time("perftest_cannon_2", R"(
            cannon_product(
                annotate_d(random(list(300, 300)), "perftest_cannon_2_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 300, 600),
                            list("rows", 300, 600)))),
                annotate_d(random(list(300, 300)), "perftest_cannon_2_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 300, 600),
                            list("rows", 300, 600))))
            )
        )");
    }
}
////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_cannon_product_0();
    test_cannon_product_1();
    test_cannon_product_2();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
