//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/include/performance_counters.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [30, 2]
    //   y: [30]
    define(lra, x, y, alpha,
        block(
            define(weights, constant(0.0, shape(x, 1))),     // weights: [2]
            define(transx, transpose(x)),                    // transx:  [2, 30]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < 10,
                block(
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),
                    store(error, pred - y),                  // error: [30]
                    store(gradient, dot(transx, error)),     // gradient: [2]
                    parallel_block(
                        store(weights, weights - (alpha * gradient)),
                        store(step, step + 1)
                    )
                )
            ),
            weights
        )
    ),
    lra
))";

std::map<std::string, std::size_t> expected_counts =
{
    { "block", 3 },
    { "parallel_block", 1 },
    { "dot", 2 },
    { "while", 1 },
    { "constant", 4 },
    { "exp", 1 },
    { "shape", 4 },
    { "transpose", 1 },
    { "__add", 2 },
    { "__div", 1 },
    { "__mul", 1 },
    { "__sub", 2 },
    { "__lt", 1 },
    { "store", 5 },
    { "__minus", 1 },
    { "access-argument", 8 },
    { "access-variable", 14 },
    { "define-variable", 6 },
    { "call-function", 1 },
    { "define-function", 1 },
    { "variable", 6 },
};

int main()
{
    blaze::DynamicMatrix<double> const v1{{15.04, 16.74}, {13.82, 24.49},
        {12.54, 16.32}, {23.09, 19.83}, {9.268, 12.87}, {9.676, 13.14},
        {12.22, 20.04}, {11.06, 17.12}, {16.3 , 15.7 }, {15.46, 23.95},
        {11.74, 14.69}, {14.81, 14.7 }, {13.4 , 20.52}, {14.58, 13.66},
        {15.05, 19.07}, {11.34, 18.61}, {18.31, 20.58}, {19.89, 20.26},
        {12.88, 18.22}, {12.75, 16.7 }, {9.295, 13.9 }, {24.63, 21.6 },
        {11.26, 19.83}, {13.71, 18.68}, {9.847, 15.68}, {8.571, 13.1 },
        {13.46, 18.75}, {12.34, 12.27}, {13.94, 13.17}, {12.07, 13.44}};

    blaze::DynamicVector<double> const v2{
        1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};

    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const lra = phylanx::execution_tree::compile(lra_code, snippets);

    // Evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{v1};
    auto y = phylanx::ir::node_data<double>{v2};
    auto alpha = phylanx::ir::node_data<double>{1e-5};

    auto result = lra(x, y, alpha);

    // Test the performance counters of all primitives
    for (auto const& pattern :
        phylanx::execution_tree::get_all_known_patterns())
    {
        std::string const& name = hpx::util::get<0>(hpx::util::get<1>(pattern));

        // HACK: There is an access-argument/access-variable primitive
        // registered in AGAS for each argument/variable access. This is a
        // problem because several instances with the same sequence ids may
        // exist and thus we ignore these two primitives for now.
        if (name == "access-argument" || name == "access-variable")
            continue;

        std::string const count_pc_name(
            "/phylanx{locality#0/total}/primitives/" + name + "/count/eval");
        hpx::performance_counters::performance_counter count_pc(
            count_pc_name);

        std::string const time_pc_name(
            "/phylanx{locality#0/total}/primitives/" + name + "/time/eval");
        hpx::performance_counters::performance_counter time_pc(time_pc_name);

        std::string const eval_pc_name(
            "/phylanx{locality#0/total}/primitives/" + name + "/eval_direct");
        hpx::performance_counters::performance_counter eval_pc(eval_pc_name);

        // Verify the number of primitive instances in lra
        auto entries = hpx::agas::find_symbols(
            hpx::launch::sync, "/phylanx/" + name + "$*");
        HPX_TEST_EQ(expected_counts[name], entries.size());

        // Time performance counters
        {
            auto const info = time_pc.get_info(hpx::launch::sync);
            HPX_TEST_EQ(info.fullname_, time_pc_name);
            HPX_TEST_EQ(
                info.type_, hpx::performance_counters::counter_raw_values);

            auto values =
                time_pc.get_counter_values_array(hpx::launch::sync, false);

            HPX_TEST_EQ(values.count_, 1ll);
            HPX_TEST_EQ(values.values_.size(), entries.size());

            // At least one of the values for each primitive should be non-zero
            for (std::size_t i = 0; i != values.values_.size(); ++i)
            {
                HPX_TEST(values.values_[i] != 0ll);
            }
        }

        // Count performance counters
        {
            auto const info = count_pc.get_info(hpx::launch::sync);
            HPX_TEST_EQ(info.fullname_, count_pc_name);
            HPX_TEST_EQ(
                info.type_, hpx::performance_counters::counter_raw_values);

            auto const values =
                count_pc.get_counter_values_array(hpx::launch::sync, false);

            HPX_TEST_EQ(values.count_, 1ll);
            HPX_TEST_EQ(values.values_.size(), entries.size());

            // At least one of the values for each primitive should be non-zero
            for (std::size_t i = 0; i != values.values_.size(); ++i)
            {
                HPX_TEST(values.values_[i] != 0ll);
            }
        }

        // Eval-direct performance counters
        {
            auto const info = eval_pc.get_info(hpx::launch::sync);
            HPX_TEST_EQ(info.fullname_, eval_pc_name);
            HPX_TEST_EQ(
                info.type_, hpx::performance_counters::counter_raw_values);

            auto const values =
                eval_pc.get_counter_values_array(hpx::launch::sync, false);

            HPX_TEST_EQ(values.count_, 1ll);
            HPX_TEST_EQ(values.values_.size(), entries.size());

            // all values should be '0' or '1'
            for (std::size_t i = 0; i != values.values_.size(); ++i)
            {
                HPX_TEST(values.values_[i] == -1 || values.values_[i] == 0 ||
                    values.values_[i] == 1);
            }
        }
    }

    return hpx::util::report_errors();
}
