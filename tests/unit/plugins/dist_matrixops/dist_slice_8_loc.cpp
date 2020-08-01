// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/iostream.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/modules/testing.hpp>

#include <string>
#include <utility>
#include <vector>

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

void test_slice_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////

void test_slice_row_assign_local_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[1, 2, 3]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 0, 1))))
            define(v, [-1, 2, 3])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-1, 2, 3]], "array_local_0/1",
                list("tile", list("rows", 0, 1), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[4, 5, 6]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 1, 2))))
            define(v, [-4, 5, 6])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-4, 5, 6]], "array_local_0/1",
                list("tile", list("rows", 1, 2), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 2)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[7, 8, 9]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 2, 3))))
            define(v, [-7, 8, 9])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-7, 8, 9]], "array_local_0/1",
                list("tile", list("rows", 2, 3), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 3)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[10, 11, 12]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 3, 4))))
            define(v, [-10, 11, 12])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-10, 11, 12]], "array_local_0/1",
                list("tile", list("rows", 3, 4), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 4)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[13, 14, 15]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 4, 5))))
            define(v, [-13, 14, 15])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-13, 14, 15]], "array_local_0/1",
                list("tile", list("rows", 4, 5), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 5)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[16, 17, 18]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 5, 6))))
            define(v, [-16, 17, 18])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-16, 17, 18]], "array_local_0/1",
                list("tile", list("rows", 5, 6), list("columns", 0, 3)))
        )");
    }
    else if (hpx::get_locality_id() == 6)
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[19, 20, 21]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 6, 7))))
            define(v, [-19, 20, 21])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-19, 20, 21]], "array_local_0/1",
                list("tile", list("rows", 6, 7), list("columns", 0, 3)))
        )");
    }
    else
    {
        test_slice_d_operation("test_slice_row_8loc_local_0", R"(
            define(a, annotate_d([[22, 23, 24]], "array_local_0",
                list("tile", list("columns", 0, 3), list("rows", 7, 8))))
            define(v, [-22, 23, 24])
            store(slice_row(a, 0), v)
            a
        )", R"(
            annotate_d([[-22, 23, 24]], "array_local_0/1",
                list("tile", list("rows", 7, 8), list("columns", 0, 3)))
        )");
    }
}



///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{

    test_slice_row_assign_local_0();



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

