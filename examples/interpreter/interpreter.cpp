//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

std::string read_user_code(std::string const& arg)
{
    std::ifstream code_stream(arg);
    if (!code_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "read_user_code",
            "Failed to open the specified file: " + arg);
    }

    std::string code;
    // Find out how much memory we need to allocate
    code_stream.seekg(0, std::ios::end);
    // Allocate all the needed memory upfront
    code.reserve(code_stream.tellg());
    // Back to the beginning of the file
    code_stream.seekg(0, std::ios::beg);

    // Read the file
    code.assign(std::istreambuf_iterator<char>(code_stream),
        std::istreambuf_iterator<char>());

    return code;
}

std::vector<phylanx::execution_tree::primitive_argument_type>
read_arguments(std::vector<std::string> const& args, std::size_t first_index)
{
    std::vector<phylanx::execution_tree::primitive_argument_type> result;
    result.reserve(args.size() - 1);

    for (std::size_t i = first_index; i != args.size(); ++i)
    {
        auto arg = phylanx::ast::generate_ast(args[i]);
        result.emplace_back(
            phylanx::execution_tree::primitive_argument_type(std::move(arg)));
    }

    return result;
}

int handle_command_line(int argc, char* argv[], po::variables_map& vm)
{
    try
    {
        po::options_description cmdline_options(
            "Usage: interpreter <physl_script> [options] [arguments...]");
        cmdline_options.add_options()
            ("help,h", "print out program usage")
            ("code,c", po::value<std::string>(),
             "Execute the PhySL code given in argument")
            ("print,p", "Print the result of evaluation of the last "
                "PhySL expression encountered in the input")
            ("transform,t", po::value<std::string>(),
                "file to read transformation rules from")
        ;

        po::positional_options_description pd;
        pd.add("positional", -1);

        po::options_description positional_options;
        positional_options.add_options()
            ("positional", po::value<std::vector<std::string> >(),
             "positional options")
        ;

        po::options_description all_options;
        all_options.add(cmdline_options).add(positional_options);

        po::parsed_options opts(
            po::command_line_parser(argc, argv)
                .options(all_options)
                .positional(pd)
                .style(po::command_line_style::unix_style)
                .run()
            );

        po::store(opts, vm);

        if (vm.count("help"))
        {
            std::cout << cmdline_options << std::endl;
            return 1;
        }

        if (vm.count("positional") == 0)
        {
            std::cout << cmdline_options << std::endl;
            return -1;
        }
    }
    catch  (std::exception const& e)
    {
        std::cerr << "command line handling: exception caught: " << e.what()
                  << "\n";
        return -1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    po::variables_map vm;
    int cmdline_result = handle_command_line(argc, argv, vm);
    if (cmdline_result != 0)
    {
        return cmdline_result > 0 ? 0 : cmdline_result;
    }

    // Read the file containing the source code
    auto positional_args = vm["positional"].as<std::vector<std::string>>();

    std::string user_code;
    std::size_t first_index = 0;
    if (vm.count("code") != 0)
    {
        // execute code as given directly on the command line
        user_code = vm["code"].as<std::string>();
    }
    else
    {
        // interpret first arguument as the file name for the PhySL code
        user_code = read_user_code(positional_args[0]);
        first_index = 1;
    }

    // Collect the arguments for running the code
    auto const args = read_arguments(positional_args, first_index);

    // Compile the given code into AST
    auto ast = phylanx::ast::generate_ast(user_code);

    // Apply transformation rules to AST, if requested
    if (vm.count("transform") != 0)
    {
        std::string transform_rules =
            read_user_code(vm["transform"].as<std::string>());

        auto rules = phylanx::ast::generate_transform_rules(transform_rules);
        for (auto const& rule : rules)
        {
            ast = phylanx::ast::transform_ast(ast, rules);
        }
    }

    // Now compile AST into expression tree (into actual executable code)
    phylanx::execution_tree::compiler::function_list snippets;
    auto code = phylanx::execution_tree::compile(ast, snippets);

    // Evaluate user code using the read data
    auto result = code(args);

    // Print the result of the last PhySL expression, if requested
    if (vm.count("print") != 0)
    {
        std::cout << result << std::endl;
    }

    return 0;
}

