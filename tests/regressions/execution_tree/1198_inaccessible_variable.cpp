// Copyright (c) 2020 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #1198: inaccessible variable

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/modules/testing.hpp>

#include <utility>

#include <blaze/Math.h>

int main(int argc, char* argv[])
{
    // External Function
    char const* const codestr = R"(block(
        define(test, ztot, block(
            define(t, 3),
            store(slice(ztot, t), __sub(slice(ztot, t), 1.0)),
            ztot
        )),
        test
    ))";

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile("test", codestr, snippets, env);
    auto test = code.run();

    blaze::DynamicVector<double> v(10, 3.0);
    auto result =
        test(phylanx::execution_tree::primitive_argument_type{std::move(v)});

    auto ztot = phylanx::execution_tree::extract_numeric_value_strict(
        std::move(result));

    blaze::DynamicVector<double> v1(10, 3.0);
    v1[3] = 2.0;
    HPX_TEST_EQ(ztot.vector(), v1);

    return hpx::util::report_errors();
}
