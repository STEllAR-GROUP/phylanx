//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #244: Can not create a list or a vector of previously defined variables

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

std::string const code = R"(block(
    define(f, a, block(
        define(b, 6),
        define(vec1, hstack(a, b, 42)),
        debug(vec1),
        define(vec2, hstack(a, vec1, b)),
        debug(vec2)
    )),
    f
))";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compile(code, snippets);
    auto f = snippets.run();

    f(5.0);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string("[5, 6, 42]\n[5, 5, 6, 42, 6]\n"));

    return hpx::util::report_errors();
}
