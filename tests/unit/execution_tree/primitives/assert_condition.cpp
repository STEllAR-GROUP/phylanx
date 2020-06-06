// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <utility>
#include <vector>

using arg_type = phylanx::execution_tree::primitive_argument_type;
using args_type = phylanx::execution_tree::primitive_arguments_type;

void test_assert_true()
{
    phylanx::execution_tree::primitive op =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), arg_type{true});

    phylanx::execution_tree::primitive assert_cond =
        phylanx::execution_tree::primitives::create_assert_condition(
            hpx::find_here(), args_type{op});

    hpx::future<arg_type> f = assert_cond.eval();

    HPX_TEST(!phylanx::execution_tree::valid(f.get()));
}

void test_assert_false()
{
    phylanx::execution_tree::primitive op =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), arg_type{false});

    phylanx::execution_tree::primitive assert_cond =
        phylanx::execution_tree::primitives::create_assert_condition(
            hpx::find_here(),
            args_type{op});

    hpx::future<arg_type> f =
        assert_cond.eval();

    bool exception_thrown = false;
    try
    {
        // Must throw an exception
        f.get();
        HPX_TEST(false);
    }
    catch (std::exception const&)
    {
        exception_thrown = true;
    }

    HPX_TEST(exception_thrown);
}

int main(int argc, char* argv[])
{
    test_assert_true();
    test_assert_false();

    return hpx::util::report_errors();
}
