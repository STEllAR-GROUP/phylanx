// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::compiler::function compile_and_run(
    phylanx::execution_tree::compiler::expression_pattern_list& patterns,
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment(
            patterns, hpx::find_here());

    auto const& code =
        phylanx::execution_tree::compile("<unknown>", "<unknown>",
            phylanx::ast::generate_ast(codestr), snippets, env, patterns);
    return code.run().arg_;
}

void test_function(
    phylanx::execution_tree::compiler::expression_pattern_list& patterns,
    char const* code, std::int64_t expected)
{
    auto test = compile_and_run(patterns, code);
    HPX_TEST_EQ(expected,
        phylanx::execution_tree::extract_scalar_numeric_value(test()));
}

///////////////////////////////////////////////////////////////////////////////
// __test1
hpx::future<phylanx::execution_tree::primitive_argument_type> __test1_func(
    phylanx::execution_tree::primitive_arguments_type const& operands,
    phylanx::execution_tree::primitive_arguments_type const& args,
    std::string const&, std::string const&,
    phylanx::execution_tree::eval_context)
{
    HPX_TEST_EQ(operands.size(), std::size_t(2));
    HPX_TEST_EQ(args.size(), std::size_t(0));

    return hpx::make_ready_future(operands[0]);
}

HPX_PLAIN_ACTION(__test1_func, __test1);

phylanx::execution_tree::match_pattern_type match__test1("__test1",
    std::vector<std::string>{"__test1(_1, _2)"},
    &phylanx::execution_tree::primitives::create_generic_function< ::__test1>,
    &phylanx::execution_tree::create_primitive<
        phylanx::execution_tree::primitives::generic_function< ::__test1>>,
    "Internal");

void test1_function_call()
{
    using namespace phylanx::execution_tree;

    compiler::expression_pattern_list patterns;

    match_pattern_type argmatch("__arg",
        std::vector<std::string>{"__arg(_1, _2)"}, nullptr, nullptr,
        "Internal");
    add_patterns(patterns, argmatch);

    add_patterns(patterns, match__test1);

    HPX_TEST_EQ(patterns.size(), std::size_t(2));

    auto it = patterns.find("__test1");
    HPX_TEST(it != patterns.end());

    HPX_TEST_EQ(it->second.pattern_, match__test1.patterns_[0]);
    HPX_TEST_EQ(it->second.args_.size(), std::size_t(2));
    for (auto const& p : it->second.args_)
    {
        HPX_TEST(p.empty());
    }
    HPX_TEST_EQ(it->second.defaults_.size(), std::size_t(0));

    test_function(patterns, "__test1(42, 43)", 42);
}

///////////////////////////////////////////////////////////////////////////////
// __test2
hpx::future<phylanx::execution_tree::primitive_argument_type> __test2_func(
    phylanx::execution_tree::primitive_arguments_type const& operands,
    phylanx::execution_tree::primitive_arguments_type const& args,
    std::string const&, std::string const&,
    phylanx::execution_tree::eval_context)
{
    HPX_TEST_EQ(operands.size(), std::size_t(3));
    HPX_TEST_EQ(args.size(), std::size_t(0));

    return hpx::make_ready_future(operands[1]);
}

HPX_PLAIN_ACTION(__test2_func, __test2);

phylanx::execution_tree::match_pattern_type match__test2("__test2",
    std::vector<std::string>{"__test2(_1, _2_name1, _3_name2)"},
    &phylanx::execution_tree::primitives::create_generic_function< ::__test2>,
    &phylanx::execution_tree::create_primitive<
        phylanx::execution_tree::primitives::generic_function< ::__test2>>,
    "Internal");

void test2_function_call()
{
    using namespace phylanx::execution_tree;

    compiler::expression_pattern_list patterns;

    match_pattern_type argmatch("__arg",
        std::vector<std::string>{"__arg(_1, _2)"}, nullptr, nullptr,
        "Internal");
    add_patterns(patterns, argmatch);

    add_patterns(patterns, match__test2);

    HPX_TEST_EQ(patterns.size(), std::size_t(2));

    auto it = patterns.find("__test2");
    HPX_TEST(it != patterns.end());

    HPX_TEST_EQ(it->second.pattern_, match__test2.patterns_[0]);
    HPX_TEST_EQ(it->second.args_.size(), std::size_t(3));
    HPX_TEST_EQ(it->second.args_[0], std::string());
    HPX_TEST_EQ(it->second.args_[1], std::string("name1"));
    HPX_TEST_EQ(it->second.args_[2], std::string("name2"));

    HPX_TEST_EQ(it->second.defaults_.size(), std::size_t(0));

    test_function(
        patterns, "__test2(42, __arg(name2, 43), __arg(name1, 44))", 44);
    test_function(
        patterns, "__test2(42, __arg(name1, 43), __arg(name2, 44))", 43);
    test_function(patterns, "__test2(42, 43, 44)", 43);
}

///////////////////////////////////////////////////////////////////////////////
// __test3
hpx::future<phylanx::execution_tree::primitive_argument_type> __test3_func(
    phylanx::execution_tree::primitive_arguments_type const& operands,
    phylanx::execution_tree::primitive_arguments_type const& args,
    std::string const&, std::string const&,
    phylanx::execution_tree::eval_context)
{
    HPX_TEST_EQ(operands.size(), std::size_t(3));
    HPX_TEST_EQ(args.size(), std::size_t(0));

    return hpx::make_ready_future(operands[1]);
}

HPX_PLAIN_ACTION(__test3_func, __test3);

phylanx::execution_tree::match_pattern_type match__test3("__test3",
    std::vector<std::string>{
        "__test3(_1, __arg(_2_name1, 1), __arg(_3_name2, 2))"},
    &phylanx::execution_tree::primitives::create_generic_function<::__test3>,
    &phylanx::execution_tree::create_primitive<
        phylanx::execution_tree::primitives::generic_function<::__test3>>,
    "Internal");

void test3_function_call()
{
    using namespace phylanx::execution_tree;

    compiler::expression_pattern_list patterns;

    match_pattern_type argmatch("__arg",
        std::vector<std::string>{"__arg(_1, _2)"}, nullptr, nullptr,
        "Internal");
    add_patterns(patterns, argmatch);

    add_patterns(patterns, match__test3);

    HPX_TEST_EQ(patterns.size(), std::size_t(4));

    auto p = patterns.equal_range("__test3");
    HPX_TEST(p.first != p.second);

    std::size_t i = 1;
    for (auto it = p.first; it != p.second; ++it, ++i)
    {
        HPX_TEST_EQ(it->second.args_.size(), 3);
        HPX_TEST_EQ(it->second.args_[0], std::string());
        HPX_TEST_EQ(it->second.args_[1], std::string("name1"));
        HPX_TEST_EQ(it->second.args_[2], std::string("name2"));

        HPX_TEST_EQ(it->second.defaults_.size(), std::size_t(2));
        HPX_TEST_EQ(it->second.defaults_[0], std::string("1"));
        HPX_TEST_EQ(it->second.defaults_[1], std::string("2"));
    }
    HPX_TEST_EQ(i, std::size_t(4));

    test_function(
        patterns, "__test3(42, __arg(name2, 43), __arg(name1, 44))", 44);
    test_function(
        patterns, "__test3(42, __arg(name1, 43), __arg(name2, 44))", 43);
    test_function(patterns, "__test3(42)", 1);
    test_function(patterns, "__test3(42, __arg(name1, 43))", 43);
    test_function(patterns, "__test3(42, __arg(name2, 44))", 1);
    test_function(patterns, "__test3(42, 43, 44)", 43);
    test_function(patterns, "__test3(42, 43, __arg(name2, 44))", 43);
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test1_function_call();
    test2_function_call();
    test3_function_call();

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    phylanx::execution_tree::register_pattern("__test1", match__test1, argv[0]);
    phylanx::execution_tree::register_pattern("__test2", match__test2, argv[0]);
    phylanx::execution_tree::register_pattern("__test3", match__test3, argv[0]);

    HPX_TEST_EQ(hpx::init(argc, argv), 0);

    return hpx::util::report_errors();
}

