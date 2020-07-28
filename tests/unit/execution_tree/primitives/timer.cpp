//   Copyright (c) 2020 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/iostream.hpp>
#include <hpx/modules/testing.hpp>

#include <sstream>
#include <string>

void test_timer_operation(char const* expr, double expected)
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(expr, snippets);
    auto f = code.run();

    HPX_TEST_EQ(expected,
        phylanx::execution_tree::extract_numeric_value(
            f()
        )[0]);
}

int hpx_main(int argc, char* argv[])
{
    test_timer_operation(R"(
        timer(
            block(
                define(x, 3.14),
                x
            ),
            lambda(x, debug("time: ", x, " [s]"))
        )
    )", 3.14);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    std::string result = strm.str();
    HPX_TEST(result.find("time: ") == 0);
    HPX_TEST(result.find(" [s]") == result.size() - 5);

    return hpx::util::report_errors();
}
