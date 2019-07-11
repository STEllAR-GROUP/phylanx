//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <cstddef>

#include <blaze/Math.h>

void test_row_iterator_empty()
{
    blaze::DynamicMatrix<double> m1;

    const phylanx::util::matrix_row_iterator<decltype(m1)> m1_begin(m1);
    const phylanx::util::matrix_row_iterator<decltype(m1)> m1_end(
        m1, m1.rows());

    HPX_TEST(m1_begin == m1_end);
}

void test_row_iterator_iteration()
{
    blaze::DynamicMatrix<double> m1{{1, 2}, {3, 4}};

    const phylanx::util::matrix_row_iterator<decltype(m1)> m1_begin(m1);
    const phylanx::util::matrix_row_iterator<decltype(m1)> m1_end(
        m1, m1.rows());

    HPX_TEST(m1_begin != m1_end);

    std::size_t count = 0ul;

    for (auto it = m1_begin; it != m1_end; ++it)
    {
        ++count;
    }

    HPX_TEST_EQ(m1.rows(), count);
}

void test_row_iterator_dereference()
{
    blaze::DynamicMatrix<double> m1{{1, 2}, {3, 4}};

    phylanx::util::matrix_row_iterator<decltype(m1)> it(m1, 1);

    HPX_TEST(blaze::row(m1, 1) == *it);
}

void test_column_iterator_empty()
{
    blaze::DynamicMatrix<double> m1;

    const phylanx::util::matrix_column_iterator<decltype(m1)> m1_begin(m1);
    const phylanx::util::matrix_column_iterator<decltype(m1)> m1_end(
        m1, m1.columns());

    HPX_TEST(m1_begin == m1_end);
}

void test_column_iterator_iteration()
{
    blaze::DynamicMatrix<double> m1{{1, 2}, {3, 4}};

    const phylanx::util::matrix_column_iterator<decltype(m1)> m1_begin(m1);
    const phylanx::util::matrix_column_iterator<decltype(m1)> m1_end(
        m1, m1.columns());

    HPX_TEST(m1_begin != m1_end);

    std::size_t count = 0ul;

    for (auto it = m1_begin; it != m1_end; ++it)
    {
        ++count;
    }

    HPX_TEST_EQ(m1.columns(), count);
}

void test_column_iterator_dereference()
{
    blaze::DynamicMatrix<double> m1{{1, 2}, {3, 4}};

    const phylanx::util::matrix_column_iterator<decltype(m1)> it(m1, 1);

    HPX_TEST(blaze::column(m1, 1) == *it);
}

int main()
{
    test_row_iterator_empty();
    test_row_iterator_iteration();
    test_row_iterator_dereference();

    test_column_iterator_empty();
    test_column_iterator_iteration();
    test_column_iterator_dereference();

    return hpx::util::report_errors();
}
