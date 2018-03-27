//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #333: Functions do not assign variables properly

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

std::string const code = R"(block(
    define(newton, xn, block(
        define(newton_fn, a, b, c,
            debug("xn ", a, " error ", b, " max_iter ", c)
        ),
        newton_fn(xn, 1.0, 10.0)
    )),
    newton
))";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto newton = phylanx::execution_tree::compile(code, snippets);

    newton(3.0);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string("xn 3 error 1 max_iter 10\n"));

    return hpx::util::report_errors();
}
