//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

void test_generate_tree(std::string const& exprstr,
    phylanx::execution_tree::pattern_list const& patterns,
    phylanx::execution_tree::variables const& variables,
    double expected_result)
{
    phylanx::execution_tree::primitive p =
        phylanx::execution_tree::generate_tree(exprstr, patterns, variables);

    HPX_TEST_EQ(p.eval().get()[0], expected_result);
}

phylanx::execution_tree::primitive create_literal_value(double value)
{
    return phylanx::execution_tree::primitive(
        hpx::new_<phylanx::execution_tree::primitives::literal_value>(
            hpx::find_here(), phylanx::ir::node_data<double>(value)));
}

void test_add_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        { "_1 + _2", &phylanx::execution_tree::primitives::create<
            phylanx::execution_tree::primitives::add_operation>}
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("A + B", patterns, variables, 42.0);
    test_generate_tree("A + (B + C)", patterns, variables, 55.0);
    test_generate_tree("A + (B + A)", patterns, variables, 83.0);
    test_generate_tree("(A + B) + C", patterns, variables, 55.0);
}

void test_file_io_primitives()
{
    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    phylanx::execution_tree::pattern_list patterns = {
        { "file_write(_1, _2)", &phylanx::execution_tree::primitives::create<
                phylanx::execution_tree::primitives::file_write>},
        { "file_read(_1)", &phylanx::execution_tree::primitives::create<
                phylanx::execution_tree::primitives::file_read>}
    };

    test_generate_tree("file_write(\"test_file\", A)", patterns, variables, 41.0);
    test_generate_tree("file_read(\"test_file\")", patterns, variables, 41.0);
}

int main(int argc, char* argv[])
{
//     test_add_primitive();
    test_file_io_primitives();

    return 0;
}

