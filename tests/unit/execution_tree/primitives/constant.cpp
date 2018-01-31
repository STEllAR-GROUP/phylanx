//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

void test_constant_0d()
{
    phylanx::execution_tree::primitive val =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive const_ =
        hpx::new_<phylanx::execution_tree::primitives::constant>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        const_.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(result.num_dimensions(), 0);
    HPX_TEST_EQ(42.0, result[0]);
}

void test_constant_1d()
{
    phylanx::execution_tree::primitive val =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive const_ =
        hpx::new_<phylanx::execution_tree::primitives::constant>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val), std::int64_t(1007)
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        const_.eval();

    blaze::DynamicVector<double> expected =
        blaze::DynamicVector<double>(1007UL, 42.0);
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(result.num_dimensions(), 1);
    HPX_TEST_EQ(result.dimension(0), 1007);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}

void test_constant_2d()
{
    phylanx::execution_tree::primitive val =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive const_ =
        hpx::new_<phylanx::execution_tree::primitives::constant>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                std::move(val),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    std::int64_t(105), std::int64_t(101)
                }
            });

    hpx::future<phylanx::execution_tree::primitive_result_type> f =
        const_.eval();

    blaze::DynamicMatrix<double> expected =
        blaze::DynamicMatrix<double>(105UL, 101UL, 42.0);
    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(result.num_dimensions(), 2);
    HPX_TEST_EQ(result.dimension(0), 105);
    HPX_TEST_EQ(result.dimension(1), 101);
    HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)), result);
}

int main(int argc, char* argv[])
{
    test_constant_0d();
    test_constant_1d();
    test_constant_2d();

    return hpx::util::report_errors();
}

