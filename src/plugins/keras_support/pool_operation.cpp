// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/pool_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const pool_operation::match_data =
    {
        match_pattern_type{"max_pool",
        std::vector<std::string>{"max_pool(_1, _2)", "max_pool(_1, _2, _3)",
        "max_pool(_1, _2, _3, _4)"},
        &create_pool_operation, &create_primitive<pool_operation>,
        R"(x, pool_size, padding, stride
        Args:

            x (array) : a matrix or a tensor
            pool_size (a tuple of integers) : the size of pooling over each
                dimension
            padding (string) : padding mode, it can be either `same` or `valid`
            stride (a tuple of integers) : the step to apply pooling over each
                dimension

        Returns:

        The result of max pooling with `pool_size` filters)"},

        match_pattern_type{"avg_pool",
        std::vector<std::string>{"avg_pool(_1, _2)", "avg_pool(_1, _2, _3)",
        "avg_pool(_1, _2, _3, _4)"},
        &create_pool_operation, &create_primitive<pool_operation>,
        R"(x, pool_size, padding, stride
        Args:

            x (array) : a matrix or a tensor
            pool_size (a tuple of integers) : the size of pooling oevr each
                dimension
            padding (string) : padding mode, it can be either `same` or `valid`
            stride (a tuple of integers) : the step to apply pooling oevr each
                dimension

        Returns:

        The result of average pooling with `pool_size` filters)"}

    };

    ///////////////////////////////////////////////////////////////////////////
    pool_operation::pool_mode extract_pool_mode(std::string const& name)
    {
        pool_operation::pool_mode result = pool_operation::max_pool;

        if (name.find("avg_pool") != std::string::npos)
        {
            result = pool_operation::avg_pool;
        }
        return result;
    }

    pool_operation::pool_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
        , mode_(extract_pool_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool pool_operation::validate_pooling(
        std::size_t const& ndim, ir::range const& pool_size) const
    {
        if (ndim != 2 && ndim != 3)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pool_operation::validate_pooling",
                generate_error_message(
                    "the pooling is only supported for matrices and tensors"));

        if (ndim != pool_size.size())
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pool_operation::validate_pooling",
                generate_error_message(
                    "the length of pool_size should be same as array "
                    "dimensions. For matrices, pool_size should be a tuple of "
                    "two integers and for tensors pool_size should be a tuple "
                    "of 3 integers"));

        auto it = pool_size.begin();
        for (std::size_t i = 0; i != pool_size.size(); ++i, ++it)
        {
            if (extract_scalar_integer_value_strict(*it) <= 0)
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pool_operation::validate_pooling",
                    generate_error_message(
                        "the height, width (and possibly depth) of a pooling "
                        "filter should be positive"));
        }
        return true;
    }

    bool pool_operation::validate_strides(
        std::size_t const& ndim, ir::range& strides) const
    {
        if (ndim != strides.size())
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pool_operation::validate_strides",
                generate_error_message(
                    "the length of strides should be same as array "
                    "dimensions. For matrices, strides should be a tuple of "
                    "two positive integers and for tensors strides should be a "
                    "tuple of three positive integers"));

        auto it = strides.begin();
        bool flag = true;
        for (std::size_t i = 0; i != strides.size(); ++i, ++it)
        {
            if (extract_scalar_integer_value_strict(*it) <= 0)
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pool_operation::validate_strides",
                    generate_error_message(
                        "the strides on each dimension should be positive"));
            if (extract_scalar_integer_value_strict(*it) != 1)
                flag = false;
        }
        if (flag == true)
            strides = ir::range(0);
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type pool_operation::max_pool2d(ir::node_data<T>&& arg,
        ir::range&& pool_size) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicMatrix<T> result(
            m.rows() - filter_height + 1, m.columns() - filter_width + 1);

        for (std::size_t r = 0; r != result.rows(); ++r)
            for (std::size_t c = 0; c != result.columns(); ++c)

                result(r, c) = (blaze::max)(
                    blaze::submatrix(m, r, c, filter_height, filter_width));

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool2d(
        ir::node_data<double>&& arg,
        ir::range&& pool_size) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicMatrix<double> result(
            m.rows() - filter_height + 1, m.columns() - filter_width + 1);

        for (std::size_t r = 0; r != result.rows(); ++r)
            for (std::size_t c = 0; c != result.columns(); ++c)

                result(r, c) = blaze::sum(
                    blaze::submatrix(m, r, c, filter_height, filter_width));

        result /= (filter_height * filter_width);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type pool_operation::max_pool2d(ir::node_data<T>&& arg,
        ir::range&& pool_size, ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        blaze::DynamicMatrix<double> result(
            blaze::ceil(static_cast<float>((m.rows() - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(static_cast<float>((m.columns() - filter_width + 1)) /
                static_cast<float>(stride_width)));

        for (std::size_t r = 0; r != result.rows(); ++r)
            for (std::size_t c = 0; c != result.columns(); ++c)

                result(r, c) =
                    (blaze::max)(blaze::submatrix(m, r * stride_height,
                        c * stride_width, filter_height, filter_width));

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool2d(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        blaze::DynamicMatrix<double> result(
            blaze::ceil(static_cast<float>((m.rows() - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(static_cast<float>((m.columns() - filter_width + 1)) /
                static_cast<float>(stride_width)));

        for (std::size_t r = 0; r != result.rows(); ++r)
            for (std::size_t c = 0; c != result.columns(); ++c)

                result(r, c) = blaze::sum(blaze::submatrix(m, r * stride_height,
                    c * stride_width, filter_height, filter_width));

        result /= (filter_height * filter_width);

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type pool_operation::max_pool2d_with_pad(
        ir::node_data<T>&& arg,
        ir::range&& pool_size) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        std::size_t pad_top = (filter_height - 1) / 2;
        std::size_t pad_left = (filter_width - 1) / 2;

        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicMatrix<T> result(nrows, ncolumns);

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
                        result(r, c) = (blaze::max)(blaze::submatrix(m, 0, 0,
                            filter_height + r_rel, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(m, 0,
                            c_rel, filter_height + r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, 0, c_rel, filter_height + r_rel, filter_width));
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(filter_height))
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, 0, nrows - r_rel, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, nrows - r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, nrows - r_rel, filter_width));
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, 0, filter_height, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, filter_height, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, filter_height, filter_width));
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool2d_with_pad(
        ir::node_data<double>&& arg,
        ir::range&& pool_size) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        std::size_t pad_top = (filter_height - 1) / 2;
        std::size_t pad_left = (filter_width - 1) / 2;

        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

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
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, 0, 0,
                                filter_height + r_rel, filter_width + c_rel)) /
                            ((filter_height + r_rel) * (filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, 0, c_rel,
                                filter_height + r_rel, ncolumns - c_rel)) /
                            ((filter_height + r_rel) * (ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, 0, c_rel,
                                filter_height + r_rel, filter_width)) /
                            ((filter_height + r_rel) * filter_width);
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(filter_height))
                {
                    if (c_rel < 0)
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, 0,
                                nrows - r_rel, filter_width + c_rel)) /
                            ((nrows - r_rel) * (filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, c_rel,
                                nrows - r_rel, ncolumns - c_rel)) /
                            ((nrows - r_rel) * (ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(
                                m, r_rel, c_rel, nrows - r_rel, filter_width)) /
                            ((nrows - r_rel) * filter_width);
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, 0,
                                filter_height, filter_width + c_rel)) /
                            (filter_height * (filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, c_rel,
                                filter_height, ncolumns - c_rel)) /
                            (filter_height * (ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(
                                m, r_rel, c_rel, filter_height, filter_width)) /
                            (filter_height * filter_width);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type pool_operation::max_pool2d_with_pad(
        ir::node_data<T>&& arg, ir::range&& pool_size,
        ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        std::size_t pad_top;
        std::size_t pad_left;
        std::size_t pad_height;
        std::size_t pad_width;

        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

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

        blaze::DynamicMatrix<T> result(
            blaze::ceil(
                static_cast<float>((nrows + pad_height - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(
                static_cast<float>((ncolumns + pad_width - filter_width + 1)) /
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
                        result(r, c) = (blaze::max)(blaze::submatrix(m, 0, 0,
                            filter_height + r_rel, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(m, 0,
                            c_rel, filter_height + r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, 0, c_rel, filter_height + r_rel, filter_width));
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(filter_height))
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, 0, nrows - r_rel, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, nrows - r_rel, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, nrows - r_rel, filter_width));
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, 0, filter_height, filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, filter_height, ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) = (blaze::max)(blaze::submatrix(
                            m, r_rel, c_rel, filter_height, filter_width));
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool2d_with_pad(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        std::size_t pad_top;
        std::size_t pad_left;
        std::size_t pad_height;
        std::size_t pad_width;

        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

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
                static_cast<float>((nrows + pad_height - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(
                static_cast<float>((ncolumns + pad_width - filter_width + 1)) /
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
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, 0, 0,
                                filter_height + r_rel, filter_width + c_rel)) /
                            ((filter_height + r_rel) * (filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, 0, c_rel,
                                filter_height + r_rel, ncolumns - c_rel)) /
                            ((filter_height + r_rel) * (ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, 0, c_rel,
                                filter_height + r_rel, filter_width)) /
                            ((filter_height + r_rel) * filter_width);
                    }
                }
                else if (r_rel > static_cast<std::int64_t>(nrows) -
                        static_cast<std::int64_t>(filter_height))
                {
                    if (c_rel < 0)
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, 0,
                                nrows - r_rel, filter_width + c_rel)) /
                            ((nrows - r_rel) * (filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, c_rel,
                                nrows - r_rel, ncolumns - c_rel)) /
                            ((nrows - r_rel) * (ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(
                                m, r_rel, c_rel, nrows - r_rel, filter_width)) /
                            ((nrows - r_rel) * filter_width);
                    }
                }
                else
                {
                    if (c_rel < 0)
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, 0,
                                filter_height, filter_width + c_rel)) /
                            (filter_height * (filter_width + c_rel));
                    }
                    else if (c_rel > static_cast<std::int64_t>(ncolumns) -
                            static_cast<std::int64_t>(filter_width))
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(m, r_rel, c_rel,
                                filter_height, ncolumns - c_rel)) /
                            (filter_height * (ncolumns - c_rel));
                    }
                    else
                    {
                        result(r, c) =
                            blaze::sum(blaze::submatrix(
                                m, r_rel, c_rel, filter_height, filter_width)) /
                            (filter_height * filter_width);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type pool_operation::max_pool3d(ir::node_data<T>&& arg,
        ir::range&& pool_size) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<T> result(t.pages() - filter_depth + 1,
            t.rows() - filter_height + 1, t.columns() - filter_width + 1);

        for (std::size_t p = 0; p != result.pages(); ++p)
            for (std::size_t r = 0; r != result.rows(); ++r)
                for (std::size_t c = 0; c != result.columns(); ++c)

                    result(p, r, c) = (blaze::max)(blaze::subtensor(
                        t, p, r, c, filter_depth, filter_height, filter_width));

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool3d(
        ir::node_data<double>&& arg,
        ir::range&& pool_size) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        blaze::DynamicTensor<double> result(t.pages() - filter_depth + 1,
            t.rows() - filter_height + 1, t.columns() - filter_width + 1);

        for (std::size_t p = 0; p != result.pages(); ++p)
            for (std::size_t r = 0; r != result.rows(); ++r)
                for (std::size_t c = 0; c != result.columns(); ++c)

                    result(p, r, c) = blaze::sum(blaze::subtensor(
                        t, p, r, c, filter_depth, filter_height, filter_width));

        result /= (filter_depth * filter_height * filter_width);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type pool_operation::max_pool3d(ir::node_data<T>&& arg,
        ir::range&& pool_size, ir::range&& strides) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_depth = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_height =
            extract_scalar_integer_value_strict(*++it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        blaze::DynamicTensor<T> result(
            (t.pages() - filter_depth) / stride_depth + 1,
            (t.rows() - filter_height) / stride_height + 1,
            (t.columns() - filter_width) / stride_width + 1);

        for (std::size_t p = 0; p != result.pages(); ++p)
            for (std::size_t r = 0; r != result.rows(); ++r)
                for (std::size_t c = 0; c != result.columns(); ++c)

                    result(p, r, c) = (blaze::max)(blaze::subtensor(t,
                        p * stride_depth, r * stride_height, c * stride_width,
                        filter_depth, filter_height, filter_width));

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool3d(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        ir::range&& strides) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_depth = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_height =
            extract_scalar_integer_value_strict(*++it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        blaze::DynamicTensor<double> result(
            (t.pages() - filter_depth) / stride_depth + 1,
            (t.rows() - filter_height) / stride_height + 1,
            (t.columns() - filter_width) / stride_width + 1);

        for (std::size_t p = 0; p != result.pages(); ++p)
            for (std::size_t r = 0; r != result.rows(); ++r)
                for (std::size_t c = 0; c != result.columns(); ++c)

                    result(p, r, c) = blaze::sum(blaze::subtensor(t,
                        p * stride_depth, r * stride_height, c * stride_width,
                        filter_depth, filter_height, filter_width));

        result /= (filter_depth * filter_height * filter_width);

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type pool_operation::max_pool3d_with_pad(
        ir::node_data<T>&& arg,
        ir::range&& pool_size) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        std::size_t pad_front = (filter_depth - 1) / 2;
        std::size_t pad_top = (filter_height - 1) / 2;
        std::size_t pad_left = (filter_width - 1) / 2;

        std::size_t npages = t.pages();
        std::size_t nrows = t.rows();
        std::size_t ncolumns = t.columns();

        std::int64_t p_rel;    //relative page
        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicTensor<T> result(npages, nrows, ncolumns);

        for (std::size_t p = 0; p != npages; ++p)
        {
            p_rel = p - pad_front;
            for (std::size_t r = 0; r != nrows; ++r)
            {
                r_rel = r - pad_top;
                for (std::size_t c = 0; c != ncolumns; ++c)
                {
                    c_rel = c - pad_left;
                    if (p_rel < 0)
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, 0, 0, filter_depth + p_rel,
                                    filter_height + r_rel,
                                    filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, 0, c_rel, filter_depth + p_rel,
                                    filter_height + r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, 0, c_rel, filter_depth + p_rel,
                                    filter_height + r_rel, filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, 0, filter_depth + p_rel,
                                    nrows - r_rel, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    nrows - r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    nrows - r_rel, filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, 0, filter_depth + p_rel,
                                    filter_height, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    filter_height, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    filter_height, filter_width));
                            }
                        }
                    }
                    else if (p_rel > static_cast<std::int64_t>(npages) -
                            static_cast<std::int64_t>(filter_depth))
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(
                                    blaze::subtensor(t, p_rel, 0, 0,
                                        npages - p_rel, filter_height + r_rel,
                                        filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, npages - p_rel,
                                    filter_height + r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, npages - p_rel,
                                    filter_height + r_rel, filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, npages - p_rel,
                                    nrows - r_rel, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    nrows - r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    nrows - r_rel, filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, npages - p_rel,
                                    filter_height, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    filter_height, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    filter_height, filter_width));
                            }
                        }
                    }
                    else
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    (blaze::max)(blaze::subtensor(t, p_rel, 0,
                                        0, filter_depth, filter_height + r_rel,
                                        filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, filter_depth,
                                    filter_height + r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, filter_depth,
                                    filter_height + r_rel, filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, filter_depth,
                                    nrows - r_rel, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    nrows - r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    nrows - r_rel, filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, filter_depth,
                                    filter_height, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    filter_height, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    filter_height, filter_width));
                            }
                        }
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool3d_with_pad(
        ir::node_data<double>&& arg,
        ir::range&& pool_size) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        std::size_t pad_front = (filter_depth - 1) / 2;
        std::size_t pad_top = (filter_height - 1) / 2;
        std::size_t pad_left = (filter_width - 1) / 2;

        std::size_t npages = t.pages();
        std::size_t nrows = t.rows();
        std::size_t ncolumns = t.columns();

        std::int64_t p_rel;    //relative page
        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicTensor<double> result(npages, nrows, ncolumns);

        for (std::size_t p = 0; p != npages; ++p)
        {
            p_rel = p - pad_front;
            for (std::size_t r = 0; r != nrows; ++r)
            {
                r_rel = r - pad_top;
                for (std::size_t c = 0; c != ncolumns; ++c)
                {
                    c_rel = c - pad_left;
                    if (p_rel < 0)
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, 0, 0,
                                        filter_depth + p_rel,
                                        filter_height + r_rel,
                                        filter_width + c_rel)) /
                                    ((filter_depth + p_rel) *
                                        (filter_height + r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, 0, c_rel,
                                        filter_depth + p_rel,
                                        filter_height + r_rel,
                                        ncolumns - c_rel)) /
                                    ((filter_depth + p_rel) *
                                        (filter_height + r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, 0, c_rel,
                                        filter_depth + p_rel,
                                        filter_height + r_rel, filter_width)) /
                                    ((filter_depth + p_rel) *
                                        (filter_height + r_rel) *
                                        (filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel, 0,
                                        filter_depth + p_rel, nrows - r_rel,
                                        filter_width + c_rel)) /
                                    ((filter_depth + p_rel) * (nrows - r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        nrows - r_rel, ncolumns - c_rel)) /
                                    ((filter_depth + p_rel) * (nrows - r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        nrows - r_rel, filter_width)) /
                                    ((filter_depth + p_rel) * (nrows - r_rel) *
                                        (filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel, 0,
                                        filter_depth + p_rel, filter_height,
                                        filter_width + c_rel)) /
                                    ((filter_depth + p_rel) * filter_height *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        filter_height, ncolumns - c_rel)) /
                                    ((filter_depth + p_rel) * filter_height *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        filter_height, filter_width)) /
                                    ((filter_depth + p_rel) * filter_height *
                                        filter_width);
                            }
                        }
                    }
                    else if (p_rel > static_cast<std::int64_t>(npages) -
                            static_cast<std::int64_t>(filter_depth))
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0, 0,
                                        npages - p_rel, filter_height + r_rel,
                                        filter_width + c_rel)) /
                                    ((npages - p_rel) *
                                        (filter_height + r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0,
                                        c_rel, npages - p_rel,
                                        filter_height + r_rel,
                                        ncolumns - c_rel)) /
                                    ((npages - p_rel) *
                                        (filter_height + r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0,
                                        c_rel, npages - p_rel,
                                        filter_height + r_rel, filter_width)) /
                                    ((npages - p_rel) *
                                        (filter_height + r_rel) * filter_width);
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, npages - p_rel, nrows - r_rel,
                                        filter_width + c_rel)) /
                                    ((npages - p_rel) * (nrows - r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, nrows - r_rel,
                                        ncolumns - c_rel)) /
                                    ((npages - p_rel) * (nrows - r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, nrows - r_rel,
                                        filter_width)) /
                                    ((npages - p_rel) * (nrows - r_rel) *
                                        filter_width);
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, npages - p_rel, filter_height,
                                        filter_width + c_rel)) /
                                    ((npages - p_rel) * filter_height *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, filter_height,
                                        ncolumns - c_rel)) /
                                    ((npages - p_rel) * filter_height *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, filter_height,
                                        filter_width)) /
                                    ((npages - p_rel) * filter_height *
                                        filter_width);
                            }
                        }
                    }
                    else
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0, 0,
                                        filter_depth, filter_height + r_rel,
                                        filter_width + c_rel)) /
                                    (filter_depth * (filter_height + r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(
                                        blaze::subtensor(t, p_rel, 0, c_rel,
                                            filter_depth, filter_height + r_rel,
                                            ncolumns - c_rel)) /
                                    (filter_depth * (filter_height + r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0,
                                        c_rel, filter_depth,
                                        filter_height + r_rel, filter_width)) /
                                    (filter_depth * (filter_height + r_rel) *
                                        filter_width);
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, filter_depth, nrows - r_rel,
                                        filter_width + c_rel)) /
                                    (filter_depth * (nrows - r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, nrows - r_rel,
                                        ncolumns - c_rel)) /
                                    (filter_depth * (nrows - r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, nrows - r_rel,
                                        filter_width)) /
                                    (filter_depth * (nrows - r_rel) *
                                        filter_width);
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, filter_depth, filter_height,
                                        filter_width + c_rel)) /
                                    (filter_depth * filter_height *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, filter_height,
                                        ncolumns - c_rel)) /
                                    (filter_depth * filter_height *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, filter_height,
                                        filter_width)) /
                                    (filter_depth * filter_height *
                                        filter_width);
                            }
                        }
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type pool_operation::max_pool3d_with_pad(
        ir::node_data<T>&& arg, ir::range&& pool_size,
        ir::range&& strides) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_depth = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_height = extract_scalar_integer_value_strict(*++it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        std::size_t pad_front;
        std::size_t pad_top;
        std::size_t pad_left;
        std::size_t pad_depth;
        std::size_t pad_height;
        std::size_t pad_width;

        std::size_t npages = t.pages();
        std::size_t nrows = t.rows();
        std::size_t ncolumns = t.columns();

        if (npages % stride_depth == 0)
            pad_depth = (blaze::max)(
                filter_depth - stride_depth, static_cast<std::size_t>(0));
        else
            pad_depth = (blaze::max)(filter_depth - (npages % stride_depth),
                static_cast<std::size_t>(0));

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

        pad_front = pad_depth / 2;
        pad_top = pad_height / 2;
        pad_left = pad_width / 2;

        std::int64_t p_rel;    //relative page
        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicTensor<T> result(
            blaze::ceil(
                static_cast<float>((npages + pad_depth - filter_depth + 1)) /
                static_cast<float>(stride_depth)),
            blaze::ceil(
                static_cast<float>((nrows + pad_height - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(
                static_cast<float>((ncolumns + pad_width - filter_width + 1)) /
                static_cast<float>(stride_width)));

        for (std::size_t p = 0; p != result.pages(); ++p)
        {
            p_rel = p * stride_depth - pad_front;
            for (std::size_t r = 0; r != result.rows(); ++r)
            {
                r_rel = r * stride_height - pad_top;
                for (std::size_t c = 0; c != result.columns(); ++c)
                {
                    c_rel = c * stride_width - pad_left;
                    if (p_rel < 0)
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, 0, 0, filter_depth + p_rel,
                                    filter_height + r_rel,
                                    filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, 0, c_rel, filter_depth + p_rel,
                                    filter_height + r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, 0, c_rel, filter_depth + p_rel,
                                    filter_height + r_rel, filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, 0, filter_depth + p_rel,
                                    nrows - r_rel, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    nrows - r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    nrows - r_rel, filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, 0, filter_depth + p_rel,
                                    filter_height, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    filter_height, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, 0, r_rel, c_rel, filter_depth + p_rel,
                                    filter_height, filter_width));
                            }
                        }
                    }
                    else if (p_rel > static_cast<std::int64_t>(npages) -
                            static_cast<std::int64_t>(filter_depth))
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(
                                    blaze::subtensor(t, p_rel, 0, 0,
                                        npages - p_rel, filter_height + r_rel,
                                        filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, npages - p_rel,
                                    filter_height + r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, npages - p_rel,
                                    filter_height + r_rel, filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, npages - p_rel,
                                    nrows - r_rel, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    nrows - r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    nrows - r_rel, filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, npages - p_rel,
                                    filter_height, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    filter_height, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, npages - p_rel,
                                    filter_height, filter_width));
                            }
                        }
                    }
                    else
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    (blaze::max)(blaze::subtensor(t, p_rel, 0,
                                        0, filter_depth, filter_height + r_rel,
                                        filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, filter_depth,
                                    filter_height + r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, 0, c_rel, filter_depth,
                                    filter_height + r_rel, filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, filter_depth,
                                    nrows - r_rel, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    nrows - r_rel, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    nrows - r_rel, filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, 0, filter_depth,
                                    filter_height, filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    filter_height, ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) = (blaze::max)(blaze::subtensor(
                                    t, p_rel, r_rel, c_rel, filter_depth,
                                    filter_height, filter_width));
                            }
                        }
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type pool_operation::avg_pool3d_with_pad(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        ir::range&& strides) const
    {
        auto t = arg.tensor();
        auto it = pool_size.begin();
        std::size_t filter_depth = extract_scalar_integer_value_strict(*it);
        std::size_t filter_height = extract_scalar_integer_value_strict(*++it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_depth = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_height = extract_scalar_integer_value_strict(*++it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        std::size_t pad_front;
        std::size_t pad_top;
        std::size_t pad_left;
        std::size_t pad_depth;
        std::size_t pad_height;
        std::size_t pad_width;

        std::size_t npages = t.pages();
        std::size_t nrows = t.rows();
        std::size_t ncolumns = t.columns();

        if (npages % stride_depth == 0)
            pad_depth = (blaze::max)(
                filter_depth - stride_depth, static_cast<std::size_t>(0));
        else
            pad_depth = (blaze::max)(filter_depth - (npages % stride_depth),
                static_cast<std::size_t>(0));

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

        pad_front = pad_depth / 2;
        pad_top = pad_height / 2;
        pad_left = pad_width / 2;

        std::int64_t p_rel;    //relative page
        std::int64_t r_rel;    //relative row
        std::int64_t c_rel;    //relative column

        blaze::DynamicTensor<double> result(
            blaze::ceil(
                static_cast<float>((npages + pad_depth - filter_depth + 1)) /
                static_cast<float>(stride_depth)),
            blaze::ceil(
                static_cast<float>((nrows + pad_height - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(
                static_cast<float>((ncolumns + pad_width - filter_width + 1)) /
                static_cast<float>(stride_width)));

        for (std::size_t p = 0; p != result.pages(); ++p)
        {
            p_rel = p * stride_depth - pad_front;
            for (std::size_t r = 0; r != result.rows(); ++r)
            {
                r_rel = r * stride_height - pad_top;
                for (std::size_t c = 0; c != result.columns(); ++c)
                {
                    c_rel = c * stride_width - pad_left;
                    if (p_rel < 0)
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, 0, 0,
                                        filter_depth + p_rel,
                                        filter_height + r_rel,
                                        filter_width + c_rel)) /
                                    ((filter_depth + p_rel) *
                                        (filter_height + r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, 0, c_rel,
                                        filter_depth + p_rel,
                                        filter_height + r_rel,
                                        ncolumns - c_rel)) /
                                    ((filter_depth + p_rel) *
                                        (filter_height + r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, 0, c_rel,
                                        filter_depth + p_rel,
                                        filter_height + r_rel, filter_width)) /
                                    ((filter_depth + p_rel) *
                                        (filter_height + r_rel) *
                                        (filter_width));
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel, 0,
                                        filter_depth + p_rel, nrows - r_rel,
                                        filter_width + c_rel)) /
                                    ((filter_depth + p_rel) * (nrows - r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        nrows - r_rel, ncolumns - c_rel)) /
                                    ((filter_depth + p_rel) * (nrows - r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        nrows - r_rel, filter_width)) /
                                    ((filter_depth + p_rel) * (nrows - r_rel) *
                                        (filter_width));
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel, 0,
                                        filter_depth + p_rel, filter_height,
                                        filter_width + c_rel)) /
                                    ((filter_depth + p_rel) * filter_height *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        filter_height, ncolumns - c_rel)) /
                                    ((filter_depth + p_rel) * filter_height *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, 0, r_rel,
                                        c_rel, filter_depth + p_rel,
                                        filter_height, filter_width)) /
                                    ((filter_depth + p_rel) * filter_height *
                                        filter_width);
                            }
                        }
                    }
                    else if (p_rel > static_cast<std::int64_t>(npages) -
                            static_cast<std::int64_t>(filter_depth))
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0, 0,
                                        npages - p_rel, filter_height + r_rel,
                                        filter_width + c_rel)) /
                                    ((npages - p_rel) *
                                        (filter_height + r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0,
                                        c_rel, npages - p_rel,
                                        filter_height + r_rel,
                                        ncolumns - c_rel)) /
                                    ((npages - p_rel) *
                                        (filter_height + r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0,
                                        c_rel, npages - p_rel,
                                        filter_height + r_rel, filter_width)) /
                                    ((npages - p_rel) *
                                        (filter_height + r_rel) * filter_width);
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, npages - p_rel, nrows - r_rel,
                                        filter_width + c_rel)) /
                                    ((npages - p_rel) * (nrows - r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, nrows - r_rel,
                                        ncolumns - c_rel)) /
                                    ((npages - p_rel) * (nrows - r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, nrows - r_rel,
                                        filter_width)) /
                                    ((npages - p_rel) * (nrows - r_rel) *
                                        filter_width);
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, npages - p_rel, filter_height,
                                        filter_width + c_rel)) /
                                    ((npages - p_rel) * filter_height *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, filter_height,
                                        ncolumns - c_rel)) /
                                    ((npages - p_rel) * filter_height *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, npages - p_rel, filter_height,
                                        filter_width)) /
                                    ((npages - p_rel) * filter_height *
                                        filter_width);
                            }
                        }
                    }
                    else
                    {
                        if (r_rel < 0)
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0, 0,
                                        filter_depth, filter_height + r_rel,
                                        filter_width + c_rel)) /
                                    (filter_depth * (filter_height + r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(
                                        blaze::subtensor(t, p_rel, 0, c_rel,
                                            filter_depth, filter_height + r_rel,
                                            ncolumns - c_rel)) /
                                    (filter_depth * (filter_height + r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, 0,
                                        c_rel, filter_depth,
                                        filter_height + r_rel, filter_width)) /
                                    (filter_depth * (filter_height + r_rel) *
                                        filter_width);
                            }
                        }
                        else if (r_rel > static_cast<std::int64_t>(nrows) -
                                static_cast<std::int64_t>(filter_height))
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, filter_depth, nrows - r_rel,
                                        filter_width + c_rel)) /
                                    (filter_depth * (nrows - r_rel) *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, nrows - r_rel,
                                        ncolumns - c_rel)) /
                                    (filter_depth * (nrows - r_rel) *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, nrows - r_rel,
                                        filter_width)) /
                                    (filter_depth * (nrows - r_rel) *
                                        filter_width);
                            }
                        }
                        else
                        {
                            if (c_rel < 0)
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        0, filter_depth, filter_height,
                                        filter_width + c_rel)) /
                                    (filter_depth * filter_height *
                                        (filter_width + c_rel));
                            }
                            else if (c_rel >
                                static_cast<std::int64_t>(ncolumns) -
                                    static_cast<std::int64_t>(filter_width))
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, filter_height,
                                        ncolumns - c_rel)) /
                                    (filter_depth * filter_height *
                                        (ncolumns - c_rel));
                            }
                            else
                            {
                                result(p, r, c) =
                                    blaze::sum(blaze::subtensor(t, p_rel, r_rel,
                                        c_rel, filter_depth, filter_height,
                                        filter_width)) /
                                    (filter_depth * filter_height *
                                        filter_width);
                            }
                        }
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type pool_operation::max_pool_nd(ir::node_data<T>&& arg,
        ir::range&& pool_size, std::string&& padding) const
    {
        if (padding == "valid")
        {
            if (arg.num_dimensions() == 2)
                return max_pool2d(std::move(arg), std::move(pool_size));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            //arg.num_dimensions == 3
            return max_pool3d(std::move(arg), std::move(pool_size));
#endif
        }
        else if (padding == "same")
        {
            if (arg.num_dimensions() == 2)
                return max_pool2d_with_pad(std::move(arg), std::move(pool_size));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            // arg.num_dimensions == 3
            return max_pool3d_with_pad(std::move(arg), std::move(pool_size));
#endif
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "pool_operation::max_pool_nd",
            generate_error_message("invalid number of dimensions"));
    }

    primitive_argument_type pool_operation::avg_pool_nd(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        std::string&& padding) const
    {
        if (padding == "valid")
        {
            if (arg.num_dimensions() == 2)
                return avg_pool2d(std::move(arg), std::move(pool_size));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            // arg.num_dimensions == 3
            return avg_pool3d(std::move(arg), std::move(pool_size));
#endif
        }
        else if (padding == "same")
        {
            if (arg.num_dimensions() == 2)
                return avg_pool2d_with_pad(
                    std::move(arg), std::move(pool_size));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            // arg.num_dimensions == 3
            return avg_pool3d_with_pad(std::move(arg), std::move(pool_size));
#endif
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "pool_operation::avg_pool_nd",
            generate_error_message("invalid number of dimensions"));
    }

    template <typename T>
    primitive_argument_type pool_operation::max_pool_nd(ir::node_data<T>&& arg,
        ir::range&& pool_size, std::string&& padding, ir::range&& strides) const
    {
        if (padding == "valid")
        {
            if (arg.num_dimensions() == 2)
                return max_pool2d(
                    std::move(arg), std::move(pool_size), std::move(strides));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            //arg.num_dimensions == 3
            return max_pool3d(
                std::move(arg), std::move(pool_size), std::move(strides));
#endif
        }
        else if (padding == "same")
        {
            if (arg.num_dimensions() == 2)
                return max_pool2d_with_pad(
                    std::move(arg), std::move(pool_size), std::move(strides));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            // arg.num_dimensions == 3
            return max_pool3d_with_pad(
                std::move(arg), std::move(pool_size), std::move(strides));
#endif
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "pool_operation::max_pool_nd",
            generate_error_message("invalid number of dimensions"));
    }

    primitive_argument_type pool_operation::avg_pool_nd(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        std::string&& padding, ir::range&& strides) const
    {
        if (padding == "valid")
        {
            if (arg.num_dimensions() == 2)
                return avg_pool2d(
                    std::move(arg), std::move(pool_size), std::move(strides));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            //arg.num_dimensions == 3
            return avg_pool3d(
                std::move(arg), std::move(pool_size), std::move(strides));
#endif
        }
        else if (padding == "same")
        {
            if (arg.num_dimensions() == 2)
                return avg_pool2d_with_pad(
                    std::move(arg), std::move(pool_size), std::move(strides));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
            // arg.num_dimensions == 3
            return avg_pool3d_with_pad(
                std::move(arg), std::move(pool_size), std::move(strides));
#endif
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "pool_operation::avg_pool_nd",
            generate_error_message("invalid number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> pool_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "pool_operation::eval",
                generate_error_message("the pool_operation primitive requires "
                                       "between 2 and 4 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "pool_operation::eval",
                    generate_error_message(
                        "the pool_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {

                std::size_t ndim = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                ir::range pool_size = extract_list_value_strict(
                    args[1], this_->name_, this_->codename_);

                std::string padding = "valid";
                if (args.size() > 2)
                {
                    padding = extract_string_value(
                        args[2], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same")
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "pool_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. padding can be either valid "
                                "or same"));
                }

                if (!this_->validate_pooling(ndim, pool_size))
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "pool_operation::eval",
                        this_->generate_error_message(
                            "invalid combination of arguments for pooling"));

                ir::range strides(0); // an empty range
                if (args.size() == 4)
                {
                    strides = extract_list_value_strict(
                        args[3], this_->name_, this_->codename_);

                    if (!this_->validate_strides(ndim, strides))
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "pool_operation::eval",
                            this_->generate_error_message(
                                "invalid strides. padding can be either valid "
                                "or same"));
                }

                if (strides.size() == 0) // strides contain only 1s
                {
                    if (this_->mode_ == max_pool)
                    {
                        switch (extract_common_type(args[0]))
                        {
                        case node_data_type_bool:
                            return this_->max_pool_nd(
                                extract_boolean_value(std::move(args[0]),
                                    this_->name_, this_->codename_),
                                std::move(pool_size), std::move(padding));
                        case node_data_type_int64:
                            return this_->max_pool_nd(
                                extract_integer_value(std::move(args[0]),
                                    this_->name_, this_->codename_),
                                std::move(pool_size), std::move(padding));
                        case node_data_type_double:
                            return this_->max_pool_nd(
                                extract_numeric_value(std::move(args[0]),
                                    this_->name_, this_->codename_),
                                std::move(pool_size), std::move(padding));
                        default:
                            break;
                        }
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "pool_operation::eval",
                            this_->generate_error_message(
                                "the pool primitive requires for all arguments "
                                "to be numeric data types"));
                    }
                    else if(this_->mode_ == avg_pool)
                        return this_->avg_pool_nd(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(pool_size), std::move(padding));

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "pool_operation::eval",
                        this_->generate_error_message(
                            "unsupported mode requested"));
                }
                // non-default strides
                if (this_->mode_ == max_pool)
                    {
                        switch (extract_common_type(args[0]))
                        {
                        case node_data_type_bool:
                            return this_->max_pool_nd(
                                extract_boolean_value(std::move(args[0]),
                                    this_->name_, this_->codename_),
                                std::move(pool_size), std::move(padding),
                                std::move(strides));
                        case node_data_type_int64:
                            return this_->max_pool_nd(
                                extract_integer_value(std::move(args[0]),
                                    this_->name_, this_->codename_),
                                std::move(pool_size), std::move(padding),
                                std::move(strides));
                        case node_data_type_double:
                            return this_->max_pool_nd(
                                extract_numeric_value(std::move(args[0]),
                                    this_->name_, this_->codename_),
                                std::move(pool_size), std::move(padding),
                                std::move(strides));
                        default:
                            break;
                        }
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "pool_operation::eval",
                            this_->generate_error_message(
                                "the pool primitive requires for all arguments "
                                "to be numeric data types"));
                    }
                    else if(this_->mode_ == avg_pool)
                        return this_->avg_pool_nd(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            std::move(pool_size), std::move(padding),
                            std::move(strides));

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "pool_operation::eval",
                        this_->generate_error_message(
                            "unsupported mode requested"));

            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
