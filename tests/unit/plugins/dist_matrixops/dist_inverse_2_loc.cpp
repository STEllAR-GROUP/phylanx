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
//    | 4  7 |   ->   |  0.6  -0.7 |
//    | 2  6 |        | -0.2   0.4 |
// using the format:
// annotate_d( <matrix cotents belonging to this locality>,
//			   <some keyword>,
//			   list("tile", list(<range of cols for this loc +end>),
//							lsit(<range of rows for this loc +end>))
// and then do other annotate_d similarly for info in other locality
void test_gauss_inverse_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_ginv_operation("test_0", R"(
			inverse_d(
				annotate_d( [[4.0], [2.0]],
					"test_0_1",
					list("tile", list("columns", 0, 1), list("rows", 0,2)))

			)
		)",
            "[[0.6, -0.7], [-0.2, 0.4]]");
    }
    else
    {
        test_ginv_operation("test_0", R"(
			inverse_d(
				annotate_d( [[7.0], [6.0]],
					"test_0_1",
					list("tile", list("columns", 1, 2), list("rows", 0,2)))

			)
		)",
            "[[0.6, -0.7], [-0.2, 0.4]]");
    }
}


// send off the tiled (by columns) matrix
// except send two cols to one locality, one col to other
//    | 3  -3  4 |        |  1  -1   0 |
//    | 2  -3  4 |   ->   | -2   3  -4 |
//    | 0  -1  1 |        | -2   3  -3 |
void test_gauss_inverse_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_ginv_operation("test_2", R"(
			inverse_d(
				annotate_d( [[3.0, -3.0], [2.0, -3.0], [0.0, -1.0]],
					"test_2_1",
					list("tile", list("columns", 0, 2), list("rows", 0,3)))

			)
		)",
            "[[1.0, -1.0, 0.0], [-2.0, 3.0, -4.0], [-2.0, 3.0, -3.0]]");
    }
    else
    {
        test_ginv_operation("test_2", R"(
			inverse_d(
				annotate_d( [[4.0], [4.0], [1.0]],
					"test_2_1",
					list("tile", list("columns", 2, 3), list("rows", 0,3)))

			)
		)",
            "[[1.0, -1.0, 0.0], [-2.0, 3.0, -4.0], [-2.0, 3.0, -3.0]]");
    }
}


//   | 1.0 1.0 1.0 0.0 |        | -3.0 -0.5   1.5   1.0 |
//   | 0.0 3.0 1.0 2.0 |   ->   |  1.0  0.25 -0.25 -0.5 |
//   | 2.0 3.0 1.0 0.0 |        |  3.0  0.25 -1.25 -0.5 |
//   | 1.0 0.0 2.0 1.0 |        | -3.0, 0.0   1.0   1.0 |
void test_gauss_inverse_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_ginv_operation("test_4", R"(
			inverse_d(
				annotate_d( [[1.0, 1.0], [0.0, 3.0], [2.0, 3.0], [1.0, 0.0]],
					"test_4_1",
					list("tile", list("columns", 0, 2), list("rows", 0,4)))

			)
		)",
     "[[-3.0, -0.5, 1.5, 1.0], [1.0, 0.25, -0.25, -0.5], [3.0, 0.25, -1.25, -0.5], [-3.0, 0.0, 1.0, 1.0]]");
    }
    else
    {
        test_ginv_operation("test_2", R"(
			inverse_d(
				annotate_d( [[1.0, 0.0], [1.0, 2.0], [1.0, 0.0], [2.0, 1.0]],
					"test_4_1",
					list("tile", list("columns", 2, 4), list("rows", 0,4)))

			)
		)",
     "[[-3.0, -0.5, 1.5, 1.0], [1.0, 0.25, -0.25, -0.5], [3.0, 0.25, -1.25, -0.5], [-3.0, 0.0, 1.0, 1.0]]");
    }
}


// Convert matrix to string to support easier testing of
// randomly generated matrices
inline std::string matToString(blaze::DynamicMatrix<double> m, std::uint64_t n,
    std::uint64_t startCol, std::uint64_t endCol)
{
    std::string randMatString = "[";
    for (std::int64_t currentRow = 0; currentRow < n; currentRow++)
    {
        randMatString += "[";
        for (std::int64_t currentCol = startCol; currentCol <= endCol;
             currentCol++)
        {
            randMatString += std::to_string(m(currentRow, currentCol));
            if (currentCol < endCol)
                randMatString += ", ";
        }
        randMatString += "]";
        if (currentRow < n - 1)
            randMatString += ", ";
    }
    randMatString += "]";
    return randMatString;
}

// Wrap the input string with necessary annotation data
inline std::string wrapInputString(std::string inString, std::uint64_t n,
    std::uint64_t id, std::uint64_t startCol, std::uint64_t endCol)
{
    std::string wrappedInput = "inverse_d(\n"
                               "    annotate_d(" +
        inString +
        ",\n"
        "        \"test_3_1" +
        "\",\n"
        "        list(\"tile\", list(\"columns\", " +
        std::to_string(startCol) + "," + std::to_string(endCol + 1) +
        "), list(\"rows\", 0," + std::to_string(n) +
        ")))\n"
        ")";

    return wrappedInput;
}

// Test random nxn matrix test compared to blaze::inv converted to string
void test_gauss_inverse_3(std::uint64_t n)
{
    // Check to ensure matrix is at least 2x2
    if (n < 2)
    {
        std::cout << "Error: This test requires an nxn matrix where n>1.";
        std::cout << std::endl;
        return;
    }

    // Generate a random nxn matrix
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(n, n);
    std::string inverseString = matToString(blaze::inv(m), n, 0, n-1);
    std::uint64_t id = hpx::get_locality_id();

    if (id == 0)
    {
        std::string inputString = matToString(m, n, 0, n / 2 - 1);
        std::string wrappedInput =
            wrapInputString(inputString, n, id, 0, n / 2 - 1);
        test_ginv_operation("test_3", wrappedInput, inverseString);
    }
    else
    {
        std::string inputString = matToString(m, n, n / 2, n - 1);
        std::string wrappedInput =
            wrapInputString(inputString, n, id, n / 2, n - 1);
        test_ginv_operation("test_3", wrappedInput, inverseString);
    }
}

int hpx_main(int argc, char* argv[])
{
     test_gauss_inverse_0();
     // test_gauss_inverse_2();
     // test_gauss_inverse_3(3);
     // test_gauss_inverse_4();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
