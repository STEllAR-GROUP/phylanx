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
#include <utility>
#include <vector>

std::string read_user_code(char const* const arg)
{
    std::string code;
    std::ifstream code_stream(arg);

    if (!code_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "read_user_code",
            "Filed to open the specified file.");
    }

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

std::vector<phylanx::execution_tree::primitive_argument_type> read_arguments(
    int const argc,
    char const* const argv[])
{
    std::vector<phylanx::execution_tree::primitive_argument_type> result;
    result.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        auto arg = phylanx::ast::generate_ast(argv[i]);
        result.emplace_back(
            phylanx::execution_tree::primitive_argument_type(std::move(arg)));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " [<hpx arguments --]"
                  << " <PhySL snippet file> [<argument> ...]" << std::endl
                  << "Example: " << argv[0] << " fibonacci.physl 10"
                  << std::endl;
        return hpx::finalize();
    }

    // Read the file containing the source code
    auto const user_code = read_user_code(argv[1]);

    // Collect the arguments for running the code
    auto const args = read_arguments(argc - 2, argv + 2);

    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;

    auto const code = phylanx::execution_tree::compile(user_code, snippets);

    // Evaluate user code using the read data
    auto const result = code(args);

    std::cout << result << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}

