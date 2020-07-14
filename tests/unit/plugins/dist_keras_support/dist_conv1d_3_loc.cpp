// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/distributed/iostream.hpp>
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

void test_conv1d_d_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    HPX_TEST_EQ(hpx::cout, result, comparison);
}

///////////////////////////////////////////////////////////////////////////////
// data parallelizaion with valid padding. The array is tiled on its pages and
// the filter is local
char const* const conv1d_d_code1 = R"(
    conv1d_d(
        constant_d(1, list(8, 6, 3), nil, nil, "const1", "page"),
        constant(-1, list(4, 3, 5)),
        "valid"
    ))";
char const* const res_code1 = R"(
    constant_d(-12, list(8, 3, 5), nil, nil, "const1/1", "page")
)";

// data parallelizaion with same padding. The array is tiled on its pages and
// the filter is local
char const* const conv1d_d_code2 = R"(
    conv1d_d(
        constant_d(1, list(4, 6, 2), nil, nil, "data_par", "page"),
        constant(-1, list(4, 2, 3)),
        "same"
    ))";

// data parallelizaion with causal padding. The array is tiled on its pages and
// the filter is local
char const* const conv1d_d_code3 = R"(
    conv1d_d(
        constant_d(1, list(3, 6, 2), nil, nil, "data_par_causal", "page"),
        constant(2, list(4, 2, 3)),
        "causal"
    ))";

void test_conv1d_d_0()
{
    test_conv1d_d_d_operation("test_conv1d_d__0", conv1d_d_code1, res_code1);
}

void test_conv1d_d_1()
{
    if (hpx::get_locality_id() == 0)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__1", conv1d_d_code2, R"(
                annotate_d([[[-6., -6., -6.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-6., -6., -6.],
                             [-4., -4., -4.]],

                            [[-6., -6., -6.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-6., -6., -6.],
                             [-4., -4., -4.]]],
                    "data_par/1", list("tile", list("pages", 0, 2),
                        list("rows", 0, 6), list("columns", 0, 3))
                )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__1", conv1d_d_code2, R"(
                annotate_d([[[-6., -6., -6.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-6., -6., -6.],
                             [-4., -4., -4.]]],
                    "data_par/1", list("tile", list("pages", 2, 3),
                        list("rows", 0, 6), list("columns", 0, 3))
                )
        )");
    }
    else
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__1", conv1d_d_code2, R"(
                annotate_d([[[-6., -6., -6.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-8., -8., -8.],
                             [-6., -6., -6.],
                             [-4., -4., -4.]]],
                    "data_par/1", list("tile", list("pages", 3, 4),
                        list("rows", 0, 6), list("columns", 0, 3))
                )
        )");
    }
}

void test_conv1d_d_2()
{
    if (hpx::get_locality_id() == 0)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__2", conv1d_d_code3, R"(
                annotate_d([[[ 4.,  4.,  4.],
                             [ 8.,  8.,  8.],
                             [12., 12., 12.],
                             [16., 16., 16.],
                             [16., 16., 16.],
                             [16., 16., 16.]]],
                    "data_par_causal/1", list("tile", list("pages", 0, 1),
                        list("rows", 0, 6), list("columns", 0, 3))
                )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__2", conv1d_d_code3, R"(
                annotate_d([[[ 4.,  4.,  4.],
                             [ 8.,  8.,  8.],
                             [12., 12., 12.],
                             [16., 16., 16.],
                             [16., 16., 16.],
                             [16., 16., 16.]]],
                    "data_par_causal/1", list("tile", list("pages", 1, 2),
                        list("rows", 0, 6), list("columns", 0, 3))
                )
        )");
    }
    else
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__2", conv1d_d_code3, R"(
                annotate_d([[[ 4.,  4.,  4.],
                             [ 8.,  8.,  8.],
                             [12., 12., 12.],
                             [16., 16., 16.],
                             [16., 16., 16.],
                             [16., 16., 16.]]],
                    "data_par_causal/1", list("tile", list("pages", 2, 3),
                        list("rows", 0, 6), list("columns", 0, 3))
                )
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
// spatial parallelization with valid padding, local kernel. The array is tiled
// on its rows and it has overlaps of fileter_length - 1
void test_conv1d_d_3()
{
    if (hpx::get_locality_id() == 0)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__3", R"(
                conv1d_d(
                    annotate_d(
                        [[[1, 2], [3, 4], [5, 6]],
                         [[-1,-2],[-3,-4],[-5,-6]]], "arg_3loc_0",
                        list("tile", list("pages", 0, 2), list("rows", 0, 3),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 1, 1, 2, 1], [-1, 1, 1, 1]]],
                    "valid"
                )
             )" , R"(
                annotate_d([[[ 1., 12.,  5.,  5.],
                             [ 5., 24.,  3.,  5.]],
                            [[-1.,-12., -5., -5.],
                             [-5.,-24., -3., -5.]]],
                    "arg_3loc_0/1", list("tile", list("pages", 0, 2),
                        list("rows", 0, 2), list("columns", 0, 4))
                )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__3", R"(
                conv1d_d(
                    annotate_d(
                        [[[3, 4], [5, 6], [7, 8], [9,10]],
                        [[-3,-4],[-5,-6],[-7,-8],[-9,-10]]], "arg_3loc_0",
                        list("tile", list("pages", 0, 2), list("rows", 1, 5),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 1, 1, 2, 1], [-1, 1, 1, 1]]],
                    "valid"
                )
             )" , R"(
                annotate_d([[[  5.,  24.,   3.,   5.],
                             [  9.,  36.,   1.,   5.],
                             [ 13.,  48.,  -1.,   5.]],
                            [[ -5., -24.,  -3.,  -5.],
                             [ -9., -36.,  -1.,  -5.],
                             [-13., -48.,   1.,  -5.]]],
                    "arg_3loc_0/1", list("tile", list("pages", 0, 2),
                        list("rows", 1, 4), list("columns", 0, 4))
                )
        )");
    }
    else
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__3", R"(
                conv1d_d(
                    annotate_d(
                        [[[7, 8],[9, 10], [11, 12]],
                        [[-7,-8],[-9,-10],[-11,-12]]], "arg_3loc_0",
                        list("tile", list("pages", 0, 2), list("rows", 3, 6),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 1, 1, 2, 1], [-1, 1, 1, 1]]],
                    "valid"
                )
             )" , R"(
                annotate_d([[[ 13.,  48.,  -1.,   5.],
                             [ 17.,  60.,  -3.,   5.]],
                            [[-13., -48.,   1.,  -5.],
                             [-17., -60.,   3.,  -5.]]],
                    "arg_3loc_0/1", list("tile", list("pages", 0, 2),
                        list("rows", 3, 5), list("columns", 0, 4))
                )
        )");
    }
}

// spatial parallelization with same padding, local kernel. The array is tiled
// on its rows and it has overlaps of fileter_length - 1
void test_conv1d_d_4()
{
    if (hpx::get_locality_id() == 0)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__4", R"(
                conv1d_d(
                    annotate_d(
                        [[[1, 2], [3, 4], [5, 6]],
                         [[-1,-2],[-3,-4],[-5,-6]]], "arg_3loc_4",
                        list("tile", list("pages", 0, 2), list("rows", 0, 3),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 3, 1, 2,-1], [ 0, 1, 0,-1]]],
                    "same"
                )
             )" , R"(
                annotate_d([[[ 11.,  12.,   1.,  -9.],
                             [ 21.,  24.,  -3., -17.]],
                            [[-11., -12.,  -1.,   9.],
                             [-21., -24.,   3.,  17.]]],
                    "arg_3loc_4/1", list("tile", list("pages", 0, 2),
                        list("rows", 0, 2), list("columns", 0, 4))
                )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__4", R"(
                conv1d_d(
                    annotate_d(
                        [[[3, 4], [5, 6], [7, 8], [9,10]],
                        [[-3,-4],[-5,-6],[-7,-8],[-9,-10]]], "arg_3loc_4",
                        list("tile", list("pages", 0, 2), list("rows", 1, 5),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 3, 1, 2,-1], [ 0, 1, 0,-1]]],
                    "same"
                )
             )" , R"(
                annotate_d([[[ 21.,  24.,  -3., -17.],
                             [ 31.,  36.,  -7., -25.],
                             [ 41.,  48., -11., -33.]],
                            [[-21., -24.,   3.,  17.],
                             [-31., -36.,   7.,  25.],
                             [-41., -48.,  11.,  33.]]],
                    "arg_3loc_4/1", list("tile", list("pages", 0, 2),
                        list("rows", 1, 4), list("columns", 0, 4))
                )
        )");
    }
    else
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__4", R"(
                conv1d_d(
                    annotate_d(
                        [[[7, 8],[9, 10], [11, 12]],
                        [[-7,-8],[-9,-10],[-11,-12]]], "arg_3loc_4",
                        list("tile", list("pages", 0, 2), list("rows", 3, 6),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 3, 1, 2,-1], [ 0, 1, 0,-1]]],
                    "same"
                )
             )" , R"(
                annotate_d([[[ 41.,  48., -11., -33.],
                             [ 51.,  60., -15., -41.],
                             [ 22.,  45., -45., -22.]],
                            [[-41., -48.,  11.,  33.],
                             [-51., -60.,  15.,  41.],
                             [-22., -45.,  45.,  22.]]],
                    "arg_3loc_4/1", list("tile", list("pages", 0, 2),
                        list("rows", 3, 6), list("columns", 0, 4))
                )
        )");
    }
}

// spatial parallelization with causal padding, local kernel. The array is tiled
// on its rows and it has overlaps of fileter_length - 1
void test_conv1d_d_5()
{
    if (hpx::get_locality_id() == 0)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__5", R"(
                conv1d_d(
                    annotate_d(
                        [[[1, 2], [3, 4], [5, 6]],
                         [[-1,-2],[-3,-4],[-5,-6]]], "arg_3loc_5",
                        list("tile", list("pages", 0, 2), list("rows", 0, 3),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 3, 1, 2,-1], [ 0, 1, 0,-1]]],
                    "causal"
                )
             )" , R"(
                annotate_d([[[  3.,   3.,   2.,  -3.],
                             [ 11.,  12.,   1.,  -9.],
                             [ 21.,  24.,  -3., -17.]],
                            [[ -3.,  -3.,  -2.,   3.],
                             [-11., -12.,  -1.,   9.],
                             [-21., -24.,   3.,  17.]]],
                    "arg_3loc_5/1", list("tile", list("pages", 0, 2),
                        list("rows", 0, 3), list("columns", 0, 4))
                )
        )");
    }
    else if (hpx::get_locality_id() == 1)
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__5", R"(
                conv1d_d(
                    annotate_d(
                        [[[3, 4], [5, 6], [7, 8], [9,10]],
                        [[-3,-4],[-5,-6],[-7,-8],[-9,-10]]], "arg_3loc_5",
                        list("tile", list("pages", 0, 2), list("rows", 1, 5),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 3, 1, 2,-1], [ 0, 1, 0,-1]]],
                    "causal"
                )
             )" , R"(
                annotate_d([[[ 21.,  24.,  -3., -17.],
                             [ 31.,  36.,  -7., -25.],
                             [ 41.,  48., -11., -33.]],
                            [[-21., -24.,   3.,  17.],
                             [-31., -36.,   7.,  25.],
                             [-41., -48.,  11.,  33.]]],
                    "arg_3loc_5/1", list("tile", list("pages", 0, 2),
                        list("rows", 2, 5), list("columns", 0, 4))
                )
        )");
    }
    else
    {
        test_conv1d_d_d_operation(
            "test_conv1d_d__5", R"(
                conv1d_d(
                    annotate_d(
                        [[[7, 8],[9, 10], [11, 12]],
                        [[-7,-8],[-9,-10],[-11,-12]]], "arg_3loc_5",
                        list("tile", list("pages", 0, 2), list("rows", 3, 6),
                            list("columns", 0, 2))
                    ),
                    [[[ 2, 3,-3,-2], [ 0, 1,-1, 0]],
                     [[ 3, 1, 2,-1], [ 0, 1, 0,-1]]],
                    "causal"
                )
             )" , R"(
                annotate_d([[[ 41.,  48., -11., -33.],
                             [ 51.,  60., -15., -41.]],
                            [[-41., -48.,  11.,  33.],
                             [-51., -60.,  15.,  41.]]],
                    "arg_3loc_5/1", list("tile", list("pages", 0, 2),
                        list("rows", 4, 6), list("columns", 0, 4))
                )
        )");
    }
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_conv1d_d_0();
    test_conv1d_d_1();
    test_conv1d_d_2();

    test_conv1d_d_3();
    test_conv1d_d_4();
    test_conv1d_d_5();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {"hpx.run_hpx_main!=1"};

    return hpx::init(argc, argv, cfg);
}
