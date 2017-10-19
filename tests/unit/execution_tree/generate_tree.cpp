//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Bibek Wagle
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
    phylanx::execution_tree::primitive_argument_type p =
        phylanx::execution_tree::generate_tree(exprstr, patterns, variables);

    HPX_TEST_EQ(
        phylanx::execution_tree::numeric_operand(p).get()[0],
        expected_result);
}

void test_generate_tree(std::string const& exprstr,
    phylanx::execution_tree::pattern_list const& patterns,
    phylanx::execution_tree::variables const& variables,
    bool expected_result)
{
    phylanx::execution_tree::primitive_argument_type p =
        phylanx::execution_tree::generate_tree(exprstr, patterns, variables);

    HPX_TEST_EQ(
        phylanx::execution_tree::boolean_operand(p).get() != 0,
        expected_result);
}

phylanx::execution_tree::primitive create_literal_value(double value)
{
    return phylanx::execution_tree::primitive(
        hpx::new_<phylanx::execution_tree::primitives::variable>(
            hpx::find_here(), phylanx::ir::node_data<double>(value)));
}

void test_add_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data
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

void test_sub_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::sub_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}};

    test_generate_tree("A - B", patterns, variables, 40.0);
    test_generate_tree("A - (B - C)", patterns, variables, 53.0);
    test_generate_tree("A - (B - A)", patterns, variables, 81.0);
    test_generate_tree("(A - B) - C", patterns, variables, 27.0);
}

void test_mul_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::mul_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}};

    test_generate_tree("A * B", patterns, variables, 41.0);
    test_generate_tree("A * (B * C)", patterns, variables, 533.0);
    test_generate_tree("A * (B * A)", patterns, variables, 1681.0);
    test_generate_tree("(A * B) * C", patterns, variables, 533.0);
}

void test_math_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data,
        phylanx::execution_tree::primitives::sub_operation::match_data,
        phylanx::execution_tree::primitives::mul_operation::match_data,
        phylanx::execution_tree::primitives::exponential_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)},
        {"D", create_literal_value(5.0)}
    };

    test_generate_tree("A + ((B - C) * A)", patterns, variables, -451.0);
    test_generate_tree("(A * (B + A)) + C", patterns, variables, 1735.0);
    test_generate_tree("(A * B) - C", patterns, variables, 28.0);
    test_generate_tree("exp(D) * 2", patterns, variables, std::exp(5.0) * 2);
    test_generate_tree("exp(D) + 2", patterns, variables, std::exp(5.0) + 2);
    test_generate_tree("exp(D) - 2", patterns, variables, std::exp(5.0) - 2);
}

void test_file_io_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::file_read::match_data,
        phylanx::execution_tree::primitives::file_write::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("file_write(\"test_file\", A)", patterns, variables, 41.0);
    test_generate_tree("file_read(\"test_file\")", patterns, variables, 41.0);
}

void test_boolean_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::unary_minus_operation::match_data,
        phylanx::execution_tree::primitives::unary_not_operation::match_data,
        phylanx::execution_tree::primitives::and_operation::match_data,
        phylanx::execution_tree::primitives::or_operation::match_data,
        phylanx::execution_tree::primitives::equal::match_data,
        phylanx::execution_tree::primitives::not_equal::match_data,
        phylanx::execution_tree::primitives::less::match_data,
        phylanx::execution_tree::primitives::less_equal::match_data,
        phylanx::execution_tree::primitives::greater::match_data,
        phylanx::execution_tree::primitives::greater_equal::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(0.0)}
    };

    test_generate_tree("!A", patterns, variables, false);
    test_generate_tree("!!A", patterns, variables, true);

    test_generate_tree("-A", patterns, variables, -41.0);
    test_generate_tree("--A", patterns, variables, 41.0);

    test_generate_tree("A && A", patterns, variables, true);
    test_generate_tree("A && B", patterns, variables, false);

    test_generate_tree("B || B", patterns, variables, false);
    test_generate_tree("A || B", patterns, variables, true);

    test_generate_tree("A == A", patterns, variables, true);
    test_generate_tree("A == B", patterns, variables, false);

    test_generate_tree("A != A", patterns, variables, false);
    test_generate_tree("A != B", patterns, variables, true);

    test_generate_tree("A <= A", patterns, variables, true);
    test_generate_tree("A <= B", patterns, variables, false);

    test_generate_tree("A < A", patterns, variables, false);
    test_generate_tree("A < B", patterns, variables, false);

    test_generate_tree("A >= A", patterns, variables, true);
    test_generate_tree("A >= B", patterns, variables, true);

    test_generate_tree("A > A", patterns, variables, false);
    test_generate_tree("A > B", patterns, variables, true);
}

void test_block_primitives()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data,
        phylanx::execution_tree::primitives::block_operation::match_data,
        phylanx::execution_tree::primitives::parallel_block_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("block(A, B, A + B + C)", patterns, variables, 55.0);
    test_generate_tree("parallel_block(A, B, A + B + C)", patterns,
        variables, 55.0);
}

void test_store_primitive()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data,
        phylanx::execution_tree::primitives::block_operation::match_data,
        phylanx::execution_tree::primitives::constant::match_data,
        phylanx::execution_tree::primitives::store_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(13.0)}
    };

    test_generate_tree("store(C, A)", patterns, variables, 41.0);
    test_generate_tree("store(C, A + B)", patterns, variables, 42.0);

    test_generate_tree("block(store(B, constant(0, A)), B)", patterns, variables, 0.0);
    test_generate_tree("store(D, 0)", patterns, variables, 0.0);
}

void test_complex_expression()
{
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::add_operation::match_data,
        phylanx::execution_tree::primitives::div_operation::match_data,
        phylanx::execution_tree::primitives::dot_operation::match_data,
        phylanx::execution_tree::primitives::exponential_operation::match_data,
        phylanx::execution_tree::primitives::unary_minus_operation::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(2.0)},
        {"B", create_literal_value(3.0)},
    };

    test_generate_tree("dot(A, B)", patterns, variables, 6.0);
    test_generate_tree("-dot(A, B)", patterns, variables, -6.0);
    test_generate_tree("exp(-dot(A, B))", patterns, variables, std::exp(-6.0));
    test_generate_tree("1.0 + exp(-dot(A, B))",
        patterns, variables, 1.0 + std::exp(-6.0));
    test_generate_tree("1.0 / (1.0 + exp(-dot(A, B)))",
        patterns, variables, 1.0 / (1.0 + std::exp(-6.0)));
}

void test_multi_patterns()
{
    using phylanx::execution_tree::create;

    phylanx::execution_tree::pattern_list patterns = {
        std::vector<phylanx::execution_tree::match_pattern_type>{
            hpx::util::make_tuple("block1", "block(_1, _2, __3)",
                &create<phylanx::execution_tree::primitives::block_operation>),
            hpx::util::make_tuple("block2", "block(__1)",
                &create<phylanx::execution_tree::primitives::block_operation>)
        }
    };

    phylanx::execution_tree::variables variables = {
        {"A", create_literal_value(41.0)},
        {"B", create_literal_value(1.0)},
        {"C", create_literal_value(42.0)},
    };

    test_generate_tree("block(A)", patterns, variables, 41.0);
    test_generate_tree("block(A, B)", patterns, variables, 1.0);
    test_generate_tree("block(A, B, C)", patterns, variables, 42.0);
}

void test_if_conditional()
{
    // Test 1 
    //  two outcome true case
    phylanx::execution_tree::pattern_list patterns = {
        phylanx::execution_tree::primitives::if_conditional::match_data
    };

    phylanx::execution_tree::variables variables = {
        {"cond", create_literal_value(1.0)},
        {"true_case", create_literal_value(42.0)},
        {"false_case", create_literal_value(54.0)}
    };

    test_generate_tree(
        "if(cond, true_case, false_case)"
      , patterns
      , variables
      , 42.0);

    // Test 2 
    //  two outcome false case
    phylanx::execution_tree::pattern_list patterns2 = {
        phylanx::execution_tree::primitives::if_conditional::match_data
    };

    phylanx::execution_tree::variables variables2 = {
        {"cond", create_literal_value(false)},
        {"true_case", create_literal_value(42.0)},
        {"false_case", create_literal_value(54.0)}
    };

    test_generate_tree(
        "if(cond, true_case, false_case)"
      , patterns2
      , variables2
      , 54.0);

    // Test 3 
    //  one outcome true case
    phylanx::execution_tree::pattern_list patterns3 = {
        phylanx::execution_tree::primitives::if_conditional::match_data
    };

    phylanx::execution_tree::variables variables3 = {
        {"cond", create_literal_value(true)},
        {"true_case", create_literal_value(42.0)},
    };

    test_generate_tree(
        "if(cond, true_case)"
      , patterns3
      , variables3
      , 42.0);

    // Test 4 
    //  one outcome false case
    phylanx::execution_tree::pattern_list patterns4 = {
        phylanx::execution_tree::primitives::if_conditional::match_data
    };

    phylanx::execution_tree::variables variables4 = {
        {"cond", create_literal_value(false)},
        {"true_case", create_literal_value(42.0)},
    };

    phylanx::execution_tree::primitive_argument_type p =
        phylanx::execution_tree::generate_tree("if(cond, true_case)", patterns, variables);

    std::cout<<"Test 4:"<<std::endl
             <<"Index Value of primitive_argument_type= "<<p.index()<<std::endl;
/*
    HPX_TEST(
        !phylanx::execution_tree::valid(phylanx::execution_tree::extract_literal_value(p))
    );
*/
}

int main(int argc, char* argv[])
{
    test_add_primitive();
    test_sub_primitive();
    test_mul_primitive();
    test_math_primitives();
    test_file_io_primitives();
    test_boolean_primitives();
    test_block_primitives();
    test_store_primitive();
    test_complex_expression();
    test_multi_patterns();
    test_if_conditional();

    return hpx::util::report_errors();
}

