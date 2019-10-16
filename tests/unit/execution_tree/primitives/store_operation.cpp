//   Copyright (c) 2017 Alireza Kheirkhahan
//   Copyright (c) 2018 Bibek Wagle
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

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

/////////////////////////////////////////////////////////////////////////////
void test_store_operation()
{
    std::string const code = R"(block(
        define(a, 57.7),
        store(a, 42.0),
        a
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(1));
    HPX_TEST_EQ(result[0], 42.0);
}

void test_set_operation_1d()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55, 0.17, 0.85]),
        define(val, [0.152, 0.195]),
        store(slice(a, list(1, 4, 2), nil), val),
        a
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(5));
    HPX_TEST_EQ(result[0], 0.052);
    HPX_TEST_EQ(result[1], 0.152);
    HPX_TEST_EQ(result[2], 0.55);
    HPX_TEST_EQ(result[3], 0.195);
    HPX_TEST_EQ(result[4], 0.85);
}

void test_set_operation_1d_single_step()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55, 0.17, 0.85]),
        define(val, [0.152, 0.195]),
        store(slice(a, list(1, 3, 1), nil), val),
        a
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(5));
    HPX_TEST_EQ(result[0], 0.052);
    HPX_TEST_EQ(result[1], 0.152);
    HPX_TEST_EQ(result[2], 0.195);
    HPX_TEST_EQ(result[3], 0.17);
    HPX_TEST_EQ(result[4], 0.85);
}

void test_set_operation_1d_single_negative_step()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55, 0.17, 0.85]),
        define(val, [0.152, 0.195]),
        store(slice(a, list(3, 1, -1), nil), val),
        a
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(5));
    HPX_TEST_EQ(result[0], 0.052);
    HPX_TEST_EQ(result[1], 0.95);
    HPX_TEST_EQ(result[2], 0.195);
    HPX_TEST_EQ(result[3], 0.152);
    HPX_TEST_EQ(result[4], 0.85);
}

void test_set_operation_2d()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(f, [0.42, 0.84]),
        define(g, [0.34, 0.69]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val,vstack(list(f,g))),
        store(slice(input, list(1, 4, 2), list(1, 4, 2)), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.42, 0.18924143, 0.84, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.34, 0.38010922, 0.69, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}});

    HPX_TEST_EQ(result, expected);
}

void test_set_operation_2d_vector_input()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val, [0.34, 0.69]),
        store(slice(input, list(1, 4, 2), list(1, 4, 2)), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.34, 0.18924143, 0.69, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.34, 0.38010922, 0.69, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}});

    HPX_TEST_EQ(result, expected);
}

void test_set_operation_2d_vector_input_single_step()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val, [0.34, 0.97, 0.69]),
        store(slice(input, list(1, 4, 1), list(1, 4, 1)), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.34, 0.97, 0.69, 0.48111144},
        {0.04567072, 0.34, 0.97, 0.69, 0.54772486},
        {0.84430163, 0.34, 0.97, 0.69, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}});

    HPX_TEST_EQ(result, expected);
}

void test_set_operation_2d_vector_input_negative_single_step()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val, [0.69, 0.97, 0.34]),
        store(slice(input, list(4, 1, -1), list(4, 1, -1)), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.34, 0.97, 0.69},
        {0.84430163, 0.22872386, 0.34, 0.97, 0.69},
        {0.63714445, 0.06884843, 0.34, 0.97, 0.69}});

    HPX_TEST_EQ(result, expected);
}

void test_set_operation_2d_negative_step()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(f, [0.42, 0.84]),
        define(g, [0.34, 0.69]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val,vstack(list(f,g))),
        store(slice(input, list(3, 1, -1), list(3, 1, -1)), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.69, 0.34, 0.54772486},
        {0.84430163, 0.22872386, 0.84, 0.42, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}});

    HPX_TEST_EQ(result, expected);
}

void test_set_operation_2d_single_element_input()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val,0.42),
        store(slice(input, list(4, 1, -1), list(4, 1, -1)), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.42, 0.42, 0.42},
        {0.84430163, 0.22872386, 0.42, 0.42, 0.42},
        {0.63714445, 0.06884843, 0.42, 0.42, 0.42}});

    HPX_TEST_EQ(result, expected);
}

void test_set_single_value_to_vector()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55, 0.17, 0.85]),
        define(val, 0.42),
        store(slice(a, 1, nil), val),
        a
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(5));
    HPX_TEST_EQ(result[0], 0.052);
    HPX_TEST_EQ(result[1], 0.42);
    HPX_TEST_EQ(result[2], 0.55);
    HPX_TEST_EQ(result[3], 0.17);
    HPX_TEST_EQ(result[4], 0.85);
}

void test_set_single_value_to_vector_negative_dir()
{
    std::string const code = R"(block(
        define(a, [0.052, 0.95, 0.55, 0.17, 0.85]),
        define(val, 0.42),
        store(slice(a, -1, nil), val),
        a
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));

    HPX_TEST_EQ(result.size(), std::size_t(5));
    HPX_TEST_EQ(result[0], 0.052);
    HPX_TEST_EQ(result[1], 0.95);
    HPX_TEST_EQ(result[2], 0.55);
    HPX_TEST_EQ(result[3], 0.17);
    HPX_TEST_EQ(result[4], 0.42);
}

void test_set_single_value_to_matrix()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val,42.42),
        store(slice(input, 3, 3), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 42.42, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}});

    HPX_TEST_EQ(result, expected);
}

void test_set_single_value_to_matrix_negative_dir()
{
    std::string const code = R"(block(
        define(a, [0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998]),
        define(b, [0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144]),
        define(c, [0.04567072, 0.15471737, 0.77637891, 0.84232174, 0.54772486]),
        define(d, [0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563]),
        define(e, [0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477]),
        define(input, vstack(list(a,b,c,d,e))),
        define(val,42.42),
        store(slice(input, -3, -3), val),
        input
    ))";

    auto result =
        phylanx::execution_tree::extract_numeric_value(compile_and_run(code));
    auto expected = phylanx::ir::node_data<double>(blaze::DynamicMatrix<double>{
        {0.05286532, 0.95232529, 0.55222064, 0.17133773, 0.85641998},
        {0.84212087, 0.69646313, 0.18924143, 0.61812872, 0.48111144},
        {0.04567072, 0.15471737, 42.42 , 0.84232174, 0.54772486},
        {0.84430163, 0.22872386, 0.38010922, 0.93930709, 0.82180563},
        {0.63714445, 0.06884843, 0.9002559, 0.14518178, 0.5056477}});

    HPX_TEST_EQ(result, expected);
}

int main(int argc, char* argv[])
{
    test_store_operation();

    test_set_operation_1d();
    test_set_operation_1d_single_step();
    test_set_operation_1d_single_negative_step();

    test_set_operation_2d();
    test_set_operation_2d_vector_input();
    test_set_operation_2d_vector_input_single_step();
    test_set_operation_2d_vector_input_negative_single_step();
    test_set_operation_2d_negative_step();
    test_set_operation_2d_single_element_input();

    test_set_single_value_to_vector();
    test_set_single_value_to_vector_negative_dir();
    test_set_single_value_to_matrix();
    test_set_single_value_to_matrix_negative_dir();

    return hpx::util::report_errors();
}
