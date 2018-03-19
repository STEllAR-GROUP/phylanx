// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

void test_int_iterator_inc()
{
    phylanx::ir::range_iterator it(0, 1);
    ++it;
    HPX_TEST_EQ(*it, 1);
}

void test_int_iterator_equal()
{
    phylanx::ir::range_iterator it(1, 1);
    HPX_TEST(it == it);
}

void test_int_iterator_deref()
{
    phylanx::ir::range_iterator it(9, 6);
    HPX_TEST_EQ(*it++, 9);
    HPX_TEST_EQ(*it, 15);
}

void test_arg_type_iterator()
{
    using arg_t = phylanx::execution_tree::primitive_argument_type;

    std::vector<arg_t> v{
        arg_t{static_cast<std::int64_t>(6)},
        arg_t{static_cast<std::int64_t>(9)},
        arg_t{static_cast<std::int64_t>(42)}};

    phylanx::ir::range_iterator it(v.begin());
    HPX_TEST_EQ(*it++, 6);
    HPX_TEST_EQ(*it++, 9);
    HPX_TEST_EQ(*it++, 42);
}

void test_int_range_stop_arg()
{
    phylanx::ir::range r(10);

    phylanx::ir::range_iterator rb = r.begin();
    for (std::size_t i = 0; i < 10; i++)
    {
        HPX_TEST_EQ(i, *rb++);
    }
    HPX_TEST(rb == r.end());
}

void test_int_range_all_args()
{
    phylanx::ir::range r(5, -4, -2);

    HPX_TEST_EQ(std::distance(r.begin(), r.end()), 5);
}

void test_arg_type_range()
{
    using arg_t = phylanx::execution_tree::primitive_argument_type;

    std::vector<arg_t> v{
        arg_t{static_cast<std::int64_t>(6)},
        arg_t{static_cast<std::int64_t>(9)},
        arg_t{static_cast<std::int64_t>(42)}};

    phylanx::ir::range r(v);

    HPX_TEST_EQ(std::distance(r.begin(), r.end()), 3);
}

void test_arg_pair_range()
{
    using arg_t = phylanx::execution_tree::primitive_argument_type;

    std::vector<arg_t> v{
        arg_t{static_cast<std::int64_t>(6)},
        arg_t{static_cast<std::int64_t>(9)},
        arg_t{static_cast<std::int64_t>(42)}};

    phylanx::ir::range r(v.begin(), v.end());

    HPX_TEST_EQ(std::distance(std::next(r.begin()), r.end()), 2);
}

int main(int argc, char* argv[])
{
    test_int_iterator_inc();
    test_int_iterator_equal();
    test_int_iterator_deref();

    test_arg_type_iterator();

    test_int_range_stop_arg();
    test_int_range_all_args();

    test_arg_type_range();
    test_arg_pair_range();

    return hpx::util::report_errors();
}
