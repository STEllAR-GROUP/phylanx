//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
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

///////////////////////////////////////////////////////////////////////////////
void print_performance_counter_data_csv()
{
    // CSV Header
    std::cout << "primitive_instance,count,time,direct_count,direct_time\n";

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;

    // Retrieve all primitive instances
    for (auto const& entry :
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*"))
    {
        existing_primitive_instances.push_back(entry.first);
    }

    // Print performance data
    std::vector<std::string> const counter_names{
        "count/eval", "time/eval", "count/eval_direct", "time/eval_direct"};

    for (auto const& entry : phylanx::util::retrieve_counter_data(
             existing_primitive_instances, counter_names))
    {
        std::cout << "\"" << entry.first << "\"";
        for (auto const& counter_value : entry.second)
        {
            std::cout << "," << counter_value;
        }
        std::cout << "\n";
    }

    std::cout << "\n";
}

///////////////////////////////////////////////////////////////////////////////
int handle_command_line(int argc, char* argv[], po::variables_map& vm)
{
    try
    {
        po::options_description cmdline_options(
            "Usage: physl <physl_script> [options] [arguments...]");
        cmdline_options.add_options()
            ("help,h", "print out program usage")
            ("code,c", po::value<std::string>(),
             "Execute the PhySL code given in argument")
            ("print,p", "Print the result of evaluation of the last "
                "PhySL expression encountered in the input")
            ("performance", "Print the topology of the created execution "
                "tree and the corresponding performance counter results")
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

        po::parsed_options const opts(
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
        std::cerr << "physl: command line handling: exception caught: "
                  << e.what() << "\n";
        return -1;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    po::variables_map vm;
    int const cmdline_result = handle_command_line(argc, argv, vm);
    if (cmdline_result != 0)
    {
        return cmdline_result > 0 ? 0 : cmdline_result;
    }

    // Read the file containing the source code
    auto positional_args = vm["positional"].as<std::vector<std::string>>();

    std::string code_source_name;
    std::string user_code;
    std::size_t first_index = 0;
    if (vm.count("code") != 0)
    {
        // Execute code as given directly on the command line
        user_code = vm["code"].as<std::string>();
        code_source_name = "<command_line>";
    }
    else
    {
        // Interpret first argument as the file name for the PhySL code
        user_code = read_user_code(positional_args[0]);
        code_source_name = fs::path(positional_args[0]).filename().string();
        first_index = 1;
    }

    // Collect the arguments for running the code
    auto const args = read_arguments(positional_args, first_index);

    try
    {
        // Compile the given code into AST
        auto ast = phylanx::ast::generate_ast(user_code);

        // Apply transformation rules to AST, if requested
        if (vm.count("transform") != 0)
        {
            std::string const transform_rules =
                read_user_code(vm["transform"].as<std::string>());

            ast = phylanx::ast::transform_ast(
                ast, phylanx::ast::generate_transform_rules(transform_rules));
        }

        // Now compile AST into expression tree (into actual executable code)
        phylanx::execution_tree::compiler::function_list snippets;
        phylanx::execution_tree::compiler::environment env =
            phylanx::execution_tree::compiler::default_environment();

        phylanx::execution_tree::define_variable(
            code_source_name, "sys_argv/0$0", snippets, env, args);
        auto const code = phylanx::execution_tree::compile(
            code_source_name, ast, snippets, env);

        // Re-init all performance counters to guarantee correct measurement
        // results if those are requested on the command line.
        hpx::reinit_active_counters();

        // Evaluate user code using the read data
        auto const result = code();

        // Print the result of the last PhySL expression, if requested
        if (vm.count("print") != 0)
        {
            std::cout << result << "\n";
        }

        // Print auxiliary information at exit: topology of the execution tree
        // and the associate performance counter data
        if (vm.count("performance") != 0)
        {
            std::cout << "\n"
                << phylanx::execution_tree::dot_tree(code_source_name,
                       snippets.snippets_.back().get_expression_topology())
                << "\n";
            print_performance_counter_data_csv();
        }
    }
    catch (std::exception const& e)
    {
        std::cout << "physl: exception caught:\n" << e.what() << "\n";
        return -1;
    }

    return 0;
}

