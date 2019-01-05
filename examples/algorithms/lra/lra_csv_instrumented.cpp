//   Copyright (c) 2017-2018 Hartmut Kaiser
//   Copyright (c) 2018 Parsa Amini
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <utility>

#include <boost/program_options.hpp>
#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////////
// This example uses part of the breast cancer dataset from UCI Machine Learning
// Repository:
//     https://archive.ics.uci.edu/ml/datasets/Breast+Cancer+Wisconsin+(Diagnostic)
//
// A copy of the full dataset in CSV format (breast_cancer.csv), obtained from
// scikit-learn datasets, is provided in the same folder as this example.
//
// The layout of the data in the provided CSV file used by the example
// is as follows:
// 30 features per line followed by the classification
// 569 lines of data
//
// This example also demonstrates how the generated primitives can be introspected
// and linked back to the source code.
//
/////////////////////////////////////////////////////////////////////////////////

std::string const read_x_code = R"(block(
    //
    // Read X-data from given CSV file
    //
    define(read_x, filepath, row_start, row_stop, col_start, col_stop,
        slice(file_read_csv(filepath),
              make_list(row_start, row_stop),
              make_list(col_start, col_stop))
    ),
    read_x
))";

std::string const read_y_code = R"(block(
    //
    // Read Y-data from given CSV file
    //
    define(read_y, filepath, row_start, row_stop, col_stop,
        slice(file_read_csv(filepath), make_list(row_start, row_stop), col_stop)
    ),
    read_y
))";

///////////////////////////////////////////////////////////////////////////////
std::string const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [N, M]
    //   y: [N]
    //
    define(lra_explicit, x, y, alpha, iterations, enable_output,
        block(
            define(weights, constant(0.0, shape(x, 1))),            // weights: [M]
            define(transx, transpose(x)),                           // transx:  [M, N]
            define(pred, constant(0.0, shape(x, 0))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    // exp(-dot(x, weights)): [N], pred: [N]
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),
                    store(weights, weights - (alpha * dot(transx, pred - y))),
                    store(step, step + 1)
                )
            ),
            weights
        )
    ),
    lra_explicit
))";

std::string const lra_code_direct = R"(block(
    //
    // Logistic regression analysis algorithm (direct implementation)
    //
    //   x: [N, M]
    //   y: [N]
    //
    define(lra_direct, x, y, alpha, iterations, enable_output,
        lra(x, y, alpha, iterations, enable_output)
    ),
    lra_direct
))";

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
void print_instrumentation(
    char const* const name, int compile_id, std::string const& code,
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
            std::cout << std::string(code.begin() + offset, end)
                      << " ...\n";
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

///////////////////////////////////////////////////////////////////////////////
int hpx_main(boost::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // Compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto const& code_read_x = phylanx::execution_tree::compile(
        "read_x", phylanx::ast::generate_ast(read_x_code), snippets);

    auto const& code_read_y = phylanx::execution_tree::compile(
        "read_y", phylanx::ast::generate_ast(read_y_code), snippets);

    auto const& code_lra = phylanx::execution_tree::compile(
        vm.count("direct") != 0 ? "lra_direct" : "lra",
        vm.count("direct") != 0 ? lra_code_direct : lra_code, snippets);

    // Enable collection of performance data for all existing primitives
    auto primitives = phylanx::util::enable_measurements();

    auto read_x = code_read_x.run();
    auto read_y = code_read_y.run();
    auto lra = code_lra.run();

    // Print instrumentation information, if enabled
    if (vm.count("instrument") != 0)
    {
        auto entries =
            hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*");

        print_instrumentation("read_x", 0, read_x_code, read_x, entries);
        print_instrumentation("read_y", 1, read_y_code, read_y, entries);
        print_instrumentation("lra", 2, lra_code, lra, entries);
    }

    auto row_start = vm["row_start"].as<std::int64_t>();
    auto col_start = vm["col_start"].as<std::int64_t>();
    auto row_stop = vm["row_stop"].as<std::int64_t>();
    auto col_stop = vm["col_stop"].as<std::int64_t>();

    // Read the data from the files
    auto x = read_x(vm["data_csv"].as<std::string>(), row_start, row_stop,
        col_start, col_stop);
    auto y =
        read_y(vm["data_csv"].as<std::string>(), row_start, row_stop, col_stop);

    // Remaining command line options
    auto alpha = vm["alpha"].as<double>();
    auto iterations = vm["num_iterations"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    // Measure execution time
    hpx::util::high_resolution_timer t;

    // Evaluate LRA using the read data
    auto result =
        lra(std::move(x), std::move(y), alpha, iterations, enable_output);

    auto elapsed = t.elapsed();

    // Print performance counter data in CSV
    if (vm.count("instrument") != 0)
    {
        print_performance_counter_data_csv(primitives);
    }

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result) << "\n"
              << "Calculated in: " << elapsed << " seconds\n";

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // Command line handling
    boost::program_options::options_description desc("usage: lra [options]");
    desc.add_options()
        ("enable_output,e", "enable progress output (default: false)")
        ("instrument,i", "print instrumentation information (default: false)")
        ("direct,d", "use direct implementation of LRA (default: false)")
        ("num_iterations,n",
            boost::program_options::value<std::int64_t>()->default_value(750),
            "number of iterations (default: 750)")
        ("alpha,a",
            boost::program_options::value<double>()->default_value(1e-5),
            "alpha (default: 1e-5)")
        ("data_csv",
            boost::program_options::value<std::string>(),
            "file name for reading data")
        ("row_start",
            boost::program_options::value<std::int64_t>()->default_value(0),
            "row_start (default: 0)")
        ("col_start",
            boost::program_options::value<std::int64_t>()->default_value(0),
            "col_start (default: 0)")
        ("row_stop",
            boost::program_options::value<std::int64_t>()->default_value(569),
            "row_stop (default: 569)")
        ("col_stop",
            boost::program_options::value<std::int64_t>()->default_value(30),
            "col_stop (default: 30)")
        ;

    return hpx::init(desc, argc, argv);
}
