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

void test_linmatrix()
{
    phylanx::execution_tree::primitive nx =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), std::int64_t(2));

    phylanx::execution_tree::primitive ny =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), std::int64_t(2));

    phylanx::execution_tree::primitive base_value =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), phylanx::ir::node_data<double>(1));

    phylanx::execution_tree::primitive dx =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), phylanx::ir::node_data<double>(0.5));

    phylanx::execution_tree::primitive dy =
    hpx::new_<phylanx::execution_tree::primitives::variable>(
        hpx::find_here(), phylanx::ir::node_data<double>(0.5));

    using namespace std;
    phylanx::execution_tree::primitive linearmatrix =
        hpx::new_<phylanx::execution_tree::primitives::linearmatrix>(
            hpx::find_here(),
            std::vector<phylanx::execution_tree::primitive_argument_type>{
                move(nx), move(ny), move(base_value), move(dx), move(dy)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        linearmatrix.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());

    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(blaze::DynamicMatrix <double>{{1.0, 1.5},
                                                                     {1.5, 2}}),
        result);
}

int main(int argc, char* argv[])
{
    test_linmatrix();
    return hpx::util::report_errors();
}
