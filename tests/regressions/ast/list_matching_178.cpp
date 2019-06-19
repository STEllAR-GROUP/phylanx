// Copyright (c) 2018 Bibek Wagle
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #178: Can not create a list in physl

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>
#include <vector>

std::string const read_code = R"(block(
    define(somelist, list(1, 2)),
    somelist
))";

int main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile(read_code, snippets);
    auto somelist = code.run();

    auto result = somelist();

    namespace pe = phylanx::execution_tree;

    pe::primitive_argument_type expected =
        pe::primitive_argument_type{pe::primitive_arguments_type{
            pe::primitive_argument_type{std::int64_t(1)},
            pe::primitive_argument_type{std::int64_t(2)}}};

    HPX_TEST(expected == result);
    return hpx::util::report_errors();
}
