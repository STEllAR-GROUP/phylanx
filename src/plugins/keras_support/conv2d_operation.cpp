// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/conv2d_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const conv2d_operation::match_data =
    {
        hpx::util::make_tuple("conv2d",
        std::vector<std::string>{R"(
            conv2d(_1, _2_kernel,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, list(1,1)),
            __arg(_5_dilation_rate, list(1,1)))
        )"},
        &create_conv2d_operation, &create_primitive<conv2d_operation>,
        R"(x, kernel, padding, strides, dilation_rate
        Args:

            x (array) : a matrix
            kernel (array) : a matrix
            padding (optional, string) : padding mode, `valid` by default. It
                can be either `valid` or `same`. `vaild` means no
                padding. `same` results the output with the same shape as
                original array in case of unit strides.
            strides (optional, a tuple of integers) : the step to apply
                convolution over array. It sets to (1,1) by default.
            dilation_rate (optional, a tuple of integers) : indicates the
                dilation rate, the rate to sample the array in each step of
                convolution, (1,1) by default.

        Returns:

        2D convolution (or 2D mathematical cross-correlation))")
    };

    ///////////////////////////////////////////////////////////////////////////
    conv2d_operation::conv2d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Matrix1, typename Matrix2>
    double conv2d_operation::convolve_step(
        const Matrix1& m1, const Matrix2& m2) const
    {
        return blaze::sum(m1 % m2);
    }

    template <typename Matrix1, typename Matrix2>
    double conv2d_operation::convolve_step(const Matrix1& m1, const Matrix2& m2,
        std::size_t dilation_height, std::size_t dilation_width,
        std::size_t kernel_rows, std::size_t kernel_columns) const
    {
        return blaze::sum(
            blaze::columns(
                blaze::rows(
                    m1,
                    [dilation_height, kernel_rows](
                        std::size_t i) { return i * dilation_height; },
                    kernel_rows),
                [dilation_width, kernel_columns](
                    std::size_t j) { return j * dilation_width; },
                kernel_columns) %
            m2);
    }

    template <typename Matrix1, typename Matrix2>
    double conv2d_operation::convolve_step(const Matrix1& m1,
        const Matrix2& m2, std::size_t dilation_height,
        std::size_t dilation_width, std::size_t kernel_rows,
        std::size_t kernel_columns, std::size_t r_remainder,
        std::size_t c_remainder) const
    {
        return blaze::sum(
            blaze::columns(
                blaze::rows(
                    m1,
                    [dilation_height, kernel_rows, r_remainder](std::size_t i) {
                        return i * dilation_height + r_remainder;
                    },
                    kernel_rows),
                [dilation_width, kernel_columns, c_remainder](
                    std::size_t j) { return j * dilation_width + c_remainder; },
                kernel_columns) %
            m2);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_valid(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t filter_height = k.rows();
        std::size_t filter_width = k.columns();
        std::size_t result_rows = m.rows() - filter_height + 1;
        std::size_t result_columns = m.columns() - filter_width + 1;

        blaze::DynamicMatrix<double> result(result_rows, result_columns);

        for (std::size_t i = 0; i != result_rows; ++i)
            for (std::size_t j = 0; j != result_columns; ++j)
                result(i, j) = convolve_step(
                    blaze::submatrix(m, i, j, filter_height, filter_width), k);

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t filter_height = k.rows();
        std::size_t filter_width = k.columns();

        auto it = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it);

        std::size_t result_rows =
            blaze::ceil(static_cast<float>(m.rows() - filter_height + 1) /
                static_cast<float>(stride_height));
        std::size_t result_columns =
            blaze::ceil(static_cast<float>(m.columns() - filter_width + 1) /
                static_cast<float>(stride_width));

        blaze::DynamicMatrix<double> result(result_rows, result_columns);

        for (std::size_t i = 0; i != result_rows; ++i)
            for (std::size_t j = 0; j != result_columns; ++j)
                result(i, j) = convolve_step(
                    blaze::submatrix(m, i * stride_height, j * stride_width,
                        filter_height, filter_width),
                    k);

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_valid_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        ir::range&& dilation_rate) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t filter_height = k.rows();
        std::size_t filter_width = k.columns();

        auto it = dilation_rate.begin();
        std::size_t dilation_height = extract_scalar_integer_value_strict(*it);
        std::size_t dilation_width = extract_scalar_integer_value_strict(*++it);

        std::size_t dilated_filter_height =
            dilation_height * (filter_height - 1) + 1;
        std::size_t dilated_filter_width =
            dilation_width * (filter_width - 1) + 1;

        blaze::DynamicMatrix<double> result(
            m.rows() - dilated_filter_height + 1,
            m.columns() - dilated_filter_width + 1);

        for (std::size_t i = 0; i != result.rows(); ++i)
            for (std::size_t j = 0; j != result.columns(); ++j)
                result(i, j) = convolve_step(
                    blaze::submatrix(
                        m, i, j, dilated_filter_height, dilated_filter_width),
                    k, dilation_height, dilation_width, filter_height,
                    filter_width);

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_same(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t filter_height = k.rows();
        std::size_t filter_width = k.columns();
        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();
        std::size_t pad_top = (filter_height - 1) / 2;
        std::size_t pad_left = (filter_width - 1) / 2;
        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicMatrix<double> result(nrows, ncolumns);

        for (std::size_t r = 0; r != nrows; ++r)
        {
            r_rel = r - pad_top;
            for (std::size_t c = 0; c != ncolumns; ++c)
            {
                c_rel = c - pad_left;
                if (r_rel < 0)
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, 0, 0, filter_height + r_rel,
                                filter_width + c_rel),
                            blaze::submatrix(k, -r_rel, -c_rel,
                                filter_height + r_rel, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, 0, c_rel, filter_height + r_rel,
                                ncolumns - c_rel),
                            blaze::submatrix(k, -r_rel, 0,
                                filter_height + r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, 0, c_rel, filter_height + r_rel,
                                filter_width),
                            blaze::submatrix(k, -r_rel, 0,
                                filter_height + r_rel, filter_width));
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(filter_height))
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, 0, nrows - r_rel,
                                filter_width + c_rel),
                            blaze::submatrix(k, 0, -c_rel, nrows - r_rel,
                                filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              nrows - r_rel, ncolumns - c_rel),
                                blaze::submatrix(
                                    k, 0, 0, nrows - r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              nrows - r_rel, filter_width),
                                blaze::submatrix(
                                    k, 0, 0, nrows - r_rel, filter_width));
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, 0, filter_height,
                                filter_width + c_rel),
                            blaze::submatrix(k, 0, -c_rel, filter_height,
                                filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              filter_height, ncolumns - c_rel),
                                blaze::submatrix(
                                    k, 0, 0, filter_height, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              filter_height, filter_width),
                                k);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t filter_height = k.rows();
        std::size_t filter_width = k.columns();
        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

        auto it = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it);

        std::size_t pad_top;
        std::size_t pad_left;
        std::size_t pad_height;
        std::size_t pad_width;

        if (nrows % stride_height == 0)
            pad_height = (blaze::max)(
                filter_height - stride_height, static_cast<std::size_t>(0));
        else
            pad_height = (blaze::max)(filter_height - (nrows % stride_height),
                static_cast<std::size_t>(0));

        if (ncolumns % stride_width == 0)
            pad_width = (blaze::max)(
                filter_width - stride_width, static_cast<std::size_t>(0));
        else
            pad_width = (blaze::max)(filter_width - (ncolumns % stride_width),
                static_cast<std::size_t>(0));

        pad_top = pad_height / 2;
        pad_left = pad_width / 2;

        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicMatrix<double> result(
            blaze::ceil(
                static_cast<float>(nrows + pad_height - filter_height + 1) /
                static_cast<float>(stride_height)),
            blaze::ceil(
                static_cast<float>(ncolumns + pad_width - filter_width + 1) /
                static_cast<float>(stride_width)));


        for (std::size_t r = 0; r != result.rows(); ++r)
        {
            r_rel = r * stride_height - pad_top;
            for (std::size_t c = 0; c != result.columns(); ++c)
            {
                c_rel = c * stride_width - pad_left;
                if (r_rel < 0)
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, 0, 0, filter_height + r_rel,
                                filter_width + c_rel),
                            blaze::submatrix(k, -r_rel, -c_rel,
                                filter_height + r_rel, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, 0, c_rel, filter_height + r_rel,
                                ncolumns - c_rel),
                            blaze::submatrix(k, -r_rel, 0,
                                filter_height + r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, 0, c_rel, filter_height + r_rel,
                                filter_width),
                            blaze::submatrix(k, -r_rel, 0,
                                filter_height + r_rel, filter_width));
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(filter_height))
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, 0, nrows - r_rel,
                                filter_width + c_rel),
                            blaze::submatrix(k, 0, -c_rel, nrows - r_rel,
                                filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              nrows - r_rel, ncolumns - c_rel),
                                blaze::submatrix(
                                    k, 0, 0, nrows - r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              nrows - r_rel, filter_width),
                                blaze::submatrix(
                                    k, 0, 0, nrows - r_rel, filter_width));
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, 0, filter_height,
                                filter_width + c_rel),
                            blaze::submatrix(k, 0, -c_rel, filter_height,
                                filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              filter_height, ncolumns - c_rel),
                                blaze::submatrix(
                                    k, 0, 0, filter_height, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              filter_height, filter_width),
                                k);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_same_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        ir::range&& dilation_rate) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t filter_height = k.rows();
        std::size_t filter_width = k.columns();
        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

        auto it = dilation_rate.begin();
        std::size_t dilation_height = extract_scalar_integer_value_strict(*it);
        std::size_t dilation_width = extract_scalar_integer_value_strict(*++it);

        std::size_t dilated_filter_height =
            dilation_height * (filter_height - 1) + 1;
        std::size_t dilated_filter_width =
            dilation_width * (filter_width - 1) + 1;

        std::size_t pad_top = (dilated_filter_height - 1) / 2;
        std::size_t pad_left = (dilated_filter_width - 1) / 2;

        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicMatrix<double> result(nrows, ncolumns);

        for (std::size_t r = 0; r != nrows; ++r)
        {
            r_rel = r - pad_top;
            for (std::size_t c = 0; c != ncolumns; ++c)
            {
                c_rel = c - pad_left;
                if (r_rel < 0)
                {
                    std::size_t kernel_row_size;
                    std::size_t r_remainder = -r_rel % dilation_height;
                    if (nrows <= r_remainder)
                    {
                        kernel_row_size = 1;
                    }
                    else if (dilated_filter_height + r_rel > nrows)
                    {
                        kernel_row_size = blaze::ceil(
                            static_cast<float>(nrows - r_remainder) /
                            static_cast<float>(dilation_height));
                    }
                    else
                    {
                        kernel_row_size = blaze::ceil(
                            static_cast<float>(dilated_filter_height + r_rel) /
                            static_cast<float>(dilation_height));
                    }
                    std::size_t vector_row_size =
                        (blaze::min)(nrows, dilated_filter_height + r_rel);

                    if (r_remainder == 0)
                    {
                        if (c_rel < 0)
                        {
                            std::size_t kernel_column_size;
                            std::size_t c_remainder = -c_rel % dilation_width;
                            if (ncolumns <= c_remainder)
                            {
                                kernel_column_size = 1;
                            }
                            else if (dilated_filter_width + c_rel > ncolumns)
                            {
                                kernel_column_size = blaze::ceil(
                                    static_cast<float>(ncolumns - c_remainder) /
                                    static_cast<float>(dilation_width));
                            }
                            else
                            {
                                kernel_column_size =
                                    blaze::ceil(static_cast<float>(
                                        dilated_filter_width + c_rel) /
                                        static_cast<float>(dilation_width));
                            }
                            std::size_t vector_column_size =
                                (blaze::min)(ncolumns, dilated_filter_width + c_rel);

                            if (c_remainder == 0)
                            {
                                result(r, c) = convolve_step(
                                    blaze::submatrix(m, 0, 0, vector_row_size,
                                        vector_column_size),
                                    blaze::submatrix(k,
                                        blaze::ceil(static_cast<float>(-r_rel) /
                                            static_cast<float>(
                                                dilation_height)),
                                        blaze::ceil(static_cast<float>(-c_rel) /
                                            static_cast<float>(dilation_width)),
                                        kernel_row_size, kernel_column_size),
                                    dilation_height, dilation_width,
                                    kernel_row_size, kernel_column_size);
                            }
                            else if (dilation_width - c_remainder >= ncolumns)
                            {
                                result(r, c) = 0;
                            }
                            else
                            {
                                result(r, c) = convolve_step(
                                    blaze::submatrix(m, 0, 0, vector_row_size,
                                        vector_column_size),
                                    blaze::submatrix(k,
                                        blaze::ceil(static_cast<float>(-r_rel) /
                                            static_cast<float>(
                                                dilation_height)),
                                        blaze::ceil(static_cast<float>(-c_rel) /
                                            static_cast<float>(dilation_width)),
                                        kernel_row_size, kernel_column_size),
                                    dilation_height, dilation_width,
                                    kernel_row_size, kernel_column_size, 0,
                                    dilation_width - c_remainder);
                            }

                        }
                        else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                                static_cast<std::int64_t>(dilated_filter_width))
                        {
                            std::size_t kernel_column_size = blaze::ceil(
                                static_cast<float>(ncolumns - c_rel) /
                                static_cast<float>(dilation_width));
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, 0, c_rel, vector_row_size,
                                    ncolumns - c_rel),
                                blaze::submatrix(k,
                                    blaze::ceil(static_cast<float>(-r_rel) /
                                        static_cast<float>(dilation_height)),
                                    0, kernel_row_size, kernel_column_size),
                                dilation_height, dilation_width,
                                kernel_row_size, kernel_column_size);
                        }
                        else
                        {
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, 0, c_rel, vector_row_size,
                                    dilated_filter_width),
                                blaze::submatrix(k,
                                    blaze::ceil(static_cast<float>(-r_rel) /
                                        static_cast<float>(dilation_height)),
                                    0, kernel_row_size, filter_width),
                                dilation_height, dilation_width,
                                kernel_row_size, filter_width);
                        }
                    }
                    else if (dilation_height - r_remainder >= nrows)
                    {
                        result(r,c) = 0;
                    }
                    else
                    {
                        if (c_rel < 0)
                        {
                            std::size_t kernel_column_size;
                            std::size_t c_remainder = -c_rel % dilation_width;
                            if (ncolumns <= c_remainder)
                            {
                                kernel_column_size = 1;
                            }
                            else if (dilated_filter_width + c_rel > ncolumns)
                            {
                                kernel_column_size = blaze::ceil(
                                    static_cast<float>(ncolumns - c_remainder) /
                                    static_cast<float>(dilation_width));
                            }
                            else
                            {
                                kernel_column_size =
                                    blaze::ceil(static_cast<float>(
                                        dilated_filter_width + c_rel) /
                                        static_cast<float>(dilation_width));
                            }
                            std::size_t vector_column_size =
                                (blaze::min)(ncolumns, dilated_filter_width + c_rel);

                            if (c_remainder == 0)
                            {
                                result(r, c) = convolve_step(
                                    blaze::submatrix(m, 0, 0, vector_row_size,
                                        vector_column_size),
                                    blaze::submatrix(k,
                                        blaze::ceil(static_cast<float>(-r_rel) /
                                            static_cast<float>(
                                                dilation_height)),
                                        blaze::ceil(static_cast<float>(-c_rel) /
                                            static_cast<float>(dilation_width)),
                                        kernel_row_size, kernel_column_size),
                                    dilation_height, dilation_width,
                                    kernel_row_size, kernel_column_size,
                                    dilation_height - r_remainder, 0);
                            }
                            else if (dilation_width - c_remainder >= ncolumns)
                            {
                                result(r, c) = 0;
                            }
                            else
                            {
                                result(r, c) = convolve_step(
                                    blaze::submatrix(m, 0, 0, vector_row_size,
                                        vector_column_size),
                                    blaze::submatrix(k,
                                        blaze::ceil(static_cast<float>(-r_rel) /
                                            static_cast<float>(
                                                dilation_height)),
                                        blaze::ceil(static_cast<float>(-c_rel) /
                                            static_cast<float>(dilation_width)),
                                        kernel_row_size, kernel_column_size),
                                    dilation_height, dilation_width,
                                    kernel_row_size, kernel_column_size,
                                    dilation_height - r_remainder,
                                    dilation_width - c_remainder);
                            }
                        }
                        else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(dilated_filter_width))
                        {
                            std::size_t kernel_column_size = blaze::ceil(
                                static_cast<float>(ncolumns - c_rel) /
                                static_cast<float>(dilation_width));
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, 0, c_rel, vector_row_size,
                                    ncolumns - c_rel),
                                blaze::submatrix(k,
                                    blaze::ceil(static_cast<float>(-r_rel) /
                                        static_cast<float>(dilation_height)),
                                    0, kernel_row_size, kernel_column_size),
                                dilation_height, dilation_width,
                                kernel_row_size, kernel_column_size,
                                dilation_height - r_remainder, 0);
                        }
                        else
                        {
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, 0, c_rel, vector_row_size,
                                    dilated_filter_width),
                                blaze::submatrix(k,
                                    blaze::ceil(static_cast<float>(-r_rel) /
                                        static_cast<float>(dilation_height)),
                                    0, kernel_row_size, filter_width),
                                dilation_height, dilation_width,
                                kernel_row_size, filter_width,
                                dilation_height - r_remainder, 0);
                        }
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(dilated_filter_height))
                {
                    std::size_t kernel_row_size =
                        blaze::ceil(static_cast<float>(nrows - r_rel) /
                            static_cast<float>(dilation_height));
                    if (c_rel < 0)
                    {
                        std::size_t kernel_column_size;
                        std::size_t c_remainder = -c_rel % dilation_width;
                        if (ncolumns <= c_remainder)
                        {
                            kernel_column_size = 1;
                        }
                        else if (dilated_filter_width + c_rel > ncolumns)
                        {
                            kernel_column_size = blaze::ceil(
                                static_cast<float>(ncolumns - c_remainder) /
                                static_cast<float>(dilation_width));
                        }
                        else
                        {
                            kernel_column_size =
                                blaze::ceil(static_cast<float>(
                                                dilated_filter_width + c_rel) /
                                    static_cast<float>(dilation_width));
                        }
                        std::size_t vector_column_size =
                            (blaze::min)(ncolumns, dilated_filter_width + c_rel);

                        if (c_remainder == 0)
                        {
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, r_rel, 0, nrows - r_rel,
                                    vector_column_size),
                                blaze::submatrix(k, 0,
                                    blaze::ceil(static_cast<float>(-c_rel) /
                                        static_cast<float>(dilation_width)),
                                    kernel_row_size, kernel_column_size),
                                dilation_height, dilation_width,
                                kernel_row_size, kernel_column_size);
                        }
                        else if (dilation_width - c_remainder >= ncolumns)
                        {
                            result(r,c) = 0;
                        }
                        else
                        {
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, r_rel, 0, nrows - r_rel,
                                    vector_column_size),
                                blaze::submatrix(k, 0,
                                    blaze::ceil(static_cast<float>(-c_rel) /
                                        static_cast<float>(dilation_width)),
                                    kernel_row_size, kernel_column_size),
                                dilation_height, dilation_width,
                                kernel_row_size, kernel_column_size, 0,
                                dilation_width - c_remainder);
                        }
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(dilated_filter_width))
                    {
                        std::size_t kernel_column_size =
                            blaze::ceil(static_cast<float>(ncolumns - c_rel) /
                                static_cast<float>(dilation_width));
                        result(r, c) =
                            convolve_step(blaze::submatrix(m, r_rel, c_rel,
                                              nrows - r_rel, ncolumns - c_rel),
                                blaze::submatrix(k, 0, 0, kernel_row_size,
                                    kernel_column_size),
                                dilation_height, dilation_width,
                                kernel_row_size, kernel_column_size);
                    }
                    else
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, c_rel, nrows - r_rel,
                                dilated_filter_width),
                            blaze::submatrix(
                                k, 0, 0, kernel_row_size, filter_width),
                            dilation_height, dilation_width, kernel_row_size,
                            filter_width);
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        std::size_t kernel_column_size;
                        std::size_t c_remainder = -c_rel % dilation_width;
                        if (ncolumns <= c_remainder)
                        {
                            kernel_column_size = 1;
                        }
                        else if (dilated_filter_width + c_rel > ncolumns)
                        {
                            kernel_column_size = blaze::ceil(
                                static_cast<float>(ncolumns - c_remainder) /
                                static_cast<float>(dilation_width));
                        }
                        else
                        {
                            kernel_column_size =
                                blaze::ceil(static_cast<float>(
                                                dilated_filter_width + c_rel) /
                                    static_cast<float>(dilation_width));
                        }
                        std::size_t vector_column_size =
                            (blaze::min)(ncolumns, dilated_filter_width + c_rel);

                        if (c_remainder == 0)
                        {
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, r_rel, 0,
                                    dilated_filter_height, vector_column_size),
                                blaze::submatrix(k, 0,
                                    blaze::ceil(static_cast<float>(-c_rel) /
                                        static_cast<float>(dilation_width)),
                                    filter_height, kernel_column_size),
                                dilation_height, dilation_width, filter_height,
                                kernel_column_size);
                        }
                        else if (dilation_width - c_remainder >= ncolumns)
                        {
                            result(r,c) = 0;
                        }
                        else
                        {
                            result(r, c) = convolve_step(
                                blaze::submatrix(m, r_rel, 0,
                                    dilated_filter_height, vector_column_size),
                                blaze::submatrix(k, 0,
                                    blaze::ceil(static_cast<float>(-c_rel) /
                                        static_cast<float>(dilation_width)),
                                    filter_height, kernel_column_size),
                                dilation_height, dilation_width, filter_height,
                                kernel_column_size, 0,
                                dilation_width - c_remainder);
                        }
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(dilated_filter_width))
                    {
                        std::size_t kernel_column_size =
                            blaze::ceil(static_cast<float>(ncolumns - c_rel) /
                                static_cast<float>(dilation_width));
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, c_rel,
                                dilated_filter_height, ncolumns - c_rel),
                            blaze::submatrix(
                                k, 0, 0, filter_height, kernel_column_size),
                            dilation_height, dilation_width, filter_height,
                            kernel_column_size);
                    }
                    else
                    {
                        result(r, c) = convolve_step(
                            blaze::submatrix(m, r_rel, c_rel,
                                dilated_filter_height, dilated_filter_width),
                            k, dilation_height, dilation_width, filter_height,
                            filter_width);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    /////////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding) const
    {
        if (padding == "valid")
            return conv2d_valid(std::move(arg), std::move(kernel));

        // padding == "same"
        return conv2d_same(std::move(arg), std::move(kernel));
    }

    primitive_argument_type conv2d_operation::conv2d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, ir::range&& strides) const
    {
        if (padding == "valid")
            return conv2d_valid(
                std::move(arg), std::move(kernel), std::move(strides));

        // padding == "same"
        return conv2d_same(
            std::move(arg), std::move(kernel), std::move(strides));
    }

    primitive_argument_type conv2d_operation::conv2d_any_pad_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, ir::range&& dilation_rate) const
    {
        if (padding == "valid")
            return conv2d_valid_dilation(
                std::move(arg), std::move(kernel), std::move(dilation_rate));

        // padding == "same"
        return conv2d_same_dilation(
                std::move(arg), std::move(kernel), std::move(dilation_rate));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> conv2d_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv2d_operation::eval",
                generate_error_message("the conv2d_operation primitive requires "
                                       "between 2 and 5 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "conv2d_operation::eval",
                    generate_error_message(
                        "the conv2d_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {

                std::size_t ndim = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (ndim !=
                        extract_numeric_value_dimension(
                            args[1], this_->name_, this_->codename_) ||
                    ndim != 2)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "conv2d operation requires for x and kernel to be "
                            "matrices"));

                std::string padding;
                padding = extract_string_value(
                    args[2], this_->name_, this_->codename_);

                if (padding != "valid" && padding != "same")
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "invalid padding. Padding can be either valid "
                            "or same"));


                if (padding == "valid")
                {
                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> x_dims =
                        extract_numeric_value_dimensions(
                            args[0], this_->name_, this_->codename_);
                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
                        kernel_dims = extract_numeric_value_dimensions(
                            args[1], this_->name_, this_->codename_);
                    if (x_dims[0]<kernel_dims[0] || x_dims[1]<kernel_dims[1])
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "the kernel size cannot be greater than the "
                                "array size in any direction in the valid "
                                "padding mode"));
                }

                ir::range strides;
                if (!is_list_operand_strict(args[3]))
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "strides should be a tuple of two positive "
                            "integers"));

                strides = extract_list_value_strict(
                    args[3], this_->name_, this_->codename_);

                if (strides.size() != 2)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "conv2d_operation requires the strides to "
                            "be of rank 2"));

                bool flag = true;
                for (auto const it : strides)
                {
                    std::int64_t temp =
                        extract_scalar_integer_value_strict(it);
                    if (temp <= 0)
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "strides on each dimension are required to "
                                "be positive"));

                    if (temp != 1)
                        flag = false;
                }
                if (flag == true)
                    strides = ir::range(0);     // an empty range


                ir::range dilation_rate;
                if (!is_list_operand_strict(args[4]))
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "dilation_rate should be a tuple of two "
                            "positive integers"));

                dilation_rate = extract_list_value_strict(
                    args[4], this_->name_, this_->codename_);

                if (dilation_rate.size() != 2)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "conv2d_operation requires the dilation_rate "
                            "to be of rank 2"));

                flag = true;
                for (auto const it : dilation_rate)
                {
                    std::int64_t temp =
                        extract_scalar_integer_value_strict(it);
                    if (temp <= 0)
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "dilation_rates on each dimension are "
                                "required to be positive"));

                    if (temp != 1)
                        flag = false;
                }
                if (flag == true)
                    dilation_rate = ir::range(0);     // an empty range

                if (!strides.empty() && !dilation_rate.empty())
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));


                if (strides.empty() && dilation_rate.empty())
                {
                    return this_->conv2d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding));
                }
                else if (dilation_rate.empty())
                {
                    return this_->conv2d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), std::move(strides));
                }
                // strides == (1,1) and dilation_rate != (1,1)
                return this_->conv2d_any_pad_dilation(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_),
                    std::move(padding), std::move(dilation_rate));

            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
