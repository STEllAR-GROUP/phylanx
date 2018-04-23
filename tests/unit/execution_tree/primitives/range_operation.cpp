// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>

//////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;

    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();
    return phylanx::execution_tree::compile(code, snippets, env);
}

//////////////////////////////////////////////////////////////////////////
void test_range_stop()
{
    char const* const code = "map(lambda(x, x), range(2))";

    auto result = phylanx::execution_tree::extract_list_value(compile(code)());

    HPX_TEST_EQ(result.size(), 2);

    int c = 0;
    for (auto const& i : result)
    {
        HPX_TEST_EQ(i, c++);
    }
}

void test_range_start_stop()
{
    char const* const code = "map(lambda(x, x), range(-1, 2))";

    auto result = phylanx::execution_tree::extract_list_value(compile(code)());

    HPX_TEST_EQ(result.size(), 3);

    int c = -1;
    for (auto const& i : result)
    {
        HPX_TEST_EQ(i, c++);
    }
}

void test_range_start_stop_step()
{
    char const* const code = "map(lambda(x, x), range(-3, 2, 4))";

    auto result = phylanx::execution_tree::extract_list_value(compile(code)());

    HPX_TEST_EQ(result.size(), 2);

    int c = -3;
    for (auto const& i : result)
    {
        HPX_TEST_EQ(i, c);
        c += 4;
    }
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    test_range_stop();
    test_range_start_stop();
    test_range_start_stop_step();

    return hpx::util::report_errors();
}

