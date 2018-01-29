//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/hpx_init.hpp>

#include <iostream>

#include <boost/program_options.hpp>
#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
char const* const lra_code = R"(block(
    //
    // Logistic regression analysis algorithm
    //
    //   x: [30, 2]
    //   y: [30]
    define(lra, x, y, alpha,
        block(
            define(weights, constant(0.0, shape(x, 1))),                // weights: [2]
            define(transx, transpose(x)),                               // transx:  [2, 30]
            define(pred, constant(0.0, shape(x, 0))),
            define(error, constant(0.0, shape(x, 0))),
            define(gradient, constant(0.0, shape(x, 1))),
            define(step, 0),
            while(
                step < 10,
                block(
                    store(pred, 1.0 / (1.0 + exp(-dot(x, weights)))),  // exp(-dot(x, weights)): [30], pred: [30]
                    store(error, pred - y),                            // error: [30]
                    store(gradient, dot(transx, error)),               // gradient: [2]
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

void print_counter(hpx::performance_counters::performance_counter& c)
{
    std::cout << "Counter values: [";
    c.reinit(hpx::launch::sync, false);
    auto values = c.get_counter_values_array(hpx::launch::sync, false);
    for (auto& i = values.values_.begin(); i != values.values_.end(); ++i)
    {
        if (i != values.values_.begin()) std::cout << ", ";
        std::cout << *i;
    }
    std::cout << "]" << std::endl;
}

int hpx_main()
{
    blaze::DynamicMatrix<double> v1{{15.04, 16.74}, {13.82, 24.49},
        {12.54, 16.32}, {23.09, 19.83}, {9.268, 12.87}, {9.676, 13.14},
        {12.22, 20.04}, {11.06, 17.12}, {16.3 , 15.7 }, {15.46, 23.95},
        {11.74, 14.69}, {14.81, 14.7 }, {13.4 , 20.52}, {14.58, 13.66},
        {15.05, 19.07}, {11.34, 18.61}, {18.31, 20.58}, {19.89, 20.26},
        {12.88, 18.22}, {12.75, 16.7 }, {9.295, 13.9 }, {24.63, 21.6 },
        {11.26, 19.83}, {13.71, 18.68}, {9.847, 15.68}, {8.571, 13.1 },
        {13.46, 18.75}, {12.34, 12.27}, {13.94, 13.17}, {12.07, 13.44}};

    blaze::DynamicVector<double> v2{1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0,
        1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1};

    // compile the given code
    phylanx::execution_tree::compiler::function_list snippets;
    auto lra = phylanx::execution_tree::compile(lra_code, snippets);

    hpx::performance_counters::performance_counter c("/phylanx{locality#*/total}/primitives/dot/time/eval");

    // evaluate generated execution tree
    auto x = phylanx::ir::node_data<double>{ v1 };
    auto y = phylanx::ir::node_data<double>{ v2 };
    auto alpha = phylanx::ir::node_data<double>{ 1e-5 };

    print_counter(c);

    auto result = lra(x, y, alpha);

    print_counter(c);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}



/////////////////////////////////////////////////////////////////////////////////
//int hpx_main(boost::program_options::variables_map& vm)
//{
//    unsigned int seed = (unsigned int)std::time(nullptr);
//    if (vm.count("seed"))
//        seed = vm["seed"].as<unsigned int>();
//
//    std::cout << "using seed: " << seed << std::endl;
//    std::srand(seed);
//
//    for (int i = 0; i != 10; ++i)
//    {
//        hpx::performance_counters::performance_counter c("/test/reinit-values");
//
//        c.reinit();
//
//        auto values = c.get_counter_values_array(hpx::launch::sync, false);
//
//        HPX_TEST_EQ(values.count_, static_cast<std::uint64_t>(i + 1));
//
//        std::vector<std::int64_t> expected(value_count_.load());
//        std::iota(expected.begin(), expected.end(), i);
//        HPX_TEST(values.values_ == expected);
//
//        std::string name = c.get_name(hpx::launch::sync);
//        HPX_TEST_EQ(name, std::string("/test{locality#0/total}/reinit-values"));
//    }
//
//    return hpx::finalize();
//}
//
//int main(int argc, char* argv[])
//{
//    // add command line option which controls the random number generator seed
//    using namespace boost::program_options;
//    options_description desc_commandline(
//        "Usage: " HPX_APPLICATION_STRING " [options]");
//
//    desc_commandline.add_options()
//        ("seed,s", value<unsigned int>(),
//            "the random number generator seed to use for this run")
//        ;
//
//    hpx::register_startup_function(&register_counter_type);
//
//    // Initialize and run HPX.
//    std::vector<std::string> const cfg = {
//        "hpx.os_threads=1"
//    };
//    HPX_TEST_EQ(hpx::init(desc_commandline, argc, argv, cfg), 0);
//
//    return hpx::util::report_errors();
//}
