//   Copyright (c) 2018 Shahrzad Shirzad
//   Copyright (c) 2020 Hartmut Kaiser
//   Copyright (c) 2020 Bita Hasheminezhad
//   Copyright (c) 2020 Nanmiao Wu
//   Distributed under the Boost Software License, Version 1.0.0. (See accompanying
//   file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>


#include <hpx/hpx_init.hpp>
#include <hpx/program_options.hpp>


#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
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

char const* const als_code = R"(
    //
    // Alternating Least squares algorithm
    //
    define(__als, ratings_user, ratings_movie, regularization, num_factors,
        iterations, alpha, enable_output, row_start, row_stop, col_start, col_stop,
        block(
            define(num_users_u, shape(ratings_user, 0)),
            define(num_items_u, shape(ratings_user, 1)),
            define(num_users_i, shape(ratings_movie, 0)),
            define(num_items_i, shape(ratings_movie, 1)),

            define(confu, alpha * ratings_user),
            define(confi, alpha * ratings_movie),

            define(conf_u, constant_d(0.0, list(num_items_u))),
            define(conf_i, constant_d(0.0, list(num_users_i))),

            define(c_u, constant_d(0.0, list(num_items_u, num_items_u))),
            define(c_i, constant_d(0.0, list(num_users_i, num_users_i))),
            define(p_u, constant_d(0.0, list(num_items_u))),
            define(p_i, constant_d(0.0, list(num_users_i))),

            set_seed(0),
            define(X, random_d(list(num_users_u, num_factors))),
            define(Y, random_d(list(num_items_i, num_factors))),
            define(I_f, identity(num_factors)),
            define(I_i, identity_d(num_items_u)),
            define(I_u, identity_d(num_users_i)),
            define(k, 0),
            define(i, 0),
            define(u, 0),

            define(XtX, constant(0.0, make_list(num_factors, num_factors))),
            define(YtY, constant(0.0, make_list(num_factors, num_factors))),
            define(A, constant(0.0, make_list(num_factors, num_factors))),
            define(b, constant(0.0, make_list(num_factors))),

            while(k < iterations,
                block(
                    if(enable_output,
                            block(
                                    cout("iteration ", k),
                                    cout("X: ", X),
                                    cout("Y: ", Y)
                            )
                    ),
                    store(Y, all_gather_d(Y)),
                    store(YtY, dot(transpose(Y), Y) + regularization * I_f),

                    while(u < num_users_u,
                        block(
                            store(conf_u, slice_row(confu, u)),
                            store(c_u, diag_d(annotate_d(conf_u,
                                list("tile", list("rows", row_start, row_stop)))),
                            store(p_u, __ne(conf_u,0.0,true)),
                            store(A, dot(dot(transpose(Y), c_u), Y)+ YtY),
                            store(b, dot(dot(transpose_d(Y), (c_u + I_i)), transpose(p_u))),
                            store(slice(X, list(u, u + 1, 1),nil), dot(inverse(A), b)),
                            store(u, u + 1)
                        )
                    ),
                    store(u, 0),

                    store(X, all_gather_d(X)),
                    store(XtX, dot_d(transpose_d(X), X) + regularization * I_f),
                    while(i < num_items_i,
                        block(
                            store(conf_i, slice_column(confi, i)),
                            store(c_i, diag(conf_i)),
                            store(p_i, __ne(conf_i, 0.0, true)),
                            store(A, dot_d(dot_d(transpose_d(X), c_i),X) + XtX),
                            store(b, dot_d(dot_d(transpose_d(X), (c_i + I_u)), transpose(p_i))),
                            store(slice(Y, list(i, i + 1, 1),nil), dot_d(inverse(A), b)),
                            store(i, i + 1)
                        )
                    ),
                    store(i, 0),
                    store(k, k + 1)
                )
            ),
            list(X, Y)
        )
    )
    __als
)";


////////////////////////////////////////////////////////////////////////////////
void calculate_tiling_parameters(std::int64_t& start,
    std::int64_t& stop)
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
}

//void calculate_horizontal_tiling_parameters(std::int64_t& row_start,
//    std::int64_t& row_stop)
//{
//    std::uint32_t num_localities = hpx::get_num_localities(hpx::launch::sync);
//    std::uint32_t this_locality = hpx::get_locality_id();
//
//    std::int64_t rows = row_stop - row_start;
//
//    if (rows > num_localities)
//    {
//        rows = (rows + num_localities) / num_localities;
//        row_start += this_locality * rows;
//        row_stop = (std::min)(row_stop, row_start + rows);
//    }
//}
//
//void calculate_vertical_tiling_parameters(
//    std::int64_t& col_start, std::int64_t& col_stop)
//{
//    std::uint32_t num_localities = hpx::get_num_localities(hpx::launch::sync);
//    std::uint32_t this_locality = hpx::get_locality_id();
//
//    std::int64_t columns = col_stop - col_start;
//
//    if (columns > num_localities)
//    {
//        columns = (columns + num_localities) / num_localities - 1;
//        col_start = col_start + this_locality * columns;
//        col_stop = col_start + columns;
//    }
//}
//
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
    bool enable_output = vm.count("enable_output") != 0;

    // calculate tiling parameters for this locality, read data
    primitive_argument_type ratings;

    //if (vm["tiling"].as<std::string>() == "horizontal")
    //{
    //    calculate_tiling_parameters(row_start, row_stop);
//
    //    // read the data from the file 
    //    ratings = read_r(filename, row_start, row_stop, col_start, col_stop);
//
    //}
    //else
    //{
    //    calculate_tiling_parameters(col_start, col_stop);
//
    //    // read the X-data from the file
    //    ratings = read_r(filename, row_start, row_stop, col_start, col_stop);
    //}

    // read the data from the file for user (rows)
    calculate_tiling_parameters(row_start, row_stop);
    ratings_user = read_r(filename, row_start, row_stop, col_start, col_stop);

    // read the X-data from the file for movies (column)
    calculate_tiling_parameters(col_start, col_stop);
    ratings_movie = read_r(filename, row_start, row_stop, col_start, col_stop);

    // evaluate ALS using the read data
    auto const& code_als = compile("als", als_code, snippets);
    auto als = code_als.run();

    // time the execution
    hpx::evaluate_active_counters(true, "start");
    hpx::util::high_resolution_timer t;

    auto result =
        als(std::move(ratings_user), std::move(ratings_movie), regularization,
            num_factors, iterations, alpha, enable_output);

    auto time_diff = t.elapsed();
    

    hpx::evaluate_active_counters(true, " finish");

    auto result_r = extract_list_value(result);
    auto it = result_r.begin();

    std::cout << "X: \n"
              << extract_numeric_value(*it++)
              << "\nY: \n"
              << extract_numeric_value(*it)
              << std::endl;
    std::cout << "Calculated in: " << time_diff << " seconds" << std::endl;

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
        ("row_stop", value<std::int64_t>()->default_value(718),
         "row_stop (default: 718)")
        ("col_start", value<std::int64_t>()->default_value(0),
          "col_start (default: 0)")
        ("col_stop", value<std::int64_t>()->default_value(27278),
         "col_stop (default: 27278)")
        ("tiling", value<std::string>()->default_value("horizontal"),
          "tiling method ('horizontal' (default) or 'vertical')")
    ;
// 718 user (rows) and 27278 movies (columns)

    // make sure hpx_main is run on all localities
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(desc, argc, argv);
}