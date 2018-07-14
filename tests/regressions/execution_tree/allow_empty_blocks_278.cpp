// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #278: Python to PhySL Translation generates empty block #278

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile("block()", snippets);
    auto f = code.run();

    auto result = f();
    HPX_TEST_EQ(result, phylanx::execution_tree::primitive_argument_type{});

    return hpx::util::report_errors();
}
