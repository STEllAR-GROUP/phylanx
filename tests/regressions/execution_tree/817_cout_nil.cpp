// Copyright (c) 2019 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #817: PhySL: lambda returning nil

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <sstream>
#include <string>

int hpx_main(int argc, char* argv[])
{
    char const* const codestr = R"(
        define(test, x, debug(x))
        test(nil)
    )";

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(
        "test", codestr, snippets, env);
    auto test = code.run();

    test();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string("nil\n"));

    return hpx::util::report_errors();
}
