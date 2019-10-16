// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <string>

//////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

//////////////////////////////////////////////////////////////////////////
void test_range_stop()
{
    char const* const code = "fmap(lambda(x, x), range(2))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 2);

    std::int64_t c = 0;
    for (auto const& i : result)
    {
        HPX_TEST_EQ(i, phylanx::execution_tree::primitive_argument_type{c++});
    }
}

void test_range_start_stop()
{
    char const* const code = "fmap(lambda(x, x), range(-1, 2))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 3);

    std::int64_t c = -1;
    for (auto const& i : result)
    {
        HPX_TEST_EQ(i, phylanx::execution_tree::primitive_argument_type{c++});
    }
}

void test_range_start_stop_step()
{
    char const* const code = "fmap(lambda(x, x), range(-3, 2, 4))";

    auto result =
        phylanx::execution_tree::extract_list_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), 2);

    std::int64_t c = -3;
    for (auto const& i : result)
    {
        HPX_TEST_EQ(i, phylanx::execution_tree::primitive_argument_type{c});
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

