//   Copyright (c) 2017 Alireza Kheirkhahan
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_literal_value()
{
    phylanx::execution_tree::primitive lhs =
            hpx::new_<phylanx::execution_tree::primitives::literal_value>(
                    hpx::find_here(), phylanx::ir::node_data<double>(0.0));

    phylanx::execution_tree::primitive rhs =
            hpx::new_<phylanx::execution_tree::primitives::literal_value>(
                    hpx::find_here(), phylanx::ir::node_data<double>(42.0));

    phylanx::execution_tree::primitive store =
            hpx::new_<phylanx::execution_tree::primitives::store_operation>(
                    hpx::find_here(),
                    std::vector<phylanx::execution_tree::primitive_argument_type>{
                            std::move(lhs), std::move(rhs)
                    });
    hpx::future<phylanx::util::optional<phylanx::ir::node_data<double>>> result =
            store.eval();

    HPX_TEST_EQ(42.0, lhs.get().value()[0]);
    HPX_TEST_EQ(42.0, result.get().value()[0]);
}

int main(int argc, char* argv[])
{
    test_literal_value();

    return hpx::util::report_errors();
}
