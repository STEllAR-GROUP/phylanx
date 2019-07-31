//   Copyright (c) 2018 Christopher Taylor
//
//   Distributed under the Boost Software License, Version 1.0.0. (See accompanying
//   file LICENSE_1_0.0.txt or copy at http://www.boost.org/LICENSE_1_0.0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/plugins/algorithms/impl/randomforest.hpp>
#include <phylanx/version.hpp>
#include <phylanx/config.hpp>
#include <phylanx/config/version.hpp>

#include <blaze/Blaze.h>

#include <hpx/hpx_init.hpp>
#include <hpx/include/agas.hpp>
#include <hpx/runtime_fwd.hpp>


#include <blaze/Math.h>
#include <boost/program_options.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

#include <phylanx/ir/node_data.hpp>
#include "impl/randomforest.hpp"

using namespace phylanx::algorithms::impl;

int hpx_main(boost::program_options::variables_map& vm)
{
    // evaluate generated execution tree
    auto ntrees = vm["trees"].as<std::uint64_t>();
    auto mnsize = vm["minsize"].as<std::uint64_t>();
    auto mxdepth = vm["maxdepth"].as<std::uint64_t>();
    auto samplesize = vm["samples"].as<double>();

    using namespace phylanx::algorithms::impl;

    blaze::DynamicMatrix<double> train{ { 1.0, 1.0, 1.0, 1.0, 0.0 }
        , { 1.0, 1.0, 1.0, 1.0, 0.0 }
        , { 1.0, 1.0, 1.0, 1.0, 1.0 }
        , { 1.0, 1.0, 1.0, 1.0, 1.0 }
    };

    blaze::DynamicVector<double> labels { 1.0, 1.0, 1.0, 1.0 };

    std::uint64_t const train_size{(train.rows() / 2UL)};
    auto const train_submat_data = blaze::submatrix( train, 0UL
        , 0UL, train.rows(), train.columns()-1UL );

    randomforest_impl rf(ntrees);

    // Measure execution time
    hpx::util::high_resolution_timer traintimer;

    rf.fit(train, labels, mxdepth, mnsize, samplesize);

    auto trainelapsed = traintimer.elapsed();

    blaze::DynamicVector<double> results(train.rows());

    // Make sure all counters are properly initialized,
    // don't reset current counter values
    hpx::reinit_active_counters(false);

    hpx::util::high_resolution_timer predicttimer;

    rf.predict(train, results);

    auto predictelapsed = predicttimer.elapsed();

    // Make sure all counters are properly initialized, don't reset current
    // counter values
    hpx::reinit_active_counters(false);

    std::cout << "fit lapsed\t" << trainelapsed << std::endl;
    std::cout << "predict lapsed\t" << predictelapsed << std::endl;

    for(auto & r : results) {
        std::cout << r << std::endl;
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // command line handling
    boost::program_options::options_description desc(
        "usage: randomforest [options]");

    desc.add_options()("trees,t",
        boost::program_options::value<std::uint64_t>()->default_value(5),
        "number of trees (default: 5)")("samples,s",
        boost::program_options::value<double>()->default_value(1.0),
        "ratio of sample size (default: 1.0)")("minsize,m",
        boost::program_options::value<std::uint64_t>()->default_value(1),
        "min size (default: 1")("maxdepth,d",
        boost::program_options::value<std::uint64_t>()->default_value(10),
        "max depth (default: 10)");

    return hpx::init(desc, argc, argv);
}
