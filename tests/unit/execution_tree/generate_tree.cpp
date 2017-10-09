//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_generate_tree(std::string const& exprstr,
    phylanx::execution_tree::pattern_list const& patterns,
    phylanx::execution_tree::variables const& variables,
    double expected_result)
{
    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::generate_tree(exprstr, patterns, variables);

    HPX_TEST_EQ(
        phylanx::execution_tree::extract_numeric_value(p.eval().get())[0],
        expected_result);
}

phylanx::execution_tree::primitive create_literal_value(double value)
{
    return phylanx::execution_tree::primitive(
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(value)));
}

void test_add_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("A + B", patterns, variables, 42.0);
    test_generate_tree("A + (B + C)", patterns, variables, 55.0);
    test_generate_tree("A + (B + A)", patterns, variables, 83.0);
    test_generate_tree("(A + B) + C", patterns, variables, 55.0);
}

void test_sub_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::sub_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}};

    test_generate_tree("A - B", patterns, variables, 40.0);
    test_generate_tree("A - (B - C)", patterns, variables, 53.0);
    test_generate_tree("A - (B - A)", patterns, variables, 81.0);
    test_generate_tree("(A - B) - C", patterns, variables, 27.0);
}

void test_mul_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::mul_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}};

    test_generate_tree("A * B", patterns, variables, 41.0);
    test_generate_tree("A * (B * C)", patterns, variables, 533.0);
    test_generate_tree("A * (B * A)", patterns, variables, 1681.0);
    test_generate_tree("(A * B) * C", patterns, variables, 533.0);
}

void test_math_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data,
        phylanx::execution_tree::primitives::sub_operation::match_data,
        phylanx::execution_tree::primitives::mul_operation::match_data,
        phylanx::execution_tree::primitives::exponential_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)},
        {"D", create_literal_value(5.0)}
    };

    test_generate_tree("A + ((B - C) * A)", patterns, variables, -451.0);
    test_generate_tree("(A * (B + A)) + C", patterns, variables, 1735.0);
    test_generate_tree("(A * B) - C", patterns, variables, 28.0);
    test_generate_tree("exp(D) * 2", patterns, variables, std::exp(5.0) * 2);
    test_generate_tree("exp(D) + 2", patterns, variables, std::exp(5.0) + 2);
    test_generate_tree("exp(D) - 2", patterns, variables, std::exp(5.0) - 2);
}

void test_file_io_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::file_read::match_data,
        phylanx::execution_tree::primitives::file_write::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("file_write(\"test_file\", A)", patterns, variables, 41.0);
    test_generate_tree("file_read(\"test_file\")", patterns, variables, 41.0);
}

void test_block_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data,
        phylanx::execution_tree::primitives::block_operation::match_data,
        phylanx::execution_tree::primitives::parallel_block_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("block(A, B, A + B + C)", patterns, variables, 55.0);
    test_generate_tree("parallel_block(A, B, A + B + C)", patterns,
        variables, 55.0);
}

int main(int argc, char* argv[])
{
    test_add_primitive();
    test_sub_primitive();
    test_mul_primitive();
    test_math_primitives();
    test_file_io_primitives();
    test_block_primitives();

    return hpx::util::report_errors();
}

