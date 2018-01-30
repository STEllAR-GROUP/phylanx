//   Copyright (c) 2018 R. Tohid
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include<cstdint>
#include <utility>
#include <vector>

void test_linspace0d()
{
    phylanx::execution_tree::primitive start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive stop =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), phylanx::ir::node_data<double>(45.0));

    phylanx::execution_tree::primitive num_samples =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), std::int64_t(1));

    phylanx::execution_tree::primitive linspace =
        hpx::new_<phylanx::execution_tree::primitives::linspace>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(start), std::move(stop), std::move(num_samples)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        linspace.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(blaze::DynamicVector <double>{42.0}),
        result);
}

void test_linspace1d()
{
    phylanx::execution_tree::primitive start =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive stop =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), phylanx::ir::node_data<double>(45.0));

    phylanx::execution_tree::primitive num_samples =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), std::int64_t(4));

    phylanx::execution_tree::primitive linspace =
        hpx::new_<phylanx::execution_tree::primitives::linspace>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(start), std::move(stop), std::move(num_samples)});

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        linspace.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(
            blaze::DynamicVector <double>{42.0, 43.0, 44.0, 45.0}), result);
}

int main(int argc, char* argv[])
{
    test_linspace0d();
    test_linspace1d();
    return hpx::util::report_errors();
}
