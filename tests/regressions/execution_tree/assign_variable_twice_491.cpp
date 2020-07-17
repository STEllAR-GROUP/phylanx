//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #491: Variables PhySL functions are written to only once

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/distributed/iostream.hpp>
#include <hpx/modules/testing.hpp>

#include <string>
#include <sstream>

std::string const codestr = R"(block(
    define(f, a, block(
        define(b, a),
        debug(a),
        debug(b)
    )),
    f(1),
    f(2)
))";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto f = code.run();

    f();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string("1\n1\n2\n2\n"));

    return hpx::util::report_errors();
}
