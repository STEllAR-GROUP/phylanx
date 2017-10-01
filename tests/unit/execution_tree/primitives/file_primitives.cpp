//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <Eigen/Dense>

#include <algorithm>
#include <cstdio>
#include <vector>

void test_file_io_lit(phylanx::ir::node_data<double> const& in)
{
    std::string filename = std::tmpnam(nullptr);

    // write to file
    {
        phylanx::execution_tree::primitive litval =
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), in);

        phylanx::execution_tree::primitive outfile =
            hpx::new_<phylanx::execution_tree::primitives::file_write>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_value_type>{
                    {filename}, litval
                });

        auto f = outfile.eval();
        f.get();
    }

    // read back the file
    hpx::future<phylanx::ir::node_data<double>> f;
    {
        phylanx::execution_tree::primitive infile =
            hpx::new_<phylanx::execution_tree::primitives::file_read>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_value_type>{
                    {filename}
                });


        f = infile.eval();
    }

    HPX_TEST(in == f.get());

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
                std::vector<phylanx::execution_tree::primitive_value_type>{
                    {filename}, in
                });

        auto f = outfile.eval();
        f.get();
    }

    // read back the file
    hpx::future<phylanx::ir::node_data<double>> f;
    {
        phylanx::execution_tree::primitive infile =
            hpx::new_<phylanx::execution_tree::primitives::file_read>(
                hpx::find_here(),
                std::vector<phylanx::execution_tree::primitive_value_type>{
                    {filename}
                });


        f = infile.eval();
    }

    HPX_TEST(in == f.get());

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

    Eigen::VectorXd ev = Eigen::VectorXd::Random(1007);
    test_file_io(phylanx::ir::node_data<double>(std::move(ev)));

    std::vector<double> v(1007);
    std::generate(v.begin(), v.end(), std::rand);
    test_file_io(phylanx::ir::node_data<double>(std::move(v)));

    Eigen::MatrixXd m = Eigen::MatrixXd::Random(101, 101);
    test_file_io(phylanx::ir::node_data<double>(std::move(m)));

    return hpx::util::report_errors();
}

