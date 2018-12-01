//   Copyright (c) 2017 Shahrzad Shirzad
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

void test_identity()
{
    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<std::int64_t>(5));

    phylanx::execution_tree::primitive identity =
        phylanx::execution_tree::primitives::create_identity(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{
                std::move(val)});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        identity.eval();

    auto result = phylanx::execution_tree::extract_numeric_value(f.get());
    HPX_TEST_EQ(
        phylanx::ir::node_data<double>(blaze::IdentityMatrix<double>(5UL)),
        result);
}

int main(int argc, char* argv[])
{
    test_identity();
    return hpx::util::report_errors();
}
