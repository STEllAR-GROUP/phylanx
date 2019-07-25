// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/format.hpp>
#include <hpx/runtime/get_num_localities.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <utility>

void test_define_variable_here(hpx::id_type const& locality)
{
    std::uint32_t locality_id = hpx::naming::get_locality_id_from_id(locality);

    auto comp = phylanx::execution_tree::create_compiler();
    auto const& def = comp.compile(hpx::launch::sync,
        hpx::util::format(R"(define{{L#{}}}(x, locality()))", locality_id));

    // executing the define will create the variable, executing whatever it
    // returned will bind the value to the variable
    def.run();

    auto const& code = comp.compile(hpx::launch::sync, "x");
    auto x = code.run();

    // executing x() will return the bound value
    HPX_TEST_EQ(std::int64_t(locality_id),
        phylanx::execution_tree::extract_scalar_integer_value(x()));
}

void test_define_variable_there(hpx::id_type const& locality)
{
    std::uint32_t locality_id = hpx::naming::get_locality_id_from_id(locality);

    auto comp = phylanx::execution_tree::create_compiler(locality);
    auto const& def = comp.compile(hpx::launch::sync,
        hpx::util::format(R"(define{{L#{}}}(x, locality()))", locality_id));

    // executing the define will create the variable, executing whatever it
    // returned will bind the value to the variable
    def.run();

    auto const& code = comp.compile(hpx::launch::sync, "x");
    auto x = code.run();

    // executing x() will return the bound value
    HPX_TEST_EQ(std::int64_t(locality_id),
        phylanx::execution_tree::extract_scalar_integer_value(x()));
}

void test_define_variable(hpx::id_type const& locality)
{
    std::uint32_t locality_id = hpx::naming::get_locality_id_from_id(locality);

    auto comp = phylanx::execution_tree::create_compiler(locality);
    auto const& def = comp.compile(hpx::launch::sync, "define(x, locality())");

    // executing the define will create the variable, executing whatever it
    // returned will bind the value to the variable
    def.run();

    auto const& code = comp.compile(hpx::launch::sync, "x");
    auto x = code.run();

    // executing x() will return the bound value
    HPX_TEST_EQ(std::int64_t(locality_id),
        phylanx::execution_tree::extract_scalar_integer_value(x()));
}

int main(int argc, char* argv[])
{
    HPX_TEST(hpx::get_num_localities(hpx::launch::sync) >= std::uint32_t(2));

    for (auto const& locality : hpx::find_all_localities())
    {
        test_define_variable_here(locality);
        test_define_variable_there(locality);
        test_define_variable(locality);
    }

    return hpx::util::report_errors();
}

