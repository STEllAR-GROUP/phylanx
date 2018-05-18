//   Copyright (c) 2018 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0.0. (See accompanying
//   file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>

#include <blaze/Math.h>
#include <boost/program_options.hpp>

#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
char const* const read_x_code = R"(block(
    //
    // Read input-data from given CSV file
    //
    define(read_x, filepath, row_start, row_stop, col_start, col_stop,
        slice(file_read_csv(filepath), make_list(row_start , row_stop),
              make_list(col_start , col_stop))
    ),
    read_x
))";


char const* const als_explicit = R"(block(
    //
    // Alternating Least squares algorithm (ALS)
    //
    //
    //
    define(als_explicit, ratings, regularization, num_factors, iterations, alpha,
        enable_output,
        block(
            define(num_users, shape(ratings, 0)),
            define(num_items, shape(ratings, 1)),
            define(conf, alpha * ratings),

            define(conf_u, constant(0.0, make_list(num_items))),
            define(conf_i, constant(0.0,make_list(num_users))),

            define(c_u, constant(0.0, make_list(num_items, num_items))),
            define(c_i, constant(0.0, make_list(num_users, num_users))),
            define(p_u, constant(0.0, make_list(num_items))),
            define(p_i, constant(0.0, make_list(num_users))),

            set_seed(0),
            define(X, random(make_list(num_users, num_factors))),
            define(Y, random(make_list(num_items, num_factors))),
            define(I_f, identity(num_factors)),
            define(I_i, identity(num_items)),
            define(I_u, identity(num_users)),
            define(k, 0),
            define(i, 0),
            define(u, 0),

            define(XtX, constant(0.0, make_list(num_factors, num_factors))),
            define(YtY, constant(0.0, make_list(num_factors, num_factors))),
            define(A, constant(0.0, make_list(num_factors, num_factors))),
            define(b, constant(0.0, make_list(num_factors))),

            while(k < iterations,
                block(
                    store(YtY, dot(transpose(Y), Y) + regularization*I_f),
                    store(XtX, dot(transpose(X), X) + regularization*I_f),

                    while(u < num_users,
                        block(
                                if(enable_output,
                                        block(
                                                cout("iteration ",k),
                                                cout("X: ",X),
                                                cout("Y: ",Y)
                                        )
                                ),
                            store(conf_u, slice_row(conf, u)),
                            store(c_u, diag(conf_u)),
                            store(p_u, __ne(conf_u,0.0,true)),
                            store(A, dot(dot(transpose(Y),c_u),Y)+ YtY),
                            store(b, dot(dot(transpose(Y),(c_u + I_i)),transpose(p_u))),
                            set_row(X,u,u+1,1,dot(inverse(A),b)),
                            store(u, u+1)
                        )
                    ),
                    store(u, 0),
                    while(i < num_items,
                        block(
                            store(conf_i, slice_column(conf, i)),
                            store(c_i, diag(conf_i)),
                            store(p_i, __ne(conf_i,0.0,true)),
                            store(A, dot(dot(transpose(X),c_i),X) + XtX),
                            store(b, dot(dot(transpose(X),(c_i + I_u)),transpose(p_i))),
                            set_row(Y,i,i+1,1,dot(inverse(A),b)),
                            store(i, i+1)
                        )
                    ),
                    store(i, 0),
                    store(k,k+1)
                )
            ),
            make_list(X, Y)
        )
    ),
    als_explicit
))";

std::string const als_direct = R"(block(
    //
    // Alternating Least squares algorithm (ALS) (direct implementation)
    //
    //
    //
    //
    define(als_direct, ratings, regularization, num_factors, iterations, alpha,
        enable_output, als(ratings, regularization, num_factors, iterations, alpha,
        enable_output)
    ),
    als_direct
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

void print_performance_counter_data_csv()
{
    std::cout << std::endl << "Primitive Performance Counter Data in CSV:";

    // CSV Header
    std::cout << "\nprimitive_instance,display_name,count,time,eval_direct\n";

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;

    // Retrieve all primitive instances
    for (auto const& entry :
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*"))
    {
        existing_primitive_instances.push_back(entry.first);
    }

    // Print performance data
    for (auto const& entry :
        phylanx::util::retrieve_counter_data(existing_primitive_instances,
            std::vector<std::string>{"count/eval", "time/eval", "eval_direct"},
            hpx::find_here()))
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

int hpx_main(boost::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto read_x =
        phylanx::execution_tree::compile("read_x", read_x_code, snippets);
    auto als = phylanx::execution_tree::compile(
        vm.count("direct") != 0 ? als_direct : als_explicit, snippets);

    // Print instrumentation information, if enabled
    if (vm.count("instrument") != 0)
    {
        auto entries = hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*");

        print_instrumentation("als", 0,
            vm.count("direct") != 0 ? als_direct : als_explicit, als, entries);
    }

    // evaluate generated execution tree
    auto row_start = static_cast<int64_t>(0);
    auto col_start = static_cast<int64_t>(0);
    auto row_stop = vm["row_stop"].as<std::int64_t>();
    auto col_stop = vm["col_stop"].as<std::int64_t>();

    auto regularization = vm["regularization"].as<double>();
    auto iterations = vm["iterations"].as<int64_t>();
    auto num_factors = vm["factors"].as<int64_t>();
    auto alpha = vm["alpha"].as<double>();
    auto filepath = vm["data_csv"].as<std::string>();

    bool enable_output = vm.count("enable_output") != 0;

    // Read the data from the files
    auto ratings = read_x(filepath, row_start, row_stop, col_start, col_stop);

    // Measure execution time
    hpx::util::high_resolution_timer t;

    // Evaluate ALS using the read data
    auto result =
        als(ratings, regularization, num_factors, iterations, alpha, enable_output);
    auto elapsed = t.elapsed();

    // Print performance counter data in CSV
    if (vm.count("instrument") != 0)
    {
        print_performance_counter_data_csv();
    }

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    auto result_r = phylanx::execution_tree::extract_list_value(result);
    auto it = result_r.begin();
    std::cout << "X: \n"
              << phylanx::execution_tree::extract_numeric_value(*it++)
              << "\nY: \n"
              << phylanx::execution_tree::extract_numeric_value(*it)
              << std::endl;
    std::cout << "time: " << t.elapsed() << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc("usage: als [options]");
    desc.add_options()("enable_output,e",
        "enable progress output (default: false)")("instrument,i",
        "print instrumentation information (default: false)")("direct,d",
        "use direct implementation of ALS (default: false)")("iterations,n",
        boost::program_options::value<std::int64_t>()->default_value(3),
        "number of iterations (default: 10.0)")("factors,f",
        boost::program_options::value<std::int64_t>()->default_value(10),
        "number of factors (default: 10)")("alpha,a",
        boost::program_options::value<double>()->default_value(40),
        "alpha (default: 40)")("regularization,r",
        boost::program_options::value<double>()->default_value(0.1),
        "regularization (default: 0.1)")("data_csv",
        boost::program_options::value<std::string>(),
        "file name for reading data")("row_stop",
        boost::program_options::value<std::int64_t>()->default_value(10),
        "row_stop (default: 10)")("col_stop",
        boost::program_options::value<std::int64_t>()->default_value(100),
        "col_stop (default: 100)");
    return hpx::init(desc, argc, argv);
}
