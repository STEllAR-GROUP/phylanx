//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0.0. (See accompanying
//   file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <hpx/hpx_init.hpp>
#include <phylanx/phylanx.hpp>

#include <iostream>

#include <hpx/program_options.hpp>
#include <blaze/Math.h>

#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
char const* const conv1d_input_code = R"(
    define(conv1d_input, input1, input2, input3,
    block(
	define(array, random_d(list(input1, input2, input3), nil, nil, "", "page")),
        array 
       )
    )
    conv1d_input
)";

///////////////////////////////////////////////////////////////////////////////
char const* const conv1d_test_code = R"(
    define(conv1d_test, input, filter1, filter2, filter3,
    block(
        define(kernel, random(list(filter1, filter2, filter3))),
        define(out, conv1d_d(input, kernel)),
        out
       )
    )
    conv1d_test
)";
///////////////////////////////////////////////////////////////////////////////
// Find the line/column position in the source code from a given iterator
// pointing into it.
//
std::pair<std::size_t, std::size_t> get_pos(std::string const& code,
    std::tuple<std::size_t, std::size_t, std::int64_t> const& tags)
{
    // Column might be given directly, in that case line is given as well
    if (std::get<2>(tags) != -1)
    {
        return std::make_pair(std::get<1>(tags), std::get<2>(tags));
    }

    // Otherwise the given value is the offset into the code
    std::size_t pos = std::get<1>(tags);
    std::size_t line = 1;
    std::size_t column = 1;

    for (std::int64_t i = 0; i != pos && i != code.size(); ++i)
    {
        if (code[i] == '\r' || code[i] == '\n')    // CR/LF
        {
            ++line;
            column = 1;
        }
        else
        {
            ++column;
        }
    }

    return std::make_pair(line, column);
}

///////////////////////////////////////////////////////////////////////////////
// Find offset into code as given by the tags argument
//
std::size_t get_offset(std::string const& code,
    std::tuple<std::size_t, std::size_t, std::size_t> const& tags)
{
    // Offset might be given directly
    if (std::get<2>(tags) == -1)
    {
        return std::get<1>(tags);
    }

    // Otherwise the given value is the line/column position in the code
    std::size_t offset = 0;
    std::size_t line = 1;
    std::size_t column = 0;

    for (std::int64_t i = 0; i != code.size(); ++i, ++offset)
    {
        if (code[i] == '\r' || code[i] == '\n')    // CR/LF
        {
            ++line;
            column = 0;
        }
        else
        {
            ++column;
        }

        if (std::get<1>(tags) == line && std::get<2>(tags) == column)
        {
            break;
        }
    }

    return offset;
}

// Extract the compile_id/tag pair from a given primitive instance name.
//
// The compile_id is a sequence number tracking invocations of the
// function phylanx::execution_tree::compile (needed to link back to the
// concrete source code compiled).
//
// The tag is an index into the array of iterators filled by
// phylanx::ast::generate_ast. It allows to find the iterator referring
// to the construct in the source code a particular primitive instance was
// created by.
//
std::tuple<std::size_t, std::size_t, std::size_t> extract_tags(
    std::string const& name)
{
    auto data = phylanx::execution_tree::compiler::parse_primitive_name(name);
    return std::make_tuple(data.compile_id, data.tag1, data.tag2);
}

// The symbolic names registered in AGAS that identify the created
// primitive instances have the following structure:
//
// /phylanx/<primitive>$<sequence-nr>[$<instance>]/<compile_id>$<tag1>[$<tag2>]
//
//  where:
//      <primitive>:   the name of primitive type representing the given
//                     node in the expression tree
//      <sequence-nr>: the sequence number of the corresponding instance
//                     of type <primitive>
//      <instance>:    (optional), some primitives have additional instance
//                     names, for instance references to function arguments
//                     have the name of the argument as their <instance>
//      <compile_id>:  the sequence number of the invocation of the
//                     function phylanx::execution_tree::compile
//      <tag1>:        if <tag2> == -1: the position inside the compiled code
//                     block where the referring to the point of usage of the
//                     primitive in the compiled source code
//                     if <tag2> != -1: the line number in the compiled code
//                     block where the referring to the point of usage of the
//                     primitive in the compiled source code
//      <tag2>:        (optional) if <tag2> != -1 or not given: the column
//                      offset in the given line (default: -1)
//
void print_instrumentation(char const* const name, int compile_id,
    std::string const& code,
    phylanx::execution_tree::compiler::function const& func,
    std::map<std::string, hpx::id_type> const& entries)
{
    std::cout << "Instrumentation information for function: " << name << "\n";

    for (auto const& e : entries)
    {
        // Extract compile_id and iterator index (tag) from the symbolic name
        auto tags = extract_tags(e.first);
        if (std::get<0>(tags) != compile_id)
            continue;

        // Find real position of given symbol in source code
        if (std::get<1>(tags) != std::size_t(-1))
        {
            auto pos = get_pos(code, tags);
            std::cout << e.first << ": " << name << "(" << pos.first << ", "
                      << pos.second << "): ";

            // Show the next (at max) 20 characters
            auto offset = get_offset(code, tags);
            auto end = code.begin() + offset;
            for (int i = 0; end != code.end() && i != 20; ++end, ++i)
            {
                if (*end == '\n' || *end == '\r')
                    break;
            }
            std::cout << std::string(code.begin() + offset, end) << " ...\n";
        }
        else
        {
            std::cout << e.first << "\n";
        }
    }

    std::cout << "\n";

    std::cout << "Tree information for function: " << name << "\n";
    std::cout << phylanx::execution_tree::newick_tree(
                     name, func.get_expression_topology())
              << "\n\n";

    std::cout << phylanx::execution_tree::dot_tree(
                     name, func.get_expression_topology())
              << "\n\n";
}

void print_performance_counter_data_csv(
    std::vector<std::string> const& existing_primitive_instances)
{
    std::cout << std::endl << "Primitive Performance Counter Data in CSV:";

    // CSV Header
    std::cout << "\nprimitive_instance,display_name,count,time,eval_direct\n";

    // Print performance data
    for (auto const& entry :
        phylanx::util::retrieve_counter_data(existing_primitive_instances))
    {
        std::cout << "\"" << entry.first << "\",\""
                  << phylanx::execution_tree::compiler::primitive_display_name(
                         entry.first)
                  << "\"";
        for (auto const& counter_value : entry.second)
        {
            std::cout << "," << counter_value;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& conv1d_input_compile = phylanx::execution_tree::compile(
        "conv1d_input", conv1d_input_code, snippets);
    auto conv1d_input = conv1d_input_compile.run();

    auto const& conv1d_test_compile = phylanx::execution_tree::compile(
        "conv1d_test", conv1d_test_code, snippets);

    // Enable collection of performance data for all existing primitives
    auto primitives = phylanx::util::enable_measurements();

    auto conv1d_test = conv1d_test_compile.run();

    // Print instrumentation information, if enabled
    if (vm.count("instrument") != 0)
    {
        auto entries = hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*");

        print_instrumentation(
            "conv1d_input", 0, conv1d_input_code, conv1d_input, entries);
        print_instrumentation(
            "conv1d_test", 0, conv1d_test_code, conv1d_test, entries);


    }

    // evaluate generated execution tree
    auto batch = vm["batch"].as<std::int64_t>();
    auto length = vm["length"].as<std::int64_t>();
    auto channel = vm["channel"].as<std::int64_t>();

    auto filter_length = vm["filter_length"].as<std::int64_t>();
    auto out_channels = vm["out_channels"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    // Read the data from the files

    auto input = conv1d_input(batch, length, channel);

    // Measure execution time
    hpx::util::high_resolution_timer t;

    // Evaluate ALS using the read data
    auto result =
        conv1d_test(input, filter_length, channel, out_channels);
    auto elapsed = t.elapsed();
    std::cout << "time: " << t.elapsed() << std::endl;

    auto output = phylanx::execution_tree::extract_numeric_value(result);
    auto output_t = output.tensor();
    std::cout << "["<< output_t.pages() << "," << output_t.rows() <<","<< output_t.columns() << "]" << std::endl;
    // Print performance counter data in CSV
    if (vm.count("instrument") != 0)
    {
        print_performance_counter_data_csv(primitives);
    }

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    if (vm.count("enable_output") != 0)
    {
        auto output = phylanx::execution_tree::extract_numeric_value(result);
        std::cout << output << std::endl;
    }
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    hpx::program_options::options_description desc("usage: conv1d [options]");
    desc.add_options()("enable_output,e",
        "enable progress output (default: false)")("instrument,i",
        "print instrumentation information (default: false)")("batch,b",
        hpx::program_options::value<std::int64_t>(),
        "batch")("length,l", hpx::program_options::value<std::int64_t>(),
        "length")("channel,c", hpx::program_options::value<std::int64_t>(),
        "channel")("filter_length,f", hpx::program_options::value<std::int64_t>(), "filter length")(
        "out_channels,o", hpx::program_options::value<std::int64_t>(), "kernel out");

    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};
    
    return hpx::init(desc, argc, argv, cfg);   
}

