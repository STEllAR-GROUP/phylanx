//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #244: Can not create a list or a vector of previously defined variables

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

std::string const codestr = R"(block(
    define(
        f,
        block(
            define(a, 0),
            map(
                lambda(
                    i,
                    block(
                        store(a, 0)
                    )
                ),
                make_list(1)
            ),
            a
        )
    ),
    f
))";

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code = phylanx::execution_tree::compile(codestr, snippets);
    auto f = code.run();

    f();

    return hpx::util::report_errors();
}

