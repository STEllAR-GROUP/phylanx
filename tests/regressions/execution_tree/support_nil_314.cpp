// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #314: Support 'nil' in PhySL

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;
    auto f = phylanx::execution_tree::compile("nil", snippets);

    auto result = f();
    HPX_TEST_EQ(result, phylanx::execution_tree::primitive_argument_type{});

    return hpx::util::report_errors();
}
