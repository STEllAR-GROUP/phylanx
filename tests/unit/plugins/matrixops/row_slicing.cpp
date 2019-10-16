// Copyright (c) 2017-2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////

void test_row_slicing_operation_0d()
{
    std::string const code = R"(
        slice_row(42.0)
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 42.0);
}


void test_row_slicing_operation_1d()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(2,4))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 3);
    HPX_TEST_EQ(result[1], 4);
}

void test_row_slicing_operation_1d_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(2,6,2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 3);
    HPX_TEST_EQ(result[1], 5);
}

void test_row_slicing_operation_1d_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(6,2,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 7);
    HPX_TEST_EQ(result[1], 5);
}

void test_row_slicing_operation_1d_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(-6,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(4));
    HPX_TEST_EQ(result[0], 3);
    HPX_TEST_EQ(result[1], 4);
    HPX_TEST_EQ(result[2], 5);
    HPX_TEST_EQ(result[3], 6);
}

void test_row_slicing_operation_1d_single_slice_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(-6,-5))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 3);
}

void test_row_slicing_operation_1d_negative_index_zero_start()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(0,-1))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(7));
    HPX_TEST_EQ(result[0], 1);
    HPX_TEST_EQ(result[1], 2);
    HPX_TEST_EQ(result[2], 3);
    HPX_TEST_EQ(result[3], 4);
    HPX_TEST_EQ(result[4], 5);
    HPX_TEST_EQ(result[5], 6);
    HPX_TEST_EQ(result[6], 7);
}

void test_row_slicing_operation_1d_negative_index_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, '(-2,-6,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 7);
    HPX_TEST_EQ(result[1], 5);
}

void test_row_slicing_operation_1d_single()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, 2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 3);
}

void test_row_slicing_operation_1d_single_negetive()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice_row(a, -2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 7);
}

void test_row_slicing_operation_2d_single_row()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, 2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicVector<double>{10, 2, 30, 40, 5, 60, 70, 80});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(2,4))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{10, 2, 30, 40, 5, 60, 70, 80},
            {101, 102, 103, 104, 105, 106, 107, 108}});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(1,5,2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{11, 12, 13, 14, 15, 16, 17, 18},
            {101, 102, 103, 104, 105, 106, 107, 108}});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(5,1,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{311, 132, 313, 134, 135, 136, 137, 318},
            {101, 102, 103, 104, 105, 106, 107, 108}});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(-5,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {11, 12, 13, 14, 15, 16, 17, 18}, {10, 2, 30, 40, 5, 60, 70, 80},
        {101, 102, 103, 104, 105, 106, 107, 108}});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d_single_slice_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(-5,-4))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{11, 12, 13, 14, 15, 16, 17, 18}});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d_negative_index_zero_start()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(0,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{1, 2, 3, 4, 5, 6, 7, 8},
            {11, 12, 13, 14, 15, 16, 17, 18}, {10, 2, 30, 40, 5, 60, 70, 80},
            {101, 102, 103, 104, 105, 106, 107, 108}});

    HPX_TEST_EQ(result, expected);
}

void test_row_slicing_operation_2d_negative_index_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice_row(input, '(-1,-4,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{311, 132, 313, 134, 135, 136, 137, 318},
            {101, 102, 103, 104, 105, 106, 107, 108}});

    HPX_TEST_EQ(result, expected);
}

int main(int argc, char* argv[])
{
    test_row_slicing_operation_0d();

    test_row_slicing_operation_1d();
    test_row_slicing_operation_1d_step();
    test_row_slicing_operation_1d_neg_step();

    test_row_slicing_operation_1d_negative_index();
    test_row_slicing_operation_1d_single_slice_negative_index();
    test_row_slicing_operation_1d_negative_index_zero_start();
    test_row_slicing_operation_1d_negative_index_neg_step();
    test_row_slicing_operation_1d_single();
    test_row_slicing_operation_1d_single_negetive();

    test_row_slicing_operation_2d_single_row();
    test_row_slicing_operation_2d();
    test_row_slicing_operation_2d_step();
    test_row_slicing_operation_2d_neg_step();

    test_row_slicing_operation_2d_negative_index();
    test_row_slicing_operation_2d_single_slice_negative_index();
    test_row_slicing_operation_2d_negative_index_zero_start();
    test_row_slicing_operation_2d_negative_index_neg_step();

    return hpx::util::report_errors();
}
