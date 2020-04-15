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
    HPX_TEST_EQ(
        compile_and_run(name, code), compile_and_run(name, expected_str));
}

void test_dot_operation_throws(std::string const& name, std::string const& code)
{
    hpx::cout << "Test: " << name << hpx::endl;
    HPX_TEST_THROW(compile_and_run(name, code),
        hpx::detail::exception_with_info<hpx::exception>);
}

////////////////////////////////////////////////////////////////////////////////
void test_dot_2d2d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_1", R"(
            dot_d(
                annotate_d(
                    [[1,2,3,4],
                     [9,10,11,12]], "test2d2d_1_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 4), list("rows", 0, 2)))),
                annotate_d(
                    [[33],[35],[37],[39]], "test2d2d_1_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 1), list("rows", 0, 4))))
            )
        )",
            R"(
            annotate_d([[1524],[4084]], "test2d2d_1_1/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 1), list("rows", 0, 2))))
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_dot_operation("test2d2d_1", R"(
            dot_d(
                annotate_d(
                    [[17,18,19,20],[25,26,27,28]], "test2d2d_1_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 0, 4), list("rows", 2, 4)))),
                annotate_d(
                    [[34],[36],[38],[40]], "test2d2d_1_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 1, 2), list("rows", 0, 4))))
            )
        )",
            R"(
            annotate_d([[1560],[4184]], "test2d2d_1_1/1",
                list("args",
                    list("locality", 1, 4),
                    list("tile", list("columns", 1, 2), list("rows", 0, 2))))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_dot_operation("test2d2d_1", R"(
            dot_d(
                annotate_d(
                    [[5,6,7,8],[13,14,15,16]], "test2d2d_1_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 4, 8), list("rows", 0, 2)))),
                annotate_d(
                    [[41],[43],[45],[47]], "test2d2d_1_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 1), list("rows", 4, 8))))
            )
        )",
            R"(
            annotate_d([[6644],[9204], "test2d2d_1_1/1",
                list("args",
                    list("locality", 2, 4),
                    list("tile", list("columns", 0, 1), list("rows", 2, 4))))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_dot_operation("test2d2d_1", R"(
            dot_d(
                annotate_d(
                    [[21,22,23,24],[29,30,31,32]], "test2d2d_1_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 4, 8), list("rows", 2, 4)))),
                annotate_d(
                    [[40],[42],[44],[46]], "test2d2d_1_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 4, 8))))
            )
        )",
            R"(
            annotate_d([[6808],[9432], "test2d2d_1_1/1",
                list("args",
                    list("locality", 3, 4),
                    list("tile", list("columns", 1, 2), list("rows", 2, 4))))
        )");
    }
}

void test_dot_2d2d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_dot_operation("test2d2d_2", R"(
            dot_d(
                annotate_d(
                    [[1,2,3,4],
                     [9,10,11,12],
                     [17,18,19,20]], "test2d2d_2_1",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 4), list("rows", 0, 3)))),
                annotate_d(
                    [[33],[35],[37],[39]], "test2d2d_2_2",
                    list("args",
                        list("locality", 0, 4),
                        list("tile", list("columns", 0, 1), list("rows", 0, 4))))
            )
        )",
            R"(
            annotate_d([[1524],[4084]], "test2d2d_2_1/1",
                list("args",
                    list("locality", 0, 4),
                    list("tile", list("columns", 0, 1), list("rows", 0, 2))))
        )");
        /*
        */
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_dot_operation_throws("test2d2d_2", R"(
            dot_d(
                annotate_d(
                    [[25,26,27,28]], "test2d2d_2_1",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 3, 4), list("rows", 2, 4)))),
                annotate_d(
                    [[34],[36],[38],[40]], "test2d2d_2_2",
                    list("args",
                        list("locality", 1, 4),
                        list("tile", list("columns", 1, 2), list("rows", 0, 4))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_dot_operation_throws("test2d2d_2", R"(
            dot_d(
                annotate_d(
                    [[5,6,7,8],[13,14,15,16]], "test2d2d_2_1",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 4, 8), list("rows", 0, 2)))),
                annotate_d(
                    [[41],[43],[45],[47]], "test2d2d_2_2",
                    list("args",
                        list("locality", 2, 4),
                        list("tile", list("columns", 0, 1), list("rows", 4, 8))))
            )
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_dot_operation_throws("test2d2d_2", R"(
            dot_d(
                annotate_d(
                    [[21,22,23,24],[29,30,31,32]], "test2d2d_2_1",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 4, 8), list("rows", 2, 4)))),
                annotate_d(
                    [[40],[42],[44],[46]], "test2d2d_2_2",
                    list("args",
                        list("locality", 3, 4),
                        list("tile", list("columns", 1, 2), list("rows", 4, 8))))
            )
        )");
    }
}


////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_dot_2d2d_1();
    test_dot_2d2d_2();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };
    hpx::cout << "Sanity check" << hpx::endl;
    return hpx::init(argc, argv, cfg);
}
