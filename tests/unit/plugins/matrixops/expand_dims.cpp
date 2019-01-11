// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code = phylanx::execution_tree::compile(codestr, snippets, env);
    return code.run();
}

///////////////////////////////////////////////////////////////////////////////
void test_expand_dims_operation(std::string const& code,
    std::string const& expected_str)
{
    HPX_TEST_EQ(compile_and_run(code), compile_and_run(expected_str));
}


///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{

    test_expand_dims_operation("expand_dims(42., -1)", "[42.]");
    test_expand_dims_operation("expand_dims(33, 0)", "[33]");
    test_expand_dims_operation(
        "expand_dims([42., 13., 33.], 1)", "[[ 42.],[ 13.],[ 33.]]");
    test_expand_dims_operation(
        "expand_dims([42., 13., 33.], -1)", "[[ 42.],[ 13.],[ 33.]]");
    test_expand_dims_operation(
        "expand_dims([42., 13., 33.], 0)", "[[42., 13., 33.]]");
    test_expand_dims_operation(
        "expand_dims([42., 13., 33.], -2)", "[[42., 13., 33.]]");
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_expand_dims_operation("expand_dims([[42., 13.],[2., 33.]], 0)",
        "[[[ 42.,  13.], [  2.,  33.]]]");
    test_expand_dims_operation("expand_dims([[42., 13.],[2., 33.]], -3)",
        "[[[ 42.,  13.], [  2.,  33.]]]");
    test_expand_dims_operation("expand_dims([[42., 13.],[2., 33.]], 2)",
        "[[[ 42.],[ 13.]], [[  2.],[ 33.]]]");
    test_expand_dims_operation("expand_dims([[42., 13.],[2., 33.]], -1)",
        "[[[ 42.],[ 13.]], [[  2.],[ 33.]]]");
    test_expand_dims_operation("expand_dims([[42., 13.],[2., 33.]], 1)",
        "[[[ 42.,  13.]], [[  2.,  33.]]]");
    test_expand_dims_operation("expand_dims([[42., 13.],[2., 33.]], -2)",
        "[[[ 42.,  13.]], [[  2.,  33.]]]");

#endif

    return hpx::util::report_errors();
}
