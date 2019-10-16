//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
std::uint32_t get_seed()
{
    std::string const code = R"(block(
            define(call, get_seed()),
            call
        ))";

    auto call = compile(code);

    return static_cast<std::uint32_t>(
        phylanx::execution_tree::extract_scalar_integer_value(call()));
}

void set_seed(std::uint32_t seed)
{
    std::string const code = R"(block(
            define(call, seed, set_seed(seed)),
            call
        ))";

    auto call = compile(code);

    call(static_cast<std::int64_t>(seed));
}
///////////////////////////////////////////////////////////////////////////////
// generate single random double value
template <typename T, typename Gen, typename Dist>
void generate_0d(phylanx::execution_tree::compiler::function const& call,
    Gen& gen, Dist& dist)
{
    phylanx::execution_tree::primitive_arguments_type dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{0}}
    };

    auto result = call(dims);

    HPX_TEST_EQ(
        static_cast<T>(dist(gen)),
        static_cast<T>(
            phylanx::execution_tree::extract_node_data<T>(result)[0]));
}

// generate a random double vector
template <typename T, typename Gen, typename Dist>
void generate_1d(phylanx::execution_tree::compiler::function const& call,
    Gen& gen, Dist& dist)
{
    phylanx::execution_tree::primitive_arguments_type dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{32}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{0}}
    };

    auto result = call(dims);

    blaze::DynamicVector<T> v(32);
    for (auto& val : v)
    {
        val = dist(gen);
    }

    HPX_TEST_EQ(phylanx::ir::node_data<T>(std::move(v)),
        phylanx::execution_tree::extract_node_data<T>(result));
}

// generate a random double matrix
template <typename T, typename Gen, typename Dist>
void generate_2d(phylanx::execution_tree::compiler::function const& call,
    Gen& gen, Dist& dist)
{
    phylanx::execution_tree::primitive_arguments_type dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{32}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{16}}
    };

    auto result = call(dims);

    blaze::DynamicMatrix<T> m(32, 16);
    for (std::size_t row = 0; row != blaze::rows(m); ++row)
    {
        for (auto& val : blaze::row(m, row))
        {
            val = dist(gen);
        }
    }

    HPX_TEST_EQ(phylanx::ir::node_data<T>(std::move(m)),
        phylanx::execution_tree::extract_node_data<T>(result));
}

// generate a random double tensor
template <typename T, typename Gen, typename Dist>
void generate_3d(phylanx::execution_tree::compiler::function const& call,
    Gen& gen, Dist& dist)
{
    phylanx::execution_tree::primitive_arguments_type dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{3}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{32}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{16}}
    };

    auto result = call(dims);

    blaze::DynamicTensor<T> t(3, 32, 16);
    for (std::size_t page = 0; page != blaze::pages(t); ++page)
    {
        for (std::size_t row = 0; row != blaze::rows(t); ++row)
        {
            for (auto& val : blaze::row(blaze::pageslice(t, page), row))
            {
                val = dist(gen);
            }
        }
    }

    HPX_TEST_EQ(phylanx::ir::node_data<T>(std::move(t)),
        phylanx::execution_tree::extract_node_data<T>(result));
}

// generate a random double 4d array
template <typename T, typename Gen, typename Dist>
void generate_4d(phylanx::execution_tree::compiler::function const& call,
    Gen& gen, Dist& dist)
{
    phylanx::execution_tree::primitive_arguments_type dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{3}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{32}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{16}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{13}}
    };

    auto result = call(dims);

    blaze::DynamicArray<4UL, T> q(3UL, 32UL, 16UL, 13UL);
    for (std::size_t quat = 0; quat != blaze::quats(q); ++quat)
    {
        for (std::size_t page = 0; page != blaze::pages(q); ++page)
        {
            for (std::size_t row = 0; row != blaze::rows(q); ++row)
            {
                for (auto& val :
                    blaze::row(
                        blaze::pageslice(blaze::quatslice(q, quat), page), row))
                {
                    val = dist(gen);
                }
            }
        }
    }

    HPX_TEST_EQ(phylanx::ir::node_data<T>(std::move(q)),
        phylanx::execution_tree::extract_node_data<T>(result));
}

///////////////////////////////////////////////////////////////////////////////
void test_normal_distribution_implicit(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size)),
            call
        ))";

    auto call = compile(code);

    {
        std::normal_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_uniform_distribution_explicit(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "uniform")),
            call
        ))";

    auto call = compile(code);

    {
        std::uniform_real_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_uniform_distribution_explicit_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("uniform", 2.0, 4.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::uniform_real_distribution<double> dist{2.0, 4.0};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist{2.0, 4.0};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist{2.0, 4.0};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist{2.0, 4.0};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::uniform_real_distribution<double> dist{2.0, 4.0};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_uniform_int_distribution_explicit(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "uniform_int")),
            call
        ))";

    auto call = compile(code);

    {
        std::uniform_int_distribution<std::int64_t> dist;
        generate_0d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist;
        generate_1d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist;
        generate_2d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist;
        generate_3d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist;
        generate_4d<std::int64_t>(call, gen, dist);
    }
}

void test_uniform_int_distribution_explicit_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("uniform_int", 200, 400))),
            call
        ))";

    auto call = compile(code);

    {
        std::uniform_int_distribution<std::int64_t> dist{200, 400};
        generate_0d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist{200, 400};
        generate_1d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist{200, 400};
        generate_2d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist{200, 400};
        generate_3d<std::int64_t>(call, gen, dist);
    }
    {
        std::uniform_int_distribution<std::int64_t> dist{200, 400};
        generate_4d<std::int64_t>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_bernoulli_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "bernoulli")),
            call
        ))";

    auto call = compile(code);

    {
        std::bernoulli_distribution dist;
        generate_0d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist;
        generate_1d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist;
        generate_2d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist;
        generate_3d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist;
        generate_4d<std::uint8_t>(call, gen, dist);
    }
}

void test_bernoulli_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("bernoulli", 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::bernoulli_distribution dist{0.8};
        generate_0d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist{0.8};
        generate_1d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist{0.8};
        generate_2d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist{0.8};
        generate_3d<std::uint8_t>(call, gen, dist);
    }
    {
        std::bernoulli_distribution dist{0.8};
        generate_4d<std::uint8_t>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_binomial_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("binomial", 1.0, 0.5))),
            call
        ))";

    auto call = compile(code);

    {
        std::binomial_distribution<int> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_binomial_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("binomial", 10, 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::binomial_distribution<int> dist{10, 0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist{10, 0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist{10, 0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist{10, 0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::binomial_distribution<int> dist{10, 0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_negative_binomial_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("negative_binomial", 1.0, 0.5))),
            call
        ))";

    auto call = compile(code);

    {
        std::negative_binomial_distribution<int> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_negative_binomial_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("negative_binomial", 10, 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::negative_binomial_distribution<int> dist{10, 0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist{10, 0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist{10, 0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist{10, 0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::negative_binomial_distribution<int> dist{10, 0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_geometric_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("geometric", 0.5))),
            call
        ))";

    auto call = compile(code);

    {
        std::geometric_distribution<int> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_geometric_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("geometric", 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::geometric_distribution<int> dist{0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist{0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist{0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist{0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::geometric_distribution<int> dist{0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_poisson_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("poisson", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::poisson_distribution<int> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_poisson_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("poisson", 4))),
            call
        ))";

    auto call = compile(code);

    {
        std::poisson_distribution<int> dist{4};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist{4};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist{4};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist{4};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::poisson_distribution<int> dist{4};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_exponential_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("exponential", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::exponential_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_exponential_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("exponential", 2.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::exponential_distribution<double> dist{2.0};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist{2.0};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist{2.0};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist{2.0};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::exponential_distribution<double> dist{2.0};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_gamma_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("gamma", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::gamma_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_gamma_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("gamma", 0.8, 1.2))),
            call
        ))";

    auto call = compile(code);

    {
        std::gamma_distribution<double> dist{0.8, 1.2};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist{0.8, 1.2};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist{0.8, 1.2};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist{0.8, 1.2};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::gamma_distribution<double> dist{0.8, 1.2};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_weibull_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("weibull", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::weibull_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_weibull_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("weibull", 0.8, 1.2))),
            call
        ))";

    auto call = compile(code);

    {
        std::weibull_distribution<double> dist{0.8, 1.2};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist{0.8, 1.2};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist{0.8, 1.2};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist{0.8, 1.2};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::weibull_distribution<double> dist{0.8, 1.2};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_extreme_value_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "extreme_value")),
            call
        ))";

    auto call = compile(code);

    {
        std::extreme_value_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_extreme_value_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("extreme_value", 0.8, 1.2))),
            call
        ))";

    auto call = compile(code);

    {
        std::extreme_value_distribution<double> dist{0.8, 1.2};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist{0.8, 1.2};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist{0.8, 1.2};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist{0.8, 1.2};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::extreme_value_distribution<double> dist{0.8, 1.2};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_normal_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "normal")),
            call
        ))";

    auto call = compile(code);

    {
        std::normal_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_normal_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("normal", 0.8, 1.2))),
            call
        ))";

    auto call = compile(code);

    {
        std::normal_distribution<double> dist{0.8, 1.2};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist{0.8, 1.2};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist{0.8, 1.2};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist{0.8, 1.2};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::normal_distribution<double> dist{0.8, 1.2};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_truncated_normal_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "truncated_normal")),
            call
        ))";

    auto call = compile(code);

    {
        phylanx::util::truncated_normal_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_truncated_normal_distribution_params(std::mt19937& gen)
{
    using namespace phylanx::execution_tree::primitives;

    std::string const code = R"(block(
            define(call, size,
                random(size, list("truncated_normal", 0.8, 1.2))
            ),
            call
        ))";

    auto call = compile(code);

    {
        phylanx::util::truncated_normal_distribution<double> dist{0.8, 1.2};
        generate_0d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist{0.8, 1.2};
        generate_1d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist{0.8, 1.2};
        generate_2d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist{0.8, 1.2};
        generate_3d<double>(call, gen, dist);
    }
    {
        phylanx::util::truncated_normal_distribution<double> dist{0.8, 1.2};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_lognormal_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "lognormal")),
            call
        ))";

    auto call = compile(code);

    {
        std::lognormal_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_lognormal_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("lognormal", 0.8, 1.2))),
            call
        ))";

    auto call = compile(code);

    {
        std::lognormal_distribution<double> dist{0.8, 1.2};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist{0.8, 1.2};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist{0.8, 1.2};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist{0.8, 1.2};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::lognormal_distribution<double> dist{0.8, 1.2};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_chi_squared_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("chi_squared", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::chi_squared_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_chi_squared_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("chi_squared", 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::chi_squared_distribution<double> dist{0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist{0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist{0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist{0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::chi_squared_distribution<double> dist{0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_cauchy_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "cauchy")),
            call
        ))";

    auto call = compile(code);

    {
        std::cauchy_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_cauchy_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("cauchy", 0.6, 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::cauchy_distribution<double> dist{0.6, 0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist{0.6, 0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist{0.6, 0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist{0.6, 0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::cauchy_distribution<double> dist{0.6, 0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_fisher_f_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("fisher_f", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::fisher_f_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_fisher_f_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("fisher_f", 0.6, 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::fisher_f_distribution<double> dist{0.6, 0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist{0.6, 0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist{0.6, 0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist{0.6, 0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::fisher_f_distribution<double> dist{0.6, 0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
void test_student_t_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("student_t", 1.0))),
            call
        ))";

    auto call = compile(code);

    {
        std::student_t_distribution<double> dist;
        generate_0d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist;
        generate_1d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist;
        generate_2d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist;
        generate_3d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist;
        generate_4d<double>(call, gen, dist);
    }
}

void test_student_t_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, list("student_t", 0.8))),
            call
        ))";

    auto call = compile(code);

    {
        std::student_t_distribution<double> dist{0.8};
        generate_0d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist{0.8};
        generate_1d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist{0.8};
        generate_2d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist{0.8};
        generate_3d<double>(call, gen, dist);
    }
    {
        std::student_t_distribution<double> dist{0.8};
        generate_4d<double>(call, gen, dist);
    }
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    std::uint32_t seed = std::random_device{}();

    set_seed(seed);
    HPX_TEST_EQ(get_seed(), seed);

    std::mt19937 gen(seed);

    test_normal_distribution_implicit(gen);

    test_uniform_distribution_explicit(gen);
    test_uniform_distribution_explicit_params(gen);

    test_uniform_int_distribution_explicit(gen);
    test_uniform_int_distribution_explicit_params(gen);

    test_bernoulli_distribution(gen);
    test_bernoulli_distribution_params(gen);

    test_binomial_distribution(gen);
    test_binomial_distribution_params(gen);

    test_negative_binomial_distribution(gen);
    test_negative_binomial_distribution_params(gen);

    test_geometric_distribution(gen);
    test_geometric_distribution_params(gen);

    test_poisson_distribution(gen);
    test_poisson_distribution_params(gen);

    test_exponential_distribution(gen);
    test_exponential_distribution_params(gen);

    test_gamma_distribution(gen);
    test_gamma_distribution_params(gen);

    test_weibull_distribution(gen);
    test_weibull_distribution_params(gen);

    test_extreme_value_distribution(gen);
    test_extreme_value_distribution_params(gen);

    test_normal_distribution(gen);
    test_normal_distribution_params(gen);

    test_truncated_normal_distribution(gen);
    test_truncated_normal_distribution_params(gen);

    test_lognormal_distribution(gen);
    test_lognormal_distribution_params(gen);

    test_chi_squared_distribution(gen);
    test_chi_squared_distribution_params(gen);

    test_cauchy_distribution(gen);
    test_cauchy_distribution_params(gen);

    test_fisher_f_distribution(gen);
    test_fisher_f_distribution_params(gen);

    test_student_t_distribution(gen);
    test_student_t_distribution_params(gen);

    return hpx::util::report_errors();
}
