//   Copyright (c) 2018 Shahrzad Shirzad
//   Copyright (c) 2020 Hartmut Kaiser
//   Copyright (c) 2020 Bita Hasheminezhad
//   Copyright (c) 2020 Nanmiao Wu
//   Distributed under the Boost Software License, Version 1.0.0. (See accompanying
//   file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/errors/exception.hpp>
#include <hpx/errors/exception_list.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/program_options.hpp>


#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <utility>


#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
// This example uses part of the MovieLens from GroupLens Research.
// https://grouplens.org/datasets/movielens/
// Here, the MovieLens 20M Dataset is used.

// 718 user (rows) and 27278 movies (columns)
///////////////////////////////////////////////////////////////////////////////

char const* const read_r_code = R"(block(
    //
    // Read X-data from given CSV file
    //
    define(read_r, filepath, row_start, row_stop, col_start, col_stop,
        annotate_d(
            slice(file_read_csv(filepath),
                list(row_start, row_stop), list(col_start, col_stop)),
            "read_r",
            list("tile",
                list("rows", row_start, row_stop),
                list("columns", col_start, col_stop)
            )
        )
    ),
    read_r
))";

///////////////////////////////////////////////////////////////////////////////

//char const* const als_code = R"(
//    //
//    // Alternating Least squares algorithm
//    //
//    define(__als, ratings_row, regularization, num_factors,
//        iterations, alpha, enable_output,
//        block(
//            define(num_users, shape(ratings_row, 0)),
//
//            define(k, 0),
//
//            while(k < iterations,
//                block(
//                    if(enable_output,
//                            block(
//                                    cout("iteration ", k),
//                                    cout("num_users: ", num_users)
//                            )
//                    ),
//                    store(k, k + 1)
//                )
//            ),
//            list(ratings_row)
//        )
//    )
//    __als
//)";
//
//
////////////////////////////////////////////////////////////////////////////////
std::tuple<std::int64_t, std::int64_t> calculate_tiling_parameters(std::int64_t start,
    std::int64_t stop)
{
    std::uint32_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    std::uint32_t this_locality = hpx::get_locality_id();

    std::int64_t dim = stop - start;

    if (dim > num_localities)
    {
        std::size_t size = static_cast<std::size_t>(dim / num_localities);
        std::size_t remainder = dim % num_localities;

        if (this_locality < remainder)
        {
            size++;
        }

        if (remainder != 0 && this_locality >= remainder)
        {
            start =
                (size + 1) * remainder + size * (this_locality - remainder);
        }
        else
        {
            start = size * this_locality;
        }

        stop = start + size;
    }
    return std::make_tuple(start, stop);
}


////////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    if (vm.count("data_csv") == 0)
    {
        std::cerr << "Please specify '--data_csv=data-file'";
        return hpx::finalize();
    }

    // compile the given code
    using namespace phylanx::execution_tree;

    compiler::function_list snippets;
    auto const& code_read_r = compile("read_r", read_r_code, snippets);
    auto read_r = code_read_r.run();

    // handle command line arguments
    auto filename = vm["data_csv"].as<std::string>();

    auto row_start = vm["row_start"].as<std::int64_t>();
    auto row_stop = vm["row_stop"].as<std::int64_t>();
    auto col_start = vm["col_start"].as<std::int64_t>();
    auto col_stop = vm["col_stop"].as<std::int64_t>();

    auto regularization = vm["regularization"].as<double>();
    auto iterations = vm["iterations"].as<int64_t>();
    auto num_factors = vm["factors"].as<int64_t>();
    auto alpha = vm["alpha"].as<double>();
    //bool enable_output = vm.count("enable_output") != 0;
    bool enable_output = 1;


    primitive_argument_type ratings_row, ratings_column;
    std::int64_t user_row_start, user_row_stop, item_col_start, item_col_stop;
    // read the data from the file for user (rows)
    std::tie(user_row_start, user_row_stop) = calculate_tiling_parameters(row_start, row_stop);
//    ratings_row = read_r(filename, user_row_start, user_row_stop, col_start, col_stop);
    //std::cout << "row tiling: row_start is " << row_start << " and row_stop is "
    //          <<  row_stop << " col_start is "<< col_start << " and col_stop is "
    //          <<  col_stop << " user_row_start is "<< user_row_start << " and user_row_stop is "
    //          <<  user_row_stop << std::endl;

    // read the data from the file for movies (column)
    //std::tie(item_col_start, item_col_stop) = calculate_tiling_parameters(col_start, col_stop);
    //ratings_column = read_r(filename, row_start, row_stop, item_col_start, item_col_stop);
    //std::cout << "column tiling: row_start is " << row_start << " and row_stop is "
    //          <<  row_stop << " col_start is "<< col_start << " and col_stop is "
    //          <<  col_stop << " item_col_start is "<< item_col_start << " and item_col_stop is "
    //          <<  item_col_stop << std::endl;
    // evaluate ALS using the read data
//    auto const& code_als = compile("als", als_code, snippets);
//    auto als = code_als.run();
//
//    // time the execution
//    hpx::evaluate_active_counters(true, "start");
//    hpx::util::high_resolution_timer t;
//
//    auto result =
//        als(std::move(ratings_row), std::move(ratings_column), regularization,
//            num_factors, iterations, alpha, enable_output);
//
//    auto time_diff = t.elapsed();
//    std::cout << "end time is: " << time_diff << std::endl;
//
//
//    hpx::evaluate_active_counters(true, " finish");
//
//    auto result_r = extract_list_value(result);
//    auto it = result_r.begin();

    std::uint32_t num_localities = hpx::get_num_localities(hpx::launch::sync);
    std::cout << " total num_localities: " << num_localities << std::endl;

    if (hpx::get_locality_id() == 0)
    {
        //auto result_r = extract_list_value(result);
        //auto it_0 = result_r.begin();
        std::cout << " on loc 0: \n"
                  << "there are " << num_localities << " localities "
                  << " user_row_start: " << user_row_start
                  << " user_row_stop: " << user_row_stop
                  << std::endl;
    }
    if (hpx::get_locality_id() == 1)
    {

        //auto result_r = extract_list_value(result);
        //auto it_1 = result_r.begin();
        std::cout << " on loc 1: \n"
                  << "there are " << num_localities << " localities "
                  << " user_row_start: " << user_row_start
                  << " user_row_stop: " << user_row_stop
                  << std::endl;
    }


    //std::cout << "X: \n"
    //          << extract_numeric_value(*it++)
    //          << std::endl;
    //std::cout << "Calculated in: " << time_diff << " seconds" << std::endl;

    return hpx::finalize();
}
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    using hpx::program_options::options_description;
    using hpx::program_options::value;

    // command line handling
    options_description desc("usage: als [options]");
    desc.add_options()
        ("enable_output,e",
          "enable progress output (default: false)")
        ("iterations,i", value<std::int64_t>()->default_value(3),
         "number of iterations (default: 10.0)")
        ("factors,f", value<std::int64_t>()->default_value(10),
         "number of factors (default: 10)")
        ("alpha,a", value<double>()->default_value(40),
         "alpha (default: 40)")
        ("regularization,r", value<double>()->default_value(0.1),
         "regularization (default: 0.1)")
        ("data_csv", value<std::string>(), "file name for reading data")
        ("row_start", value<std::int64_t>()->default_value(0),
          "row_start (default: 0)")
        ("row_stop", value<std::int64_t>()->default_value(30),
         "row_stop (default: 718)")
        ("col_start", value<std::int64_t>()->default_value(0),
          "col_start (default: 0)")
        ("col_stop", value<std::int64_t>()->default_value(100),
         "col_stop (default: 27278)")
        ("tiling", value<std::string>()->default_value("horizontal"),
          "tiling method ('horizontal' (default) or 'vertical')")
    ;

    // make sure hpx_main is run on all localities
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(desc, argc, argv);
}