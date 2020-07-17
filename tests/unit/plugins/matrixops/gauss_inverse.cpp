//   Copyright (c) 2020 Rory Hector
//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/distributed/iostream.hpp>

#include <chrono>
#include <cstdint>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>
#include <iomanip>

// Used across each test function
// Used for steps matrix -> future to result
static inline hpx::future<phylanx::execution_tree::primitive_argument_type>
phylanx_boilerplate(blaze::DynamicMatrix<double> m)
{
    // Create variable node in execution tree
    phylanx::execution_tree::primitive lhs =
        phylanx::execution_tree::primitives::create_variable(
            hpx::find_here(), phylanx::ir::node_data<double>(m));

    // Create inverse operation node in execution tree
    phylanx::execution_tree::primitive gaussInversion =
        phylanx::execution_tree::primitives::create_matrix_GJE_Inverse(
            hpx::find_here(),
            phylanx::execution_tree::primitive_arguments_type{std::move(lhs)});

    // Get a future to the eval() result
    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        gaussInversion.eval();

    return f;
}

// Tests the time needed for sequential matrix inversion
// by gaussian elimination compared to blaze, can
// optionally also test for correst result with testEQ
void test_gauss_inversion_2d_random_timing(unsigned long n)
{
    // Generate a random 12x12 matrix
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(n, n);
    hpx::cout << "Timing Results Given n = " << m.rows()
        << hpx::endl;

    auto startTime = std::chrono::high_resolution_clock::now();

    // Used for steps matrix -> future to result
    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        phylanx_boilerplate(m);

    // Get the result from the future
    f.get();

    // Display the time needed for everything
    // given a previously generated matrix
    auto endTimeGauss = std::chrono::high_resolution_clock::now();
    auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(
        endTimeGauss - startTime);
    hpx::cout << "Computation time gauss inverse: " << duration1.count()
              << " us" << hpx::endl;

    // Get the timing for blaze inversion
    auto beginTimeBlaze = std::chrono::high_resolution_clock::now();

    blaze::DynamicMatrix<double> expected = blaze::inv(m);

    auto endTimeBlaze = std::chrono::high_resolution_clock::now();
    auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(
        endTimeBlaze - beginTimeBlaze);
    hpx::cout << "Computation time blaze inverse: " << duration2.count()
              << " us" << hpx::endl;

    hpx::cout << "Blaze time / gauss time: "
              << 100.0 * (1.0 * duration2.count() / duration1.count()) << "%"
              << hpx::endl
              << hpx::endl;
}

// Tests for correstness comparing matrix inversion
// by gaussian elimination compared to blaze inv
// NOTE: Often throws errors due to rounding differences
// Should be updated soon to allow equality ranges
void test_gauss_inversion_2d_random_equality(unsigned long n)
{
    // Generate a random 12x12 matrix
    blaze::Rand<blaze::DynamicMatrix<double>> gen{};
    blaze::DynamicMatrix<double> m = gen.generate(n, n);

    // Used for steps matrix -> future to result
    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        phylanx_boilerplate(m);

    // See if result matches what it should be
    blaze::DynamicMatrix<double> expected = blaze::inv(m);
    if (HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get())))
    {
        hpx::cout << "Passed Equality Test on Random " << n << "x"
                  << n << " Matrix" << hpx::endl
                  << hpx::endl;
    }
}


void test_gauss_inversion_2d_specific_equality()
{
    // Generate a specfic simple matrix for testing solution
    // blaze::DynamicMatrix<double> m{{4.0F, 7.0F}, {2.0F, 6.0F}};
    blaze::DynamicMatrix<double>
      m{{5.0F, 4.0F, 7.0F, 1.0F, 3.0F},
        {0.0F, 0.0F, 0.0F, 2.0F, 1.0F},
        {0.0F, 0.0F, 3.0F, 2.0F, 2.0F},
        {0.0F, 3.0F, 2.0F, 3.0F, 4.0F},
        {5.0F, 5.0F, 1.0F, 9.0F, 1.0F}};

    hpx::future<phylanx::execution_tree::primitive_argument_type> f =
        phylanx_boilerplate(m);

    // See if result matches what it should be
    blaze::DynamicMatrix<double> expected = blaze::inv(m);
    if (HPX_TEST_EQ(phylanx::ir::node_data<double>(std::move(expected)),
            phylanx::execution_tree::extract_numeric_value(f.get())))
    {
        hpx::cout << "Passed Equality Test on Specific Matrix" << hpx::endl;
        hpx::cout << hpx::endl;
    }
}

int main(int argc, char* argv[])
{
    test_gauss_inversion_2d_specific_equality();
    test_gauss_inversion_2d_random_equality(100UL);
    test_gauss_inversion_2d_random_timing(50UL);

    return hpx::util::report_errors();
}
