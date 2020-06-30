// Copyright (c) 2019-2020 Bita Hasheminezhad
// Copyright (c) 2019-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/common/conv1d_all_paddings.hpp>
#include <phylanx/plugins/keras_support/conv_indices_helper.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type conv1d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        std::size_t batch = a.pages();
        std::size_t filter_length = k.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::size_t result_length = a.rows() - filter_length + 1;

        blaze::DynamicTensor<double> result(batch, result_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != result_length; ++i)
            {
                auto schur_product = blaze::subtensor(a, 0, i, 0, batch,
                                         filter_length, in_channels) %
                    kslice;
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    execution_tree::primitive_argument_type conv1d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        std::size_t batch = a.pages();
        std::size_t filter_length = k.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::size_t result_length = blaze::ceil(
            static_cast<double>(a.rows() - filter_length + 1) / strides);

        blaze::DynamicTensor<double> result(batch, result_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != result_length; ++i)
            {
                auto schur_product = blaze::subtensor(a, 0, i * strides, 0,
                                         batch, filter_length, in_channels) %
                    kslice;
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    execution_tree::primitive_argument_type conv1d_valid_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t dilation_rate)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();

        std::int64_t result_length =
            data_length - dilation_rate * (filter_length - 1);

        if(result_length <= 0)
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "conv1d_valid_dilation",
                util::generate_error_message(
                    "this dilation_rate causes non-positive "
                    "result_length where padding is valid"));

        blaze::DynamicTensor<double> result(batch, result_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != result_length; ++i)
            {
                auto schur_product =
                    blaze::dilatedsubtensor(a, 0, i, 0, batch, filter_length,
                        in_channels, 1, dilation_rate, 1) %
                    kslice;
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type conv1d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length   = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();

        std::int64_t pad_top = (filter_length - 1) / 2;

        blaze::DynamicTensor<double> result(batch, data_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != data_length; ++i)
            {
                auto sub = conv_indices::get_subsizes(
                    data_length, filter_length, i - pad_top);
                auto schur_product = blaze::subtensor(a, 0, sub.image_beg_, 0,
                                         batch, sub.size_, in_channels) %
                    blaze::submatrix(
                        kslice, sub.kernel_beg_, 0, sub.size_, in_channels);
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    execution_tree::primitive_argument_type conv1d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length   = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::int64_t pad_width;

        if (data_length % strides == 0)
        {
            pad_width = filter_length > strides ? filter_length - strides :
                                                  static_cast<std::int64_t>(0);
        }
        else
        {
            pad_width = filter_length > (data_length % strides) ?
                filter_length - (data_length % strides) :
                static_cast<std::int64_t>(0);
        }

        std::size_t result_length = blaze::ceil(
            static_cast<double>(data_length + pad_width - filter_length + 1) /
            strides);

        blaze::DynamicTensor<double> result(batch, result_length, out_channels);
        std::size_t pad_top = pad_width / 2;

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != result_length; ++i)
            {
                auto sub = conv_indices::get_subsizes(
                    data_length, filter_length, i * strides - pad_top);
                auto schur_product = blaze::subtensor(a, 0, sub.image_beg_, 0,
                                         batch, sub.size_, in_channels) %
                    blaze::submatrix(
                        kslice, sub.kernel_beg_, 0, sub.size_, in_channels);
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    execution_tree::primitive_argument_type conv1d_same_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t dilation_rate)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length   = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::int64_t pad_top = (dilation_rate * (filter_length - 1)) / 2;

        blaze::DynamicTensor<double> result(
            batch, data_length, out_channels, 0.);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != data_length; ++i)
            {
                auto sub = conv_indices::get_subsizes_dilated(
                    data_length, filter_length, i - pad_top, dilation_rate);

                if (sub.size_ == 0)
                    continue;

                auto schur_product =
                    blaze::dilatedsubtensor(a, 0, sub.image_beg_, 0, batch,
                        sub.size_, in_channels, 1, dilation_rate, 1) %
                    blaze::submatrix(
                        kslice, sub.kernel_beg_, 0, sub.size_, in_channels);
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type conv1d_causal(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length   = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::int64_t pad_top = filter_length - 1;    // no pad_bottom

        blaze::DynamicTensor<double> result(batch, data_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != data_length; ++i)
            {
                auto sub = conv_indices::get_subsizes_causal(
                    filter_length, i - pad_top);
                auto schur_product = blaze::subtensor(a, 0, sub.image_beg_, 0,
                                         batch, sub.size_, in_channels) %
                    blaze::submatrix(
                        kslice, sub.kernel_beg_, 0, sub.size_, in_channels);
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    execution_tree::primitive_argument_type conv1d_causal(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length   = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::int64_t pad_top = filter_length - 1;    // no pad_bottom

        std::size_t result_length =
            blaze::ceil(static_cast<double>(data_length) / strides);

        blaze::DynamicTensor<double> result(batch, result_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != result_length; ++i)
            {
                auto sub = conv_indices::get_subsizes_causal(
                    filter_length, i * strides - pad_top);
                auto schur_product = blaze::subtensor(a, 0, sub.image_beg_, 0,
                                         batch, sub.size_, in_channels) %
                    blaze::submatrix(
                        kslice, sub.kernel_beg_, 0, sub.size_, in_channels);
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    execution_tree::primitive_argument_type conv1d_causal_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t dilation_rate)
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length   = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();
        std::int64_t pad_top =
            dilation_rate * (filter_length - 1);    // no pad_bottom

        blaze::DynamicTensor<double> result(batch, data_length, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto kslice = blaze::columnslice(k, c);
            for (std::size_t i = 0; i != data_length; ++i)
            {
                auto sub = conv_indices::get_subsizes_causal_dilated(
                    filter_length, i - pad_top, dilation_rate);

                if (sub.size_ == 0)
                    continue;

                auto schur_product =
                    blaze::dilatedsubtensor(a, 0, sub.image_beg_, 0, batch,
                        sub.size_, in_channels, 1, dilation_rate, 1) %
                    blaze::submatrix(
                        kslice, sub.kernel_beg_, 0, sub.size_, in_channels);
                for (std::size_t p = 0; p != batch; ++p)
                {
                    auto pslice = blaze::pageslice(schur_product, p);
                    result(p, i, c) = blaze::sum(pslice);
                }
            }
        }
        return execution_tree::primitive_argument_type{std::move(result)};
    }

    /////////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type conv1d_all_paddings(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::string const& name,
        std::string const& codename)
    {
        if (execution_tree::extract_numeric_value_dimensions(
            arg, name, codename)[2] !=
            execution_tree::extract_numeric_value_dimensions(
                kernel, name, codename)[1])
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv1d_all_paddings::conv1d_all_paddings",
                util::generate_error_message(
                    "input depth must be evenly divisible by filter depth. "
                    "Number of input channels is not the same",
                    name, codename));
        }

        if (padding == "valid")
        {
            return conv1d_valid(std::move(arg), std::move(kernel));
        }
        if (padding == "same")
        {
            return conv1d_same(std::move(arg), std::move(kernel));
        }

        // padding == causal
        return conv1d_causal(std::move(arg), std::move(kernel));
    }

    execution_tree::primitive_argument_type conv1d_all_paddings(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::int64_t strides, std::string const& name,
        std::string const& codename)
    {
        if (execution_tree::extract_numeric_value_dimensions(
                arg, name, codename)[2] !=
            execution_tree::extract_numeric_value_dimensions(
                kernel, name, codename)[1])
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv1d_all_paddings::conv1d_all_paddings",
                util::generate_error_message(
                    "input depth must be evenly divisible by filter depth. "
                    "Number of input channels is not the same",
                    name, codename));
        }

        if (padding == "valid")
        {
            return conv1d_valid(std::move(arg), std::move(kernel), strides);
        }
        if (padding == "same")
        {
            return conv1d_same(std::move(arg), std::move(kernel), strides);
        }

        // padding == causal
        return conv1d_causal(std::move(arg), std::move(kernel), strides);
    }

    execution_tree::primitive_argument_type conv1d_all_paddings_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::int64_t dilation_rate,
        std::string const& name, std::string const& codename)
    {
        if (execution_tree::extract_numeric_value_dimensions(
                arg, name, codename)[2] !=
            execution_tree::extract_numeric_value_dimensions(
                kernel, name, codename)[1])
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv1d_all_paddings::conv1d_all_paddings",
                util::generate_error_message(
                    "input depth must be evenly divisible by filter depth. "
                    "Number of input channels is not the same",
                    name, codename));
        }

        if (padding == "valid")
        {
            return conv1d_valid_dilation(
                std::move(arg), std::move(kernel), dilation_rate);
        }
        if (padding == "same")
        {
            return conv1d_same_dilation(
                std::move(arg), std::move(kernel), dilation_rate);
        }

        // padding == causal
        return conv1d_causal_dilation(
            std::move(arg), std::move(kernel), dilation_rate);
    }
}}
