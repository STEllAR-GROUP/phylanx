//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::string read_user_code(char* const arg)
{
    std::string code;
    std::ifstream code_stream(arg);

    // Find out how much memory we need to allocate
    code_stream.seekg(0, std::ios::end);
    // Allocate all the needed memory upfront
    code.reserve(code_stream.tellg());
    // Back to the beginning of the file
    code_stream.seekg(0, std::ios::beg);

    // Read the file
    code.assign((std::istreambuf_iterator<char>(code_stream)),
        std::istreambuf_iterator<char>());

    return code;
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    // Read the file containing the source code
    auto const user_code = read_user_code(argv[1]);

    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto const fibonacci = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(user_code), snippets);

    // Evaluate user code using the read data
    auto const result = fibonacci();

    std::cout << result << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}
