//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

void test_file_io_lit(phylanx::ir::node_data<double> const& in)
{
    std::string filename = std::tmpnam(nullptr);

    // write to file
    {
        phylanx::execution_tree::primitive litval =
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), in);

        phylanx::execution_tree::primitive outfile =
            hpx::new_<phylanx::execution_tree::primitives::file_write>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    {filename}, litval
                });

        auto f = outfile.eval();
        f.get();
    }

    // read back the file
    hpx::future<phylanx::execution_tree::primitive_argument_type> f;
    {
        phylanx::execution_tree::primitive infile =
            hpx::new_<phylanx::execution_tree::primitives::file_read>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    {filename}
                });


        f = infile.eval();
    }

    HPX_TEST(in == phylanx::execution_tree::extract_numeric_value(f.get()));

    std::remove(filename.c_str());
}

void test_file_io_primitive(phylanx::ir::node_data<double> const& in)
{
    std::string filename = std::tmpnam(nullptr);

    // write to file
    {
        phylanx::execution_tree::primitive outfile =
            hpx::new_<phylanx::execution_tree::primitives::file_write>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    {filename}, in
                });

        auto f = outfile.eval();
        f.get();
    }

    // read back the file
    hpx::future<phylanx::execution_tree::primitive_argument_type> f;
    {
        phylanx::execution_tree::primitive infile =
            hpx::new_<phylanx::execution_tree::primitives::file_read>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    {filename}
                });


        f = infile.eval();
    }

    HPX_TEST(in == phylanx::execution_tree::extract_numeric_value(f.get()));

    std::remove(filename.c_str());
}

void test_file_io(phylanx::ir::node_data<double> const& in)
{
    test_file_io_lit(in);
    test_file_io_primitive(in);
}

int main(int argc, char* argv[])
{
    blaze::Rand<blaze::DynamicVector<double>> gen{};

    test_file_io(phylanx::ir::node_data<double>(42.0));

    blaze::DynamicVector<double> ev = gen.generate(1007UL);
    test_file_io(phylanx::ir::node_data<double>(std::move(ev)));

    std::vector<double> v(1007);
    std::generate(v.begin(), v.end(), std::rand);
    test_file_io(phylanx::ir::node_data<double>(std::move(v)));

    blaze::Rand<blaze::DynamicMatrix<double>> gen2{};

    blaze::DynamicMatrix<double> m = gen2.generate(101UL, 101UL);
    test_file_io(phylanx::ir::node_data<double>(std::move(m)));

    return hpx::util::report_errors();
}

