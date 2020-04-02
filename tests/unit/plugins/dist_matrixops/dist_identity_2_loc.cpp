// Copyright (c) 2020 Nanmiao Wu

//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);
    return code.run().arg_;
}

void test_identity_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);


    std::cout << result<<"\n";
    // comparing annotations
    HPX_TEST_EQ(*(result.annotation()),*(comparison.annotation()));
}

///////////////////////////////////////////////////////////////////////////////
void test_identity_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_identity_d_operation("test_identity_2loc_0", R"(
            identity_d(4, 0, 2)
        )", R"(
            annotate_d([[1.0, 0.0], [0.0, 1.0], [0.0, 0.0], 
                [0.0, 0.0]], 
                "identity_array_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 2), list("rows", 0, 4))))
        )");
    }
    else
    {
        test_identity_d_operation("test_identity_2loc_0", R"(
            identity_d(4, 1, 2)
        )", R"(
            annotate_d([[0.0, 0.0], [0.0, 0.0], [1.0, 0.0], 
                [0.0, 1.0]], 
                "identity_array_1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 2, 4), list("rows", 0, 4))))
        )");
    }
}



///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    // only annotations are compared
    test_identity_0();
    

    hpx::finalize();
    return hpx::util::report_errors();
}
int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(argc, argv, cfg);
}

