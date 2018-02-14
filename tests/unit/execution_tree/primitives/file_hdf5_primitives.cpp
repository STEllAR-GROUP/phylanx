//   Copyright (c) 2018 Alireza Kheirkhahan
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
    std::string dataset_name("dataset");

    // write to file
    {
        phylanx::execution_tree::primitive litval =
            hpx::new_<phylanx::execution_tree::primitives::variable>(
                hpx::find_here(), in);

        phylanx::execution_tree::primitive outfile =
            hpx::new_<phylanx::execution_tree::primitives::file_write_hdf5>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    filename, dataset_name, litval});

        auto f = outfile.eval();
        f.get();
    }

    // read back the file
    hpx::future<phylanx::execution_tree::primitive_result_type> f;
    {
        phylanx::execution_tree::primitive infile =
            hpx::new_<phylanx::execution_tree::primitives::file_read_hdf5>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    filename, dataset_name});

        f = infile.eval();
    }

    HPX_TEST(in == phylanx::execution_tree::extract_numeric_value(f.get()));

    std::remove(filename.c_str());
}

void test_file_io_primitive(phylanx::ir::node_data<double> const& in)
{
    std::string filename = std::tmpnam(nullptr);
    std::string dataset_name("dataset");

    // write to file
    {
        phylanx::execution_tree::primitive outfile =
            hpx::new_<phylanx::execution_tree::primitives::file_write_hdf5>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    filename, dataset_name, in});

        auto f = outfile.eval();
        f.get();
    }

    // read back the file
    hpx::future<phylanx::execution_tree::primitive_result_type> f;
    {
        phylanx::execution_tree::primitive infile =
            hpx::new_<phylanx::execution_tree::primitives::file_read_hdf5>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_argument_type>{
                    filename, dataset_name});

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

    test_file_io(phylanx::ir::node_data<double>(42.0));

    blaze::Rand<blaze::DynamicVector<double>> gen{};
    blaze::DynamicVector<double> ev = gen.generate(1007UL);
    test_file_io(phylanx::ir::node_data<double>(std::move(ev)));

    std::vector<double> v(1007);
    std::generate(v.begin(), v.end(), std::rand);
    test_file_io(phylanx::ir::node_data<double>(std::move(v)));

    blaze::Rand<blaze::DynamicMatrix<double>> gen2{};

    blaze::DynamicMatrix<double> m = gen2.generate(101UL, 102UL);
    test_file_io(phylanx::ir::node_data<double>(std::move(m)));

    return hpx::util::report_errors();
}
