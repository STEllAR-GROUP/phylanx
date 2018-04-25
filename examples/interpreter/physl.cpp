// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

std::string read_user_code(std::string const& path)
{
    std::ifstream code_stream(path);
    if (!code_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "read_user_code",
            "Failed to open the specified file: " + path);
    }

    // Read the file
    std::ostringstream str_stream;
    if (!(str_stream << code_stream.rdbuf()))
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "read_user_code",
            "Failed to read code from the specified file: " + path);
    }

    return str_stream.str();
}

void dump_ast(std::vector<phylanx::ast::expression> ast, std::string path)
{
    std::ofstream ast_stream(path, std::ios::binary);
    if (!ast_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "dump_ast",
            "Failed to open the specified file: " + path);
    }

    // Serialize the AST into a byte array
    std::vector<char> bytes = phylanx::util::serialize(ast);

    // char is 1 byte
    ast_stream.write(bytes.data(), bytes.size());
}

std::vector<phylanx::ast::expression> load_ast(std::string path)
{
    std::ifstream str_stream(path, std::ios::binary);
    auto const start_pos = str_stream.tellg();
    if (std::streamsize(-1) == start_pos)
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "load_ast",
            "Failed to open the specified file: " + path);
    }
    // Find out where the end of the file is
    if (!str_stream.seekg(0, std::ios::end))
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "load_ast",
            "Failed to find the end of the specified file: " + path);
    }

    auto const char_count = str_stream.tellg();

    if (!str_stream.seekg(start_pos))
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "load_ast",
            "Failed to perform seek() on the specified file: " + path);
    }

    // Allocate all the memory needed to load the AST upfront
    std::vector<char> bytes(char_count);

    if (0 != bytes.size())
    {
        if (!str_stream.read(bytes.data(), bytes.size()))
        {
            HPX_THROW_EXCEPTION(hpx::filesystem_error,
                "load_ast",
                "Failed to read from the specified file: " + path);
        }
    }

    return phylanx::util::unserialize<std::vector<phylanx::ast::expression>>(
        bytes);
}

std::vector<phylanx::execution_tree::primitive_argument_type>
read_arguments(std::vector<std::string> const& args, std::size_t first_index)
{
    std::vector<phylanx::execution_tree::primitive_argument_type> result;

    if (args.size() > 1)
    {
        result.reserve(args.size() - 1);
    }

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
    std::cout << "primitive_instance,display_name,count,time,eval_direct\n";

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
        "count/eval", "time/eval", "eval_direct"
    };

    for (auto const& entry : phylanx::util::retrieve_counter_data(
             existing_primitive_instances, counter_names))
    {
        std::cout << "\"" << entry.first << "\",\""
                  << phylanx::execution_tree::compiler::primitive_display_name(
                         entry.first)
                  << "\"";
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
            ("dump-ast,d", po::value<std::string>(),
                "file to dump AST to")
            ("load-ast,l", po::value<std::string>(),
                "file to dump AST to")
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
    std::vector<std::string> positional_args;
    if (vm.count("positional") != 0)
    {
        positional_args = vm["positional"].as<std::vector<std::string>>();
    }

    std::string code_source_name;
    std::string user_code;
    std::size_t first_index = 0;
    std::vector<phylanx::ast::expression> ast;

    if (vm.count("load-ast") != 0)
    {
        auto path = vm["load-ast"].as<std::string>();
        ast = load_ast(path);
        code_source_name = "<ast_dump>";
    }
    else
    {
        if (vm.count("code") != 0)
        {
            // Execute code as given directly on the command line
            user_code = vm["code"].as<std::string>();
            code_source_name = "<command_line>";
        }
        else if (positional_args.size() > 0)
        {
            // Interpret first argument as the file name for the PhySL code
            user_code = read_user_code(positional_args[0]);
            code_source_name = fs::path(positional_args[0]).filename().string();
            first_index = 1;
        }
        else
        {
            std::cout << "No code was provided.\n";

            return -1;
        }

        try
        {
            // Compile the given code into AST
            ast = phylanx::ast::generate_ast(user_code);

            // Apply transformation rules to AST, if requested
            if (vm.count("transform") != 0)
            {
                std::string const transform_rules =
                    read_user_code(vm["transform"].as<std::string>());

                ast = phylanx::ast::transform_ast(
                    ast, phylanx::ast::generate_transform_rules(transform_rules));
            }

            // Dump the AST to a file, if requested
            if (vm.count("dump-ast") != 0)
            {
                dump_ast(ast, vm["dump-ast"].as<std::string>());
            }
        }
        catch (std::exception const& e)
        {
            std::cout << "physl: exception caught:\n" << e.what() << "\n";
            return -1;
        }
    }

    // Collect the arguments for running the code
    auto const args = read_arguments(positional_args, first_index);

    try
    {
        // Now compile AST into expression tree (into actual executable code)
        phylanx::execution_tree::compiler::function_list snippets;
        phylanx::execution_tree::compiler::environment env =
            phylanx::execution_tree::compiler::default_environment();

        phylanx::execution_tree::define_variable(code_source_name,
            "sys_argv/0$0", snippets, env,
            phylanx::execution_tree::primitive_argument_type{args});
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
            auto topology = snippets.snippets_.back().get_expression_topology();

            std::cout << "\n"
                << phylanx::execution_tree::dot_tree(code_source_name, topology)
                << "\n";

            std::cout << "\n"
                << phylanx::execution_tree::newick_tree(code_source_name, topology)
                << "\n\n";

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

