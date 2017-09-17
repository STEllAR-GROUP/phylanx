//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <Eigen/Dense>

#include <algorithm>

void test_serialization(phylanx::ir::node_data<double> const& array_value1)
{
    phylanx::ir::node_data<double> array_value2;

    std::vector<char> buffer = phylanx::util::serialize(array_value1);
    phylanx::util::detail::unserialize(buffer, array_value2);

    HPX_TEST(array_value1 == array_value2);
}

int main(int argc, char* argv[])
{
    {
        phylanx::ir::node_data<double> single_value(42.0);
        HPX_TEST_EQ(single_value[0], 42.0);

        auto begin = hpx::util::begin(single_value);
        auto end = hpx::util::end(single_value);

        HPX_TEST(begin != end);
        HPX_TEST_EQ(*begin, 42.0);
        HPX_TEST(++begin == end);

        HPX_TEST_EQ(single_value.num_dimensions(), std::ptrdiff_t(0));
        HPX_TEST(single_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type({1, 0}));

        test_serialization(single_value);
    }

    {
        Eigen::VectorXd v = Eigen::VectorXd::Random(1007);

        phylanx::ir::node_data<double> array_value(v);

        auto begin = hpx::util::begin(array_value);
        auto end = hpx::util::end(array_value);

        HPX_TEST_EQ(std::distance(begin, end), v.rows());
        HPX_TEST(std::equal(begin, end, hpx::util::begin(v), hpx::util::end(v)));

        HPX_TEST_EQ(array_value.num_dimensions(), std::ptrdiff_t(1));
        HPX_TEST(array_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type({v.rows(), 0}));

        test_serialization(array_value);
    }

    {
        std::vector<double> v(1007);
        std::generate(v.begin(), v.end(), std::rand);

        phylanx::ir::node_data<double> array_value(v);

        auto begin = hpx::util::begin(array_value);
        auto end = hpx::util::end(array_value);

        HPX_TEST_EQ(std::distance(begin, end), v.size());
        HPX_TEST(std::equal(begin, end, std::begin(v), std::end(v)));

        HPX_TEST_EQ(array_value.num_dimensions(), std::ptrdiff_t(1));
        HPX_TEST(array_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type(
                {static_cast<std::ptrdiff_t>(v.size()), 0}));

        test_serialization(array_value);
    }

    {
        Eigen::MatrixXd m = Eigen::MatrixXd::Random(101, 101);

        phylanx::ir::node_data<double> array_value(m);

        auto begin = hpx::util::begin(array_value);
        auto end = hpx::util::end(array_value);

        HPX_TEST_EQ(std::distance(begin, end), m.size());
        HPX_TEST(std::equal(begin, end, hpx::util::begin(m), hpx::util::end(m)));

        HPX_TEST_EQ(array_value.num_dimensions(), std::ptrdiff_t(2));
        HPX_TEST(array_value.dimensions() ==
            phylanx::ir::node_data<double>::dimensions_type({m.rows(), m.cols()}));

        test_serialization(array_value);
    }

    return hpx::util::report_errors();
}
