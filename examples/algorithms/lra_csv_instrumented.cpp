//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/agas.hpp>

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
// Repository. https://goo.gl/U2Uwz
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
        slice(file_read_csv(filepath), row_start, row_stop, col_start, col_stop)
    ),
    read_x
))";

std::string const read_y_code = R"(block(
    //
    // Read Y-data from given CSV file
    //
    define(read_y, filepath, row_start, row_stop,
        slice(file_read_csv(filepath), row_start, row_stop, -1, 0)
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
    define(lra, x, y, alpha, iterations, enable_output,
        block(
            define(weights, constant(0.0, shape(x, 1))),                // weights: [M]
            define(transx, transpose(x)),                               // transx:  [M, N]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < iterations,
                block(
                    if(enable_output, cout("step: ", step, ", ", weights)),
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),  // exp(-dot(x, weights)): [N], pred: [N]
                    store(error, pred - y),                            // error: [N]
                    store(gradient, dot(transx, error)),               // gradient: [M]
                    parallel_block(
                        store(weights, weights - (alpha * gradient)),
                        store(step, step + 1)
                    )
                )
            ),
            weights
        )
    ),
    lra
))";

///////////////////////////////////////////////////////////////////////////////
// Find the line/column position in the source code from a given iterator
// pointing into it.
//
std::pair<std::size_t, std::size_t> get_pos(std::string const& code,
    std::string::const_iterator pos)
{
    std::size_t line = 1;
    std::size_t column = 1;

    for (std::string::const_iterator i = code.begin();
         i != code.end() && i != pos; ++i)
    {
        if (*i == '\r' || *i == '\n')    // CR/LF
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
std::pair<std::size_t, std::int64_t> extract_tags(std::string const& name)
{
    std::size_t compile_id = 0;
    std::int64_t tag = 0;

    auto p = name.find_last_of("#");
    if (p != std::string::npos)
    {
        char* end = nullptr;
        tag = std::strtoll(name.c_str() + p + 1, &end, 10);

        p = name.find_last_of("/", p);
        if (p != std::string::npos)
        {
            compile_id = std::strtoll(name.c_str() + p + 1, &end, 10);
        }
    }

    return std::make_pair(compile_id, tag);
}

// The symbolic names registered in AGAS that identify the created
// primitive instances have the following structure:
//
//      /phylanx/<primitive>/<compile_id>#<tag>
//
// where:
//      <primitive>:  the name of primitive type representing the given
//                    node in the expression tree
//      <compile_id>: the sequence number of the invocation of the
//                    function phylanx::execution_tree::compile
//      <tag>:        the index into the vector of iterators, where the
//                    iterator refers to the point of usage of the
//                    primitive in the compiled source code
//
void print_instrumentation(char const* const name, int compile_id,
    std::string const& code,
    std::vector<std::string::const_iterator> const& iterators,
    std::map<std::string, hpx::id_type> const& entries)
{
    std::cout << "Instrumentation information for function: " << name << "\n";

    for (auto const& e : entries)
    {
        // extract compile_id and iterator index (tag) from the symbolic name
        auto tags = extract_tags(e.first);
        if (tags.first != compile_id)
            continue;

        // find real position of given symbol in source code
        if (tags.second >= 0 &&
            tags.second < static_cast<std::int64_t>(iterators.size()))
        {
            auto pos = get_pos(code, iterators[tags.second]);
            std::cout << e.first << ": " << name << "(" << pos.first << ", "
                      << pos.second << "): ";

            // show the next (at max) 20 characters
            auto end = iterators[tags.second];
            for (int i = 0; end != code.end() && i != 20; ++end, ++i)
            {
                if (*end == '\n' || *end == '\r')
                    break;
            }
            std::cout << std::string(iterators[tags.second], end) << " ...\n";
        }
        else
        {
            std::cout << e.first << "\n";
        }
    }

    std::cout << "\n";
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(boost::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // compile the given code
    std::vector<std::string::const_iterator> iterators;
    phylanx::execution_tree::compiler::function_list snippets;

    auto read_x = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(read_x_code, iterators), snippets);
    auto read_y = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(read_y_code, iterators), snippets);
    auto lra = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(lra_code, iterators), snippets);

    // print instrumentation information, if enabled
    if (vm.count("instrument") != 0)
    {
        auto entries =
            hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*");

        print_instrumentation("read_x", 0, read_x_code, iterators, entries);
        print_instrumentation("read_y", 1, read_y_code, iterators, entries);
        print_instrumentation("lra", 2, lra_code, iterators, entries);
    }

    auto row_start = vm["row_start"].as<std::int64_t>();
    auto col_start = vm["col_start"].as<std::int64_t>();
    auto row_stop = vm["row_stop"].as<std::int64_t>();
    auto col_stop = vm["col_stop"].as<std::int64_t>();

    // read the data from the files
    auto x = read_x(vm["data_csv"].as<std::string>(), row_start, row_stop,
        col_start, col_stop);
    auto y = read_y(vm["data_csv"].as<std::string>(), row_start, row_stop);

    // remaining command line options
    auto alpha = vm["alpha"].as<double>();
    auto iterations = vm["num_iterations"].as<std::int64_t>();
    bool enable_output = vm.count("enable_output") != 0;

    phylanx::ir::reset_node_statistics();

    // time execution
    hpx::util::high_resolution_timer t;

    // evaluate LRA using the read data
    auto result =
        lra(std::move(x), std::move(y), alpha, iterations, enable_output);

    auto elapsed = t.elapsed();

    std::cout << "Result: \n"
              << phylanx::execution_tree::extract_numeric_value(result) << "\n"
              << "Calculated in: " << elapsed << " seconds\n";

    phylanx::ir::print_node_statistics();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: lra [options]");
    desc.add_options()
        ("enable_output,e", "enable progress output (default: false)")
        ("instrument,i", "print instrumentation information (default: false)")
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
