// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

void test_broadcast_to_scalar()
{
    using phylanx::execution_tree::extract_value_scalar;
    using phylanx::execution_tree::primitive_argument_type;
    using phylanx::ir::node_data;

    // scalar->scalar
    {
        auto result = extract_value_scalar<double>(
            primitive_argument_type{42.0}, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 0UL);
        HPX_TEST_EQ(result[0], 42.0);
    }

    // vector->scalar
    {
        blaze::DynamicVector<double> v(1, 42.0);
        auto result = extract_value_scalar<double>(
            primitive_argument_type{std::move(v)}, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 0UL);
        HPX_TEST_EQ(result[0], 42.0);
    }

    // matrix->scalar
    {
        blaze::DynamicMatrix<double> m(1, 1, 42.0);
        auto result = extract_value_scalar<double>(
            primitive_argument_type{std::move(m)}, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 0UL);
        HPX_TEST_EQ(result[0], 42.0);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // tensor->scalar
    {
        blaze::DynamicTensor<double> t(1, 1, 1, 42.0);
        auto result = extract_value_scalar<double>(
            primitive_argument_type{std::move(t)}, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 0UL);
        HPX_TEST_EQ(result[0], 42.0);
    }
#endif
}

void test_broadcast_to_vector()
{
    using phylanx::execution_tree::extract_value_vector;
    using phylanx::execution_tree::primitive_argument_type;
    using phylanx::ir::node_data;

    node_data<double> expected(blaze::DynamicVector<double>(16, 42.0));

    // scalar->vector
    {
        auto result = extract_value_vector<double>(
            primitive_argument_type{42.0}, 16, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 1UL);
        HPX_TEST_EQ(result.dimension(0), 16UL);
        HPX_TEST_EQ(result, expected);
    }

    // vector->vector
    {
        blaze::DynamicVector<double> v1(16, 42.0);
        auto result1 = extract_value_vector<double>(
            primitive_argument_type{std::move(v1)}, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 1UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1, expected);

        blaze::DynamicVector<double> v2(1, 42.0);
        auto result2 = extract_value_vector<double>(
            primitive_argument_type{std::move(v2)}, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 1UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2, expected);
    }

    // matrix->vector
    {
        blaze::DynamicMatrix<double> m1(16, 1, 42.0);
        auto result1 = extract_value_vector<double>(
            primitive_argument_type{std::move(m1)}, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 1UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1, expected);

        blaze::DynamicMatrix<double> m2(1, 16, 42.0);
        auto result2 = extract_value_vector<double>(
            primitive_argument_type{std::move(m2)}, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 1UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2, expected);

        blaze::DynamicMatrix<double> m3(1, 1, 42.0);
        auto result3 = extract_value_vector<double>(
            primitive_argument_type{std::move(m3)}, 16, "", "");
        HPX_TEST_EQ(result3.num_dimensions(), 1UL);
        HPX_TEST_EQ(result3.dimension(0), 16UL);
        HPX_TEST_EQ(result3, expected);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // tensor->vector
    {
        blaze::DynamicTensor<double> t1(16, 1, 1, 42.0);
        auto result1 = extract_value_vector<double>(
            primitive_argument_type{std::move(t1)}, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 1UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1, expected);

        blaze::DynamicTensor<double> t2(1, 16, 1, 42.0);
        auto result2 = extract_value_vector<double>(
            primitive_argument_type{std::move(t2)}, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 1UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2, expected);

        blaze::DynamicTensor<double> t3(1, 1, 16, 42.0);
        auto result3 = extract_value_vector<double>(
            primitive_argument_type{std::move(t3)}, 16, "", "");
        HPX_TEST_EQ(result3.num_dimensions(), 1UL);
        HPX_TEST_EQ(result3.dimension(0), 16UL);
        HPX_TEST_EQ(result3, expected);

        blaze::DynamicTensor<double> t4(1, 1, 1, 42.0);
        auto result4 = extract_value_vector<double>(
            primitive_argument_type{std::move(t4)}, 16, "", "");
        HPX_TEST_EQ(result4.num_dimensions(), 1UL);
        HPX_TEST_EQ(result4.dimension(0), 16UL);
        HPX_TEST_EQ(result4, expected);
    }
#endif
}

void test_broadcast_to_matrix()
{
    using phylanx::execution_tree::extract_value_matrix;
    using phylanx::execution_tree::primitive_argument_type;
    using phylanx::ir::node_data;

    node_data<double> expected16x16(blaze::DynamicMatrix<double>(16, 16, 42.0));
    node_data<double> expected16x8(blaze::DynamicMatrix<double>(16, 8, 42.0));
    node_data<double> expected8x16(blaze::DynamicMatrix<double>(8, 16, 42.0));

    // scalar->matrix
    {
        auto result = extract_value_matrix<double>(
            primitive_argument_type{42.0}, 16, 16, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 2UL);
        HPX_TEST_EQ(result.dimension(0), 16UL);
        HPX_TEST_EQ(result.dimension(1), 16UL);
        HPX_TEST_EQ(result, expected16x16);
    }

    // vector->matrix
    {
        blaze::DynamicVector<double> v1(16, 42.0);
        auto result1 = extract_value_matrix<double>(
            primitive_argument_type{std::move(v1)}, 16, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 2UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1.dimension(1), 16UL);
        HPX_TEST_EQ(result1, expected16x16);

        blaze::DynamicVector<double> v2(1, 42.0);
        auto result2 = extract_value_matrix<double>(
            primitive_argument_type{std::move(v2)}, 16, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 2UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2.dimension(1), 16UL);
        HPX_TEST_EQ(result2, expected16x16);
    }

    // matrix->matrix
    {
        blaze::DynamicMatrix<double> m1(16, 16, 42.0);
        auto result1 = extract_value_matrix<double>(
            primitive_argument_type{std::move(m1)}, 16, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 2UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1.dimension(1), 16UL);
        HPX_TEST_EQ(result1, expected16x16);

        blaze::DynamicMatrix<double> m2(16, 1, 42.0);
        auto result2 = extract_value_matrix<double>(
            primitive_argument_type{std::move(m2)}, 16, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 2UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2.dimension(1), 16UL);
        HPX_TEST_EQ(result2, expected16x16);

        blaze::DynamicMatrix<double> m3(1, 16, 42.0);
        auto result3 = extract_value_matrix<double>(
            primitive_argument_type{std::move(m3)}, 16, 16, "", "");
        HPX_TEST_EQ(result3.num_dimensions(), 2UL);
        HPX_TEST_EQ(result3.dimension(0), 16UL);
        HPX_TEST_EQ(result3.dimension(1), 16UL);
        HPX_TEST_EQ(result3, expected16x16);

        blaze::DynamicMatrix<double> m4(16, 1, 42.0);
        auto result4 = extract_value_matrix<double>(
            primitive_argument_type{std::move(m4)}, 16, 8, "", "");
        HPX_TEST_EQ(result4.num_dimensions(), 2UL);
        HPX_TEST_EQ(result4.dimension(0), 16UL);
        HPX_TEST_EQ(result4.dimension(1), 8UL);
        HPX_TEST_EQ(result4, expected16x8);

        blaze::DynamicMatrix<double> m5(1, 16, 42.0);
        auto result5 = extract_value_matrix<double>(
            primitive_argument_type{std::move(m5)}, 8, 16, "", "");
        HPX_TEST_EQ(result5.num_dimensions(), 2UL);
        HPX_TEST_EQ(result5.dimension(0), 8UL);
        HPX_TEST_EQ(result5.dimension(1), 16UL);
        HPX_TEST_EQ(result5, expected8x16);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // tensor->matrix
    {
        blaze::DynamicTensor<double> t2(1, 16, 16, 42.0);
        auto result2 = extract_value_matrix<double>(
            primitive_argument_type{std::move(t2)}, 16, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 2UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2.dimension(1), 16UL);
        HPX_TEST_EQ(result2, expected16x16);

        blaze::DynamicTensor<double> t4(1, 1, 16, 42.0);
        auto result4 = extract_value_matrix<double>(
            primitive_argument_type{std::move(t4)}, 16, 16, "", "");
        HPX_TEST_EQ(result4.num_dimensions(), 2UL);
        HPX_TEST_EQ(result4.dimension(0), 16UL);
        HPX_TEST_EQ(result4.dimension(1), 16UL);
        HPX_TEST_EQ(result4, expected16x16);

        blaze::DynamicTensor<double> t5(1, 16, 1, 42.0);
        auto result5 = extract_value_matrix<double>(
            primitive_argument_type{std::move(t5)}, 16, 16, "", "");
        HPX_TEST_EQ(result5.num_dimensions(), 2UL);
        HPX_TEST_EQ(result5.dimension(0), 16UL);
        HPX_TEST_EQ(result5.dimension(1), 16UL);
        HPX_TEST_EQ(result5, expected16x16);
    }
#endif
}

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
void test_broadcast_to_tensor()
{
    using phylanx::execution_tree::extract_value_tensor;
    using phylanx::execution_tree::primitive_argument_type;
    using phylanx::ir::node_data;

    node_data<double> expected(blaze::DynamicTensor<double>(16, 16, 16, 42.0));

    // scalar->tensor
    {
        auto result = extract_value_tensor<double>(
            primitive_argument_type{42.0}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result.num_dimensions(), 3UL);
        HPX_TEST_EQ(result.dimension(0), 16UL);
        HPX_TEST_EQ(result.dimension(1), 16UL);
        HPX_TEST_EQ(result.dimension(2), 16UL);
        HPX_TEST_EQ(result, expected);
    }

    // vector->tensor
    {
        blaze::DynamicVector<double> v1(16, 42.0);
        auto result1 = extract_value_tensor<double>(
            primitive_argument_type{std::move(v1)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 3UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1.dimension(1), 16UL);
        HPX_TEST_EQ(result1.dimension(2), 16UL);
        HPX_TEST_EQ(result1, expected);

        blaze::DynamicVector<double> v2(1, 42.0);
        auto result2 = extract_value_tensor<double>(
            primitive_argument_type{std::move(v2)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 3UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2.dimension(1), 16UL);
        HPX_TEST_EQ(result2.dimension(2), 16UL);
        HPX_TEST_EQ(result2, expected);
    }

    // matrix->tensor
    {
        blaze::DynamicMatrix<double> m1(16, 16, 42.0);
        auto result1 = extract_value_tensor<double>(
            primitive_argument_type{std::move(m1)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 3UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1.dimension(1), 16UL);
        HPX_TEST_EQ(result1.dimension(2), 16UL);
        HPX_TEST_EQ(result1, expected);

        blaze::DynamicMatrix<double> m2(1, 1, 42.0);
        auto result2 = extract_value_tensor<double>(
            primitive_argument_type{std::move(m2)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 3UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2.dimension(1), 16UL);
        HPX_TEST_EQ(result2.dimension(2), 16UL);
        HPX_TEST_EQ(result2, expected);

        blaze::DynamicMatrix<double> m3(16, 1, 42.0);
        auto result3 = extract_value_tensor<double>(
            primitive_argument_type{std::move(m3)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result3.num_dimensions(), 3UL);
        HPX_TEST_EQ(result3.dimension(0), 16UL);
        HPX_TEST_EQ(result3.dimension(1), 16UL);
        HPX_TEST_EQ(result3.dimension(2), 16UL);
        HPX_TEST_EQ(result3, expected);

        blaze::DynamicMatrix<double> m4(1, 16, 42.0);
        auto result4 = extract_value_tensor<double>(
            primitive_argument_type{std::move(m4)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result4.num_dimensions(), 3UL);
        HPX_TEST_EQ(result4.dimension(0), 16UL);
        HPX_TEST_EQ(result4.dimension(1), 16UL);
        HPX_TEST_EQ(result4.dimension(2), 16UL);
        HPX_TEST_EQ(result4, expected);
    }

    // tensor->tensor
    {
        blaze::DynamicTensor<double> t1(16, 16, 16, 42.0);
        auto result1 = extract_value_tensor<double>(
            primitive_argument_type{std::move(t1)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result1.num_dimensions(), 3UL);
        HPX_TEST_EQ(result1.dimension(0), 16UL);
        HPX_TEST_EQ(result1.dimension(1), 16UL);
        HPX_TEST_EQ(result1.dimension(2), 16UL);
        HPX_TEST_EQ(result1, expected);

        blaze::DynamicTensor<double> t2(1, 1, 1, 42.0);
        auto result2 = extract_value_tensor<double>(
            primitive_argument_type{std::move(t2)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result2.num_dimensions(), 3UL);
        HPX_TEST_EQ(result2.dimension(0), 16UL);
        HPX_TEST_EQ(result2.dimension(1), 16UL);
        HPX_TEST_EQ(result2.dimension(2), 16UL);
        HPX_TEST_EQ(result2, expected);

        blaze::DynamicTensor<double> t3(1, 1, 16, 42.0);
        auto result3 = extract_value_tensor<double>(
            primitive_argument_type{std::move(t3)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result3.num_dimensions(), 3UL);
        HPX_TEST_EQ(result3.dimension(0), 16UL);
        HPX_TEST_EQ(result3.dimension(1), 16UL);
        HPX_TEST_EQ(result3.dimension(2), 16UL);
        HPX_TEST_EQ(result3, expected);

        blaze::DynamicTensor<double> t4(1, 16, 1, 42.0);
        auto result4 = extract_value_tensor<double>(
            primitive_argument_type{std::move(t4)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result4.num_dimensions(), 3UL);
        HPX_TEST_EQ(result4.dimension(0), 16UL);
        HPX_TEST_EQ(result4.dimension(1), 16UL);
        HPX_TEST_EQ(result4.dimension(2), 16UL);
        HPX_TEST_EQ(result4, expected);

        blaze::DynamicTensor<double> t5(1, 16, 16, 42.0);
        auto result5 = extract_value_tensor<double>(
            primitive_argument_type{std::move(t5)}, 16, 16, 16, "", "");
        HPX_TEST_EQ(result5.num_dimensions(), 3UL);
        HPX_TEST_EQ(result5.dimension(0), 16UL);
        HPX_TEST_EQ(result5.dimension(1), 16UL);
        HPX_TEST_EQ(result5.dimension(2), 16UL);
        HPX_TEST_EQ(result5, expected);
    }
}
#endif

int main(int argc, char* argv[])
{
    test_broadcast_to_scalar();
    test_broadcast_to_vector();
    test_broadcast_to_matrix();
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    test_broadcast_to_tensor();
#endif
    return hpx::util::report_errors();
}

