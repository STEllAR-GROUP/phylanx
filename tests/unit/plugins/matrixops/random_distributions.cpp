//   Copyright (c) 2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile(std::string const& code)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    return phylanx::execution_tree::compile(code, snippets, env);
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
        phylanx::execution_tree::extract_integer_value(call()));
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
    std::vector<phylanx::execution_tree::primitive_argument_type> dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{1}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{1}}
    };

    auto result = call(dims);

    HPX_TEST_EQ(
        static_cast<T>(dist(gen)),
        static_cast<T>(
            phylanx::execution_tree::extract_numeric_value(result)[0]));
}

// generate a random double vector
template <typename T, typename Gen, typename Dist>
void generate_1d(phylanx::execution_tree::compiler::function const& call,
    Gen& gen, Dist& dist)
{
    std::vector<phylanx::execution_tree::primitive_argument_type> dims = {
        phylanx::execution_tree::primitive_argument_type{std::int64_t{32}},
        phylanx::execution_tree::primitive_argument_type{std::int64_t{1}}
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
    std::vector<phylanx::execution_tree::primitive_argument_type> dims = {
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
}

void test_uniform_distribution_explicit_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("uniform", 2.0, 4.0))),
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
}

void test_bernoulli_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("bernoulli", 0.8))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_binomial_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "binomial")),
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
}

void test_binomial_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("binomial", 10, 0.8))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_negative_binomial_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "negative_binomial")),
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
}

void test_negative_binomial_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("negative_binomial", 10, 0.8))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_geometric_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "geometric")),
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
}

void test_geometric_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("geometric", 0.8))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_poisson_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "poisson")),
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
}

void test_poisson_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("poisson", 4))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_exponential_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "exponential")),
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
}

void test_exponential_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("exponential", 2.0))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_gamma_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "gamma")),
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
}

void test_gamma_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("gamma", 0.8, 1.2))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_weibull_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "weibull")),
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
}

void test_weibull_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("weibull", 0.8, 1.2))),
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
}

void test_extreme_value_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("extreme_value", 0.8, 1.2))),
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
}

void test_normal_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("normal", 0.8, 1.2))),
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
}

void test_lognormal_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("lognormal", 0.8, 1.2))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_chi_squared_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "chi_squared")),
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
}

void test_chi_squared_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("chi_squared", 0.8))),
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
}

void test_cauchy_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("cauchy", 0.6, 0.8))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_fisher_f_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "fisher_f")),
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
}

void test_fisher_f_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("fisher_f", 0.6, 0.8))),
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
}

///////////////////////////////////////////////////////////////////////////////
void test_student_t_distribution(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, "student_t")),
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
}

void test_student_t_distribution_params(std::mt19937& gen)
{
    std::string const code = R"(block(
            define(call, size, random(size, '("student_t", 0.8))),
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
