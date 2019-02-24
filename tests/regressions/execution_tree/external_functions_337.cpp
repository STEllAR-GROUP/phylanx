// Copyright (c) 2018 Adrian Serio
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// #337: Issues with external functions

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <string>
#include <sstream>

int hpx_main()
{
    // External Function
    char const* const external_code = R"(
        define(extern_print, var, debug("I am external!: ", var))
    )";

    // Test calling a function defined in another string.
    char const* const main_code = R"(block(
        define(print, var, debug(var)),
        define(test_fn, a, block(
            print(a),
            extern_print(a)
        )),
        define(main_func, x, test_fn(x)),
        main_func
    ))";

    phylanx::execution_tree::eval_context ctx;

    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment envir =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code1 = phylanx::execution_tree::compile(
        "extern_print", external_code, snippets, envir);
    auto external_ = code1.run(ctx);
    external_(ctx, 4.0);

    auto const& code2 = phylanx::execution_tree::compile(
        "main_func", main_code, snippets, envir);
    auto main_ = code2.run(ctx);
    main_(ctx, 3.0);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(),
        std::string("I am external!: 4\n3\nI am external!: 3\n"));

    return hpx::util::report_errors();
}
