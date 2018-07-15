// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <utility>

void test_formatting(
    std::string const& codestr, std::string const& expected)
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto f = code.run();

    HPX_TEST_EQ(phylanx::execution_tree::extract_string_value(f()), expected);
}

int main(int argc, char* argv[])
{
    test_formatting(R"(format("abc"))", "abc");
    test_formatting(R"(format("test: {:s}", "abc"))", "test: abc");
    test_formatting(R"(format("test: {:s}", 42))", "test: 42");
    test_formatting(R"(format("test: {:lf}", 42.0))", "test: 42.000000");
    test_formatting(R"(format("test: {:6.3lf}", 42.0))", "test: 42.000");
    test_formatting(R"(format("test: {:d}", 42.0))", "test: 42");

    test_formatting(R"(format("test: {2:d}, {1:lf}", 42.0, 43.0))",
        "test: 43, 42.000000");

    return hpx::util::report_errors();
}
