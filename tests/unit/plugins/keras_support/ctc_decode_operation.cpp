// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

#include <cstdint>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
void test_ctc_decode_operation_1()
{
    blaze::DynamicTensor<double> arg1{
        {{0.2, 0.2, 0.6}, {0.4, 0.3, 0.3}}, {{0.7, 0.15, 0.15}, {0., 0., 0.}}};
    blaze::DynamicVector<std::int64_t> arg2{2, 1};

    phylanx::execution_tree::primitive y_pred =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(arg1));
    phylanx::execution_tree::primitive input_length =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(arg2));
    phylanx::execution_tree::primitive greedy =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(1));
    phylanx::execution_tree::primitive beam_width =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(10));
    phylanx::execution_tree::primitive top_paths =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(10));

    phylanx::execution_tree::primitive ctc_decode =
        phylanx::execution_tree::primitives::create_ctc_decode_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(y_pred),
                std::move(input_length), std::move(greedy),
                std::move(beam_width), std::move(top_paths)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        ctc_decode.eval();
    auto result = phylanx::execution_tree::extract_list_value(f.get());
    auto it = result.begin();
    auto first = phylanx::execution_tree::extract_list_value(*it);

    blaze::DynamicMatrix<double> expected_decoded_dense{{0.}, {0.}};
    blaze::DynamicMatrix<double> expected_log_prob{{1.42711636}, {0.35667494}};

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected_decoded_dense)),
        phylanx::execution_tree::extract_numeric_value(*first.begin()));
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected_log_prob)),
            phylanx::execution_tree::extract_numeric_value(*++it)));
}

void test_ctc_decode_operation_2()
{
    blaze::DynamicTensor<double> arg1{
        {{1., 0., 0., 0.}, {0., 0., 0.4, 0.6}, {0., 0., 0.4, 0.6},
            {0., 0.9, 0.1, 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}},
        {{0.1, 0.9, 0., 0.}, {0., 0.9, 0.1, 0.}, {0., 0., 0.1, 0.9},
            {0., 0.9, 0.1, 0.1}, {0.9, 0.1, 0., 0.}, {0., 0., 0., 0.}}};
    blaze::DynamicVector<std::int64_t> arg2{4, 5};

    phylanx::execution_tree::primitive y_pred =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(arg1));
    phylanx::execution_tree::primitive input_length =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(arg2));
    phylanx::execution_tree::primitive greedy =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::uint8_t>(1));
    phylanx::execution_tree::primitive beam_width =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(10));
    phylanx::execution_tree::primitive top_paths =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(10));

    phylanx::execution_tree::primitive ctc_decode =
        phylanx::execution_tree::primitives::create_ctc_decode_operation(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(y_pred),
                std::move(input_length), std::move(greedy),
                std::move(beam_width), std::move(top_paths)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        ctc_decode.eval();
    auto result = phylanx::execution_tree::extract_list_value(f.get());
    auto it = result.begin();
    auto first = phylanx::execution_tree::extract_list_value(*it);

    blaze::DynamicMatrix<double> expected_decoded_dense{
        {0., 1., -1.}, {1., 1., 0.}};
    blaze::DynamicMatrix<double> expected_log_prob{{1.12701166}, {0.52680272}};

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(std::move(expected_decoded_dense)),
        phylanx::execution_tree::extract_numeric_value(*first.begin()));
    HPX_TEST(
        allclose(phylanx::ir::node_data<double>(std::move(expected_log_prob)),
            phylanx::execution_tree::extract_numeric_value(*++it)));
}

int main(int argc, char* argv[])
{
    test_ctc_decode_operation_1();
    test_ctc_decode_operation_2();

    return hpx::util::report_errors();
}

