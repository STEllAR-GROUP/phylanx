//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

void test_literal_value()
{
    phylanx::execution_tree::primitive val =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(),
            phylanx::execution_tree::primitive_argument_type{42.0});

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        val.eval();

    HPX_TEST_EQ(
        42.0, phylanx::execution_tree::extract_numeric_value(f.get())[0]);
}

int main(int argc, char* argv[])
{
    test_literal_value();

    return hpx::util::report_errors();
}

