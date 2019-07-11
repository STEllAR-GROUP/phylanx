//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #232: physl interpreter variable referenced-out-of-scope error message

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/testing.hpp>

#include <string>

std::string const code = R"(block(
    define(sum,
        block(
            for(define(n, 0), n < 10, store(n, n+1), cout(n)),
            n
        )
    ),
    sum
))";

int main(int argc, char* argv[])
{
    bool caught_exception = false;
    try
    {
        phylanx::execution_tree::compiler::function_list snippets;
        phylanx::execution_tree::compile("code", code, snippets);
    }
    catch (hpx::exception const& e)
    {
        caught_exception = true;
        HPX_TEST_EQ(std::string(e.what()),
            "code(5, 13): couldn't find variable 'n' in symbol table: "
            "HPX(bad_parameter)");
    }
    HPX_TEST(caught_exception);

    return hpx::util::report_errors();
}
