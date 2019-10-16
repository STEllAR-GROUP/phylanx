// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/ir/dictionary.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run().arg_;
}

void test_dictionary_object()
{
    phylanx::ir::dictionary u;
    auto key_1 = phylanx::execution_tree::primitive_argument_type{
        phylanx::ir::node_data<std::int64_t>(42)};
    auto key_2 = phylanx::execution_tree::primitive_argument_type{
        phylanx::ir::node_data<std::int64_t>(42)};
    auto val_1 = phylanx::execution_tree::primitive_argument_type{
        std::string("Question of Life, Universe, and Everything")};
    u[key_1] = val_1;
    HPX_TEST_EQ(u[key_1], u[key_2]);
}

void test_hash_operation()
{
    std::hash<phylanx::util::recursive_wrapper<
        phylanx::execution_tree::primitive_argument_type>>
        v;

    // testing same hash inputs for uint8_t data type
    HPX_TEST_EQ(v(phylanx::execution_tree::primitive_argument_type{
                    phylanx::ir::node_data<std::uint8_t>(6)}),
        v(phylanx::execution_tree::primitive_argument_type{
            phylanx::ir::node_data<std::uint8_t>(6)}));

    // testing hash value for uint8_t data type
    HPX_TEST_EQ(9,
        v(phylanx::execution_tree::primitive_argument_type{
            phylanx::ir::node_data<std::uint8_t>(9)}));

    // testing hash value for int64_t data type
    HPX_TEST_EQ(42,
        v(phylanx::execution_tree::primitive_argument_type{
            phylanx::ir::node_data<std::int64_t>(42)}));

    // testing same hash inputs for int64_t data type
    HPX_TEST_EQ(v(phylanx::execution_tree::primitive_argument_type{
                    phylanx::ir::node_data<std::int64_t>(9)}),
        v(phylanx::execution_tree::primitive_argument_type{
            phylanx::ir::node_data<std::int64_t>(9)}));

    // testing same hash inputs for std::string data type
    HPX_TEST_EQ(v(phylanx::execution_tree::primitive_argument_type{
                    std::string("Question of Life, Universe, and Everything")}),
        v(phylanx::execution_tree::primitive_argument_type{
            std::string("Question of Life, Universe, and Everything")}));
}

void test_dict_print_function()
{
    phylanx::ir::dictionary u;

    auto key_1 = phylanx::execution_tree::primitive_argument_type{
        phylanx::ir::node_data<std::int64_t>(42)};
    auto val_1 = phylanx::execution_tree::primitive_argument_type{
        std::string("Question of Life, Universe, and Everything")};

    auto key_2 = phylanx::execution_tree::primitive_argument_type{
        std::string("Question?")};
    auto val_2 = phylanx::execution_tree::primitive_argument_type{
        phylanx::ir::node_data<double>(42.0)};

    u[key_1] = val_1;
    u[key_2] = val_2;

    std::ostringstream stream;
    stream << phylanx::execution_tree::primitive_argument_type{std::move(u)};
    const std::string str = stream.str();

    auto fail = std::string::npos;
    auto find1 = str.find("Question?: 42");
    auto find2 = str.find("42: Question of Life, Universe, and Everything");
    HPX_TEST_NEQ(fail,find1);
    HPX_TEST_NEQ(fail,find2);
}

int main(int argc, char* argv[])
{
    test_dictionary_object();
    test_hash_operation();
    test_dict_print_function();

    return hpx::util::report_errors();
}
