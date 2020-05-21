//   Copyright (c) 2020 Rory Hector
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

#include <algorithm>
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

void test_ginv_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(
        compile_and_run(name, code), compile_and_run(name, expected_str));
}


// send off the tiled (by columns) matrix
// and test against the known solution
//     l1  l2  l3           l1  l2  l3
//    | 3  -3  4 |        |  1  -1   0 |
//    | 2  -3  4 |   ->   | -2   3  -4 |
//    | 0  -1  1 |        | -2   3  -3 |
void test_gauss_inverse_3loc_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_ginv_operation("test_3_1", R"(
			inverse_d(
				annotate_d( [[3.0], [2.0], [0.0]],
					"test_3_1a",
					list("tile", list("columns", 0, 1), list("rows", 0,3)))
			)
		)",
            R"(
            annotate_d([[1.0], [-2.0], [-2.0]], "test_3_1a/1",
                list("tile", list("columns", 0, 1), list("rows", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_ginv_operation("test_3_1", R"(
			inverse_d(
				annotate_d( [[-3.0], [-3.0], [-1.0]],
					"test_3_1a",
					list("tile", list("columns", 1, 2), list("rows", 0,3)))
			)
		)",
            R"(
            annotate_d([[-1.0], [3.0], [3.0]], "test_3_1a/1",
                list("tile", list("columns", 1, 2), list("rows", 0, 3)))
        )");
    }
	else
    {
        test_ginv_operation("test_3_1", R"(
			inverse_d(
				annotate_d( [[4.0], [4.0], [1.0]],
					"test_3_1a",
					list("tile", list("columns", 2, 3), list("rows", 0,3)))
			)
		)",
            R"(
            annotate_d([[0.0], [-4.0], [-3.0]], "test_3_1a/1",
                list("tile", list("columns", 2, 3), list("rows", 0, 3)))
        )");
    }
}

// send off the tiled (by columns) matrix
// and test against the known solution
//     l1  l2  l3             l1   l2   l3
//    | 3   0   2 |        |  0.2   0   0.2 |
//    | 0   1   1 |   ->   | -0.2   1   0.3 |
//    | 2   0  -2 |        |  0.2   0  -0.3 |
void test_gauss_inverse_3loc_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_ginv_operation("test_3_2", R"(
			inverse_d(
				annotate_d( [[3.0], [0.0], [2.0]],
					"test_3_2a",
					list("tile", list("columns", 0, 1), list("rows", 0,3)))
			)
		)",
            R"(
            annotate_d([[0.2], [-0.2], [0.2]], "test_3_2a/1",
                list("tile", list("columns", 0, 1), list("rows", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_ginv_operation("test_3_2", R"(
			inverse_d(
				annotate_d( [[0.0], [1.0], [0.0]],
					"test_3_2a",
					list("tile", list("columns", 1, 2), list("rows", 0,3)))
			)
		)",
            R"(
            annotate_d([[0.0], [1.0], [0.0]], "test_3_2a/1",
                list("tile", list("columns", 1, 2), list("rows", 0, 3)))
        )");
    }
	else
    {
        test_ginv_operation("test_3_2", R"(
			inverse_d(
				annotate_d( [[2.0], [1.0], [-2.0]],
					"test_3_2a",
					list("tile", list("columns", 2, 3), list("rows", 0,3)))
			)
		)",
            R"(
            annotate_d([[0.2], [0.3], [-0.3]], "test_3_2a/1",
                list("tile", list("columns", 2, 3), list("rows", 0, 3)))
        )");
    }
}


//      l1  l1  l2  l3             l1   l1    l2    l3
//   | 1.0 1.0 1.0 0.0 |        | -3.0 -0.5   1.5   1.0 |
//   | 0.0 3.0 1.0 2.0 |   ->   |  1.0  0.25 -0.25 -0.5 |
//   | 2.0 3.0 1.0 0.0 |        |  3.0  0.25 -1.25 -0.5 |
//   | 1.0 0.0 2.0 1.0 |        | -3.0, 0.0   1.0   1.0 |
void test_gauss_inverse_3loc_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_ginv_operation("test_3_3", R"(
			inverse_d(
				annotate_d( [[1.0, 1.0], [0.0, 3.0], [2.0, 3.0], [1.0, 0.0]],
					"test_3_3a",
					list("tile", list("columns", 0, 2), list("rows", 0,4)))
			)
		)",
            R"(
            annotate_d([[-3.0, -0.5], [1.0, 0.25], [3.0, 0.25], [-3.0, 0.0]], "test_3_3a/1",
                list("tile", list("columns", 0, 2), list("rows", 0, 4)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_ginv_operation("test_3_3", R"(
			inverse_d(
				annotate_d( [[1.0], [1.0], [1.0], [2.0]],
					"test_3_3a",
					list("tile", list("columns", 2, 3), list("rows", 0,4)))
			)
		)",
            R"(
            annotate_d([[1.5], [-0.25], [-1.25], [1.0]], "test_3_3a/1",
                list("tile", list("columns", 2, 3), list("rows", 0, 4)))
        )");
    }
    else
    {
        test_ginv_operation("test_3_3", R"(
			inverse_d(
				annotate_d( [[0.0], [2.0], [0.0], [1.0]],
					"test_3_3a",
					list("tile", list("columns", 3, 4), list("rows", 0,4)))
			)
		)",
            R"(
            annotate_d([[1.0], [-0.5], [-0.5], [1.0]], "test_3_3a/1",
                list("tile", list("columns", 3, 4), list("rows", 0, 4)))
        )");
    }
}





int hpx_main(int argc, char* argv[])
{
     test_gauss_inverse_3loc_1();
	 test_gauss_inverse_3loc_2();
	 test_gauss_inverse_3loc_3();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};
    return hpx::init(argc, argv, cfg);
}
