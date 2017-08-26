//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <algorithm>
#include <cstdint>

void test_serialization(phylanx::ir::node_data<double> const& array_value1)
{
    std::vector<char> out_buffer;
    std::size_t archive_size = 0;

    {
        hpx::serialization::output_archive archive(out_buffer);
        archive << array_value1;
        archive_size = archive.bytes_written();
    }

    phylanx::ir::node_data<double> array_value2;

    {
        hpx::serialization::input_archive archive(
            out_buffer, archive_size);

        archive >> array_value2;
    }

    HPX_TEST(
        std::equal(
            array_value1.begin(), array_value1.end(),
            array_value2.begin(), array_value2.end()));
}

int main(int argc, char* argv[])
{
    {
        phylanx::ir::node_data<double> single_value(42.0);
        HPX_TEST_EQ(single_value[0], 42.0);

        auto begin = single_value.begin();
        auto end = single_value.end();

        HPX_TEST(begin != end);
        HPX_TEST_EQ(*begin, 42.0);
        HPX_TEST(++begin == end);

        test_serialization(single_value);
    }

    {
        std::vector<double> v(1007);
        std::generate(v.begin(), v.end(), std::rand);

        phylanx::ir::node_data<double> array_value(v);

        auto begin = array_value.begin();
        auto end = array_value.end();

        HPX_TEST_EQ(std::distance(begin, end), v.size());
        HPX_TEST(std::equal(begin, end, v.begin(), v.end()));

        test_serialization(array_value);
    }

    return hpx::util::report_errors();
}
