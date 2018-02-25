//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #245: PhySL Modifies Untouched Variable

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

std::string const code = R"(
    define(A, [[1, 2, 3], [4, 5, 6]])
    define(B, [[3, 4, 5], [2, 7, 9]])
    store(A, B + A)
    debug(A)
    debug(B)
)";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile(code, snippets);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(),
        std::string("[[4, 6, 8], [6, 12, 15]]\n[[3, 4, 5], [2, 7, 9]]\n"));

    return hpx::util::report_errors();
}
