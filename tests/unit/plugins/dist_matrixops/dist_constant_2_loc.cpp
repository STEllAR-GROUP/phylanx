// Copyright (c) 2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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

void test_constant_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);
    //std::cout << "result" << result << ": " << std::endl;
    //std::cout << "expected result" << comparison << ": " << std::endl;
    HPX_TEST_EQ(result, comparison);
}

//void test_empty_operation(std::string const& code,
//    std::array<int, 2> const& dims)
//{
//    auto f = compile_and_run(code);
//    auto result_dims =
//        phylanx::execution_tree::extract_numeric_value_dimensions(f());
//
//    HPX_TEST_EQ(dims[0], result_dims[0]);
//    HPX_TEST_EQ(dims[1], result_dims[1]);
//}
//
//void test_empty_operation(std::string const& code,
//    std::array<int, 3> const& dims)
//{
//    auto f = compile_and_run(code);
//    auto result_dims =
//        phylanx::execution_tree::extract_numeric_value_dimensions(f());
//
//    HPX_TEST_EQ(dims[0], result_dims[0]);
//    HPX_TEST_EQ(dims[1], result_dims[1]);
//    HPX_TEST_EQ(dims[2], result_dims[2]);
//}
//
//void test_empty_operation(std::string const& code,
//    std::array<int, 4> const& dims)
//{
//    auto f = compile_and_run(code);
//    auto result_dims =
//        phylanx::execution_tree::extract_numeric_value_dimensions(f());
//
//    HPX_TEST_EQ(dims[0], result_dims[0]);
//    HPX_TEST_EQ(dims[1], result_dims[1]);
//    HPX_TEST_EQ(dims[2], result_dims[2]);
//    HPX_TEST_EQ(dims[3], result_dims[3]);
//}

///////////////////////////////////////////////////////////////////////////////
void test_constant_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test1d_0", R"(
            constant_d(42, list(4), 0, 2)
        )", R"(
            annotate_d([42.0, 42.0], "constant_2_4",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2))))
        )");
    }
    else
    {
        test_constant_d_operation("test1d_0", R"(
            constant_d(42, list(4), 1, 2)
        )", R"(
            annotate_d([42.0, 42.0], "constant_2_4",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 2, 4))))
        )");
    }
}

void test_constant_1d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_constant_d_operation("test1d_1", R"(
            constant_d(42, list(5), 0, 2)
        )", R"(
            annotate_d([42.0, 42.0, 42.0], "constant_2_5",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 3))))
        )");
    }
    else
    {
        test_constant_d_operation("test1d_1", R"(
            constant_d(42, list(5), 1, 2)
        )", R"(
            annotate_d([42.0, 42.0], "constant_2_5",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 3, 5))))
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_constant_1d_0();
    test_constant_1d_1();
    //test_constant_1d_2();

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

