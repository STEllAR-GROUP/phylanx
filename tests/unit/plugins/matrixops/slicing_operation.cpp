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
void test_slicing_operation_0d()
{
    std::string const code = R"(
        slice(42.0)
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 42.0);
}

void test_tuple_slicing_operation_0d()
{
    std::string const code = R"(
        tuple_slice(42.0, list())
    )";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 42.0);
}

void test_slicing_operation_1d()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(2,4))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 3);
    HPX_TEST_EQ(result[1], 4);
}

void test_tuple_slicing_operation_1d()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        tuple_slice(a, list(2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 3);
}

void test_slicing_operation_1d_start()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 3);
}

void test_slicing_operation_1d_start_negative()
{
    std::string const code = R"(block(
        define(a, [1, 2, 3, 4, 5, 6, 7, 8]),
        slice(a, list(-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 7);
}

void test_slicing_operation_1d_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(2,6,2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 3);
    HPX_TEST_EQ(result[1], 5);
}

void test_slicing_operation_1d_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(6,2,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 7);
    HPX_TEST_EQ(result[1], 5);
}

void test_slicing_operation_1d_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(-6,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(4));
    HPX_TEST_EQ(result[0], 3);
    HPX_TEST_EQ(result[1], 4);
    HPX_TEST_EQ(result[2], 5);
    HPX_TEST_EQ(result[3], 6);
}

void test_slicing_operation_1d_single_slice_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(-6,-5))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 3);
}

void test_slicing_operation_1d_negative_index_zero_start()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(0,-1))
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

void test_slicing_operation_1d_negative_index_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, list(-2,-6,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(2));
    HPX_TEST_EQ(result[0], 7);
    HPX_TEST_EQ(result[1], 5);
}

void test_slicing_operation_1d_single()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, 2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 3);
}

void test_slicing_operation_1d_single_negative()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        slice(a, -2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 7);
}

void test_slicing_operation_2d_value()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, 2, 2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 30);
}

void test_slicing_operation_2d_single_row()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, 2, list(0, 8))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicVector<double>{10, 2, 30, 40, 5, 60, 70, 80});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_single_row_v2()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(0, 6), 2)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicVector<double>{3, 13, 30, 103, 33, 313});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_single_row_result_matrix()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(2,3), list(0,8))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{10, 2, 30, 40, 5, 60, 70, 80}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(2,4), list(2,4))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{30, 40}, {103, 104}});

    HPX_TEST_EQ(result, expected);
}

void test_tuple_slicing_operation_2d_value()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        tuple_slice(input, list(2, 2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 30);
}

void test_tuple_slicing_operation_2d()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        tuple_slice(input, list(list(2,4), list(2,4)))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{30, 40}, {103, 104}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(1,5,2), list(1,5,2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{12, 14}, {102, 104}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(5,1,-2), list(5,1,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{136, 134}, {106, 104}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(-5,-2), list(-5,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {14, 15, 16}, {40, 5, 60}, {104, 105, 106}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_single_slice_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(-5,-4),list(0,8))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{11, 12, 13, 14, 15, 16, 17, 18}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_single_slice_negative_index_v2()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(0,6),list(-5,-4))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{4}, {14}, {40}, {104}, {34}, {134}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_negative_index_zero_start()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(0,-2), list(0,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {1, 2, 3, 4, 5, 6}, {11, 12, 13, 14, 15, 16}, {10, 2, 30, 40, 5, 60},
        {101, 102, 103, 104, 105, 106}});

    HPX_TEST_EQ(result, expected);
}

void test_slicing_operation_2d_negative_index_neg_step()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(e, [31,32,33,34,35,36,37,83]),
        define(f, [311,132,313,134,135,136,137,318]),
        define(input, vstack(list(a,b,c,d,e,f))),
        slice(input, list(-1,-4,-2), list(-1,-4,-2))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(
        blaze::DynamicMatrix<double>{{318, 136}, {108, 106}});

    HPX_TEST_EQ(result, expected);
}

void test_set_index_scalar()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55]),
        slice(a, 1)
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.num_dimensions(), std::size_t(0));
    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 0.95);
}

void test_set_index_list()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55]),
        slice(a, list(1, 2, 1))
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.num_dimensions(), std::size_t(1));
    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 0.95);
}

void test_slicing_operation_3d_value()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(x1, vstack(list(a,b,c,d))),
        define(x2, vstack(list(b,c,d,a))),
        define(x3, vstack(list(c,d,a,b))),
        define(input, dstack(list(x1, x2, x3))),
        slice(input, 0, 1, 2)
    ))";

    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result.num_dimensions(), std::size_t(0));
    HPX_TEST_EQ(result[0], 2);
}

void test_tuple_slicing_operation_3d_value()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(x1, vstack(list(a,b,c,d))),
        define(x2, vstack(list(b,c,d,a))),
        define(x3, vstack(list(c,d,a,b))),
        define(input, dstack(list(x1, x2, x3))),
        tuple_slice(input, list(0, 1, 2))
    ))";

    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result.num_dimensions(), std::size_t(0));
    HPX_TEST_EQ(result[0], 2);
}

void test_slicing_operation_3d_value_negative_index()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(x1, vstack(list(a,b,c,d))),
        define(x2, vstack(list(b,c,d,a))),
        define(x3, vstack(list(c,d,a,b))),
        define(input, dstack(list(x1, x2, x3))),
        slice(input, -1, -2, -3)
    ))";

    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result.num_dimensions(), std::size_t(0));
    HPX_TEST_EQ(result[0], 107);
}

void test_slicing_operation_3d_single_slice()
{
    std::string const code = R"(block(
        define(a, [1,2,3,4,5,6,7,8]),
        define(b, [11,12,13,14,15,16,17,18]),
        define(c, [10,02,30,40,05,60,70,80]),
        define(d, [101,102,103,104,105,106,107,108]),
        define(x1, vstack(list(a,b,c,d))),
        define(x2, vstack(list(b,c,d,a))),
        define(x3, vstack(list(c,d,a,b))),
        define(input, dstack(list(x1, x2, x3))),
        slice(input, list(0, 1), list(1, 2), list(2, 3))
    ))";

    auto result =
        phylanx::execution_tree::extract_integer_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result.num_dimensions(), std::size_t(3));
    HPX_TEST_EQ(result.tensor()(0, 0, 0), 2);
}

int main(int argc, char* argv[])
{
    test_slicing_operation_0d();

    test_slicing_operation_1d();
    test_slicing_operation_1d_start();
    test_slicing_operation_1d_start_negative();
    test_slicing_operation_1d_step();
    test_slicing_operation_1d_neg_step();

    test_slicing_operation_1d_negative_index();
    test_slicing_operation_1d_single_slice_negative_index();
    test_slicing_operation_1d_negative_index_zero_start();
    test_slicing_operation_1d_negative_index_neg_step();
    test_slicing_operation_1d_single();
    test_slicing_operation_1d_single_negative();

    test_slicing_operation_2d_value();
    test_slicing_operation_2d_single_row();
    test_slicing_operation_2d_single_row_v2();
    test_slicing_operation_2d_single_row_result_matrix();
    test_slicing_operation_2d();
    test_slicing_operation_2d_step();
    test_slicing_operation_2d_neg_step();

    test_slicing_operation_2d_negative_index();
    test_slicing_operation_2d_single_slice_negative_index();
    test_slicing_operation_2d_single_slice_negative_index_v2();
    test_slicing_operation_2d_negative_index_zero_start();
    test_slicing_operation_2d_negative_index_neg_step();

    test_set_index_scalar();
    test_set_index_list();

    test_slicing_operation_3d_value();
    test_slicing_operation_3d_value_negative_index();

    test_slicing_operation_3d_single_slice();

    test_tuple_slicing_operation_0d();
    test_tuple_slicing_operation_1d();
    test_tuple_slicing_operation_2d_value();
    test_tuple_slicing_operation_2d();

    test_tuple_slicing_operation_3d_value();

    return hpx::util::report_errors();
}
