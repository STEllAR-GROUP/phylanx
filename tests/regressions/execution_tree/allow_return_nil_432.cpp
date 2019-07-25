// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #432: Cannot return nil in PhySL

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/testing.hpp>

#include <string>
#include <sstream>

void compile(std::string const code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compile(code, snippets);
}

int hpx_main(int argc, char* argv[])
{
    compile(R"(
        define(f, block(
            nil
        ))
        cout(f())
    )");

    compile(R"(
        define(f, block(
            0,
            nil
        ))
        cout(f())
    )");

    compile(R"(
        define(f, block(
            nil,
            0
        ))
        cout(f())
    )");

    compile(R"(
        define(f, block(
            0,
            nil,
            0
        ))
        cout(f())
    )");

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string(""));

    return hpx::util::report_errors();
}
