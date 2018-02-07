//   Copyright (c) 2018 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Fixing #177: Arguments are not being passed correctly to define

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <sstream>
#include <string>
#include <utility>

std::string const read_code = R"(block(
    //
    // display data
    //
    define(read, data_m, data_v, data_0,
        block(
            debug("Scalar Data Received = ", data_0),
            debug("Vector Data Recieved = ", data_v),
            debug("Matrix Data Recieved = ", data_m)
        )
    ),
    read
))";

int hpx_main(int argc, char* argv[])
{
    phylanx::execution_tree::compiler::function_list snippets;

    blaze::DynamicVector<double> vec{1, 10, 11, 12, 13, 14, 15, 16};
    blaze::DynamicMatrix<double> mat{{1, 10, 11}, {12, 13, 14}, {15, 16, 17}};

    auto data_0d = phylanx::ir::node_data<double>{42.0};
    auto data_vec = phylanx::ir::node_data<double>{std::move(vec)};
    auto data_mat = phylanx::ir::node_data<double>{std::move(mat)};

    auto func = phylanx::execution_tree::compile(read_code, snippets);

    func(std::move(data_mat), std::move(data_vec), std::move(data_0d));

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    std::stringstream const& strm = hpx::get_consolestream();
    HPX_TEST_EQ(strm.str(), std::string(
        "Scalar Data Received = 42\n"
        "Vector Data Recieved = [1, 10, 11, 12, 13, 14, 15, 16]\n"
        "Matrix Data Recieved = [[1, 10, 11], [12, 13, 14], [15, 16, 17]]\n"));

    return hpx::util::report_errors();
}
