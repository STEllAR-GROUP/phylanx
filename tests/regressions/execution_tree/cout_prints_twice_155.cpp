//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #155: console output prints twice if it takes an argument

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/testing.hpp>

#include <string>
#include <sstream>

std::string const codestr = R"(block(
    define(pr, a, debug("hello: ", a)),
    pr
))";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto pr = code.run();

    pr(42.0);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string("hello: 42\n"));

    return hpx::util::report_errors();
}
