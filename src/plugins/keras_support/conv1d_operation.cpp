// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/conv1d_operation.hpp>

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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const conv1d_operation::match_data =
    {
        hpx::util::make_tuple("conv1d",
        std::vector<std::string>{R"(
            conv1d(_1, _2_kernel,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, 1),
            __arg(_5_dilation_rate, 1))
        )"},
        &create_conv1d_operation, &create_primitive<conv1d_operation>,
        R"(x, kernel, padding, strides, dilation_rate
        Args:

            x (array) : a 3d array consiting of batch, i_length and in_channels
                dimensions.
            kernel (array) : a 3d array consisting of filter_length,
                in_channels and out_channels dimension. Note that the
                in_channels should be the same as the original array
            padding (optional, string) : padding mode, `valid` by default. It
                can be either `valid`, `same` or `causal`. `vaild` means no
                padding. `same` results the output with the same shape as
                original array in case of unit strides. `causal` zero pads the
                array in a way that no output element depend on the input
                elements of its future
            strides (optional, integer) : the step to apply convolution over
                array. It sets to 1 by default.
            dilation_rate (optional, integer) : indicates the dilation rate,
                the rate to sample the array in each step of convolution, 1
                by default.

        Returns:

        1D convolution (or 1D mathematical cross-correlation))")
    };

    ///////////////////////////////////////////////////////////////////////////
    conv1d_operation::conv1d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Tensor, typename Matrix>
    double conv1d_operation::convolve_step(Tensor const& t, Matrix const& m)
    {
        size_t t_pages = t.pages();
        for (std::size_t p = 0; p != t_pages; ++p)
        {
            auto pslice = blaze::pageslice()
        }
        return blaze::sum(t % m);
    }

    //template <typename Vector1, typename Vector2>
    //double conv1d_operation::convolve_step(Vector1 const& v1, Vector2 const& v2,
    //    std::int64_t dilation_rate, std::size_t kernel_size, std::size_t offset)
    //{
    //    auto dilated_v1 = blaze::elements(v1,
    //        [dilation_rate, offset](std::size_t i) {
    //            return i * dilation_rate + offset;
    //        },
    //        kernel_size);
    //    return blaze::sum(dilated_v1 * v2);
    //}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv1d_operation::conv1d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel) const
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
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv1d_operation::conv1d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides) const
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
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv1d_operation::conv1d_valid_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t dilation_rate) const
    {
        auto a = arg.tensor();
        auto k = kernel.tensor();
        auto filter_length = static_cast<std::int64_t>(k.pages());
        auto data_length = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t out_channels = k.columns();

        std::int64_t dilated_k_size = dilation_rate * (filter_length - 1) + 1;
        std::int64_t result_length = (blaze::max)(
            data_length - dilated_k_size + 1, static_cast<std::int64_t>(0));

        blaze::DynamicTensor<double> result(batch, result_length, out_channels);
        //for (std::size_t i = 0; i != result_size; ++i)
        //{
        //    result[i] = convolve_step(blaze::subvector(v, i, dilated_k_size), k,
        //        dilation_rate, k_size);
        //}
        return primitive_argument_type{std::move(result)};
    }

    /////////////////////////////////////////////////////////////////////////////
    //namespace detail
    //{
    //    ///////////////////////////////////////////////////////////////////////
    //    struct sizes
    //    {
    //        std::int64_t position_;
    //        std::int64_t size_;
    //        std::int64_t offset_ = 0;
    //    };

    //    ///////////////////////////////////////////////////////////////////////
    //    inline sizes image_subsizes_helper(std::int64_t image_size,
    //        std::int64_t kernel_size, std::int64_t relative_position)
    //    {
    //        if (relative_position < 0)
    //        {
    //            return sizes{0, kernel_size + relative_position};
    //        }

    //        if (relative_position > image_size - kernel_size)
    //        {
    //            return sizes{relative_position, image_size - relative_position};
    //        }

    //        return sizes{relative_position, kernel_size};
    //    }

    //    inline sizes image_subsizes(std::int64_t position,
    //        std::int64_t image_size, std::int64_t kernel_size,
    //        std::int64_t pad_left)
    //    {
    //        return image_subsizes_helper(
    //            image_size, kernel_size, position - pad_left);
    //    }

    //    inline sizes image_subsizes_strided(std::int64_t position,
    //        std::int64_t image_size, std::int64_t kernel_size,
    //        std::int64_t pad_left, std::int64_t stride)
    //    {
    //        return image_subsizes_helper(
    //            image_size, kernel_size, position * stride - pad_left);
    //    }

    //    inline sizes image_subsizes_dilated(std::int64_t position,
    //        std::int64_t image_size, std::int64_t dilated_kernel_size,
    //        std::int64_t pad_left)
    //    {
    //        std::int64_t relative_position = position - pad_left;

    //        if (relative_position < 0)
    //        {
    //            return sizes{0, (blaze::min)(
    //                image_size, relative_position + dilated_kernel_size)};
    //        }

    //        if (relative_position > image_size - dilated_kernel_size)
    //        {
    //            return sizes{relative_position, image_size - relative_position};
    //        }

    //        return sizes{relative_position, dilated_kernel_size};
    //    }

    //    ///////////////////////////////////////////////////////////////////////
    //    inline sizes image_subsizes_causal_helper(std::int64_t kernel_size,
    //        std::int64_t relative_position)
    //    {
    //        if (relative_position < 0)
    //        {
    //            return sizes{0, kernel_size + relative_position};
    //        }

    //        return sizes{relative_position, kernel_size};
    //    }

    //    inline sizes image_subsizes_causal(std::int64_t position,
    //        std::int64_t kernel_size, std::int64_t pad_left)
    //    {
    //        return image_subsizes_causal_helper(
    //            kernel_size, position - pad_left);
    //    }

    //    inline sizes image_subsizes_causal_strided(std::int64_t position,
    //        std::int64_t kernel_size, std::int64_t pad_left,
    //        std::int64_t strides)
    //    {
    //        return image_subsizes_causal_helper(
    //            kernel_size, position * strides - pad_left);
    //    }

    //    inline sizes image_subsizes_causal_dilated(std::int64_t position,
    //        std::int64_t image_size, std::int64_t kernel_size,
    //        std::int64_t pad_left, std::int64_t dilated_kernel_size,
    //        std::int64_t dilation_rate)
    //    {
    //        std::int64_t relative_position = position - pad_left;

    //        if (relative_position < 0)
    //        {
    //            return sizes{0, (blaze::min)(
    //                image_size, relative_position + dilated_kernel_size)};
    //        }

    //        return sizes{relative_position, dilated_kernel_size};
    //    }

    //    ///////////////////////////////////////////////////////////////////////
    //    sizes kernel_subsizes_helper(std::int64_t image_size,
    //        std::int64_t kernel_size, std::int64_t relative_position)
    //    {
    //        if (relative_position < 0)
    //        {
    //            return sizes{
    //                -relative_position, kernel_size + relative_position};
    //        }

    //        if (relative_position > image_size - kernel_size)
    //        {
    //            return sizes{0, image_size - relative_position};
    //        }

    //        return sizes{0, kernel_size};
    //    }

    //    sizes kernel_subsizes(std::int64_t position, std::int64_t image_size,
    //        std::int64_t kernel_size, std::int64_t pad_left)
    //    {
    //        return kernel_subsizes_helper(
    //            image_size, kernel_size, position - pad_left);
    //    }

    //    sizes kernel_subsizes_strided(std::int64_t position,
    //        std::int64_t image_size, std::int64_t kernel_size,
    //        std::int64_t pad_left, std::int64_t stride)
    //    {
    //        return kernel_subsizes_helper(
    //            image_size, kernel_size, position * stride - pad_left);
    //    }

    //    sizes kernel_subsizes_dilated(std::int64_t position,
    //        std::int64_t image_size, std::int64_t kernel_size,
    //        std::int64_t pad_left, std::int64_t dilated_kernel_size,
    //        std::int64_t dilation_rate)
    //    {
    //        std::int64_t relative_position = position - pad_left;
    //        if (relative_position < 0)
    //        {
    //            std::int64_t remainder = (-relative_position) % dilation_rate;

    //            std::int64_t corrected_position = blaze::ceil(
    //                static_cast<double>(-relative_position) / dilation_rate);
    //            std::int64_t corrected_kernel_size = 0;

    //            if (dilated_kernel_size + relative_position > image_size)
    //            {
    //                corrected_kernel_size = blaze::ceil(
    //                    static_cast<double>(image_size - remainder) /
    //                    dilation_rate);
    //            }
    //            else
    //            {
    //                corrected_kernel_size = blaze::ceil(
    //                    static_cast<double>(
    //                        dilated_kernel_size + relative_position) /
    //                    dilation_rate);
    //            }

    //            if (remainder == 0)
    //            {
    //                return sizes{corrected_position, corrected_kernel_size};
    //            }

    //            return sizes{corrected_position, corrected_kernel_size,
    //                dilation_rate - remainder};
    //        }

    //        if (relative_position > image_size - dilated_kernel_size)
    //        {
    //            std::int64_t corrected_kernel_size = blaze::ceil(
    //                static_cast<double>(image_size - relative_position) /
    //                dilation_rate);

    //            return sizes{0, corrected_kernel_size};
    //        }

    //        return sizes{0, kernel_size};
    //    }

    //    ///////////////////////////////////////////////////////////////////////
    //    sizes kernel_subsizes_causal_helper(std::int64_t kernel_size,
    //        std::int64_t relative_position)
    //    {
    //        if (relative_position < 0)
    //        {
    //            return sizes{
    //                -relative_position, kernel_size + relative_position};
    //        }
    //        return sizes{0, kernel_size};
    //    }

    //    sizes kernel_subsizes_causal(std::int64_t position,
    //        std::int64_t kernel_size, std::int64_t pad_left)
    //    {
    //        return kernel_subsizes_causal_helper(
    //            kernel_size, position - pad_left);
    //    }

    //    sizes kernel_subsizes_causal_strided(std::int64_t position,
    //        std::int64_t kernel_size, std::int64_t pad_left,
    //        std::int64_t strides)
    //    {
    //        return kernel_subsizes_causal_helper(
    //            kernel_size, position *strides - pad_left);
    //    }

    //    inline sizes kernel_subsizes_causal_dilated(std::int64_t position,
    //        std::int64_t kernel_size, std::int64_t pad_left,
    //        std::int64_t dilated_kernel_size, std::int64_t dilation_rate)
    //    {
    //        std::int64_t relative_position = position - pad_left;

    //        if (relative_position < 0)
    //        {
    //            std::int64_t corrected_kernel_size = blaze::ceil(
    //                static_cast<double>(
    //                    dilated_kernel_size + relative_position) /
    //                dilation_rate);
    //            std::int64_t corrected_position = blaze::ceil(
    //                static_cast<double>(-relative_position) / dilation_rate);

    //            std::int64_t remainder = (-relative_position) % dilation_rate;
    //            if (remainder == 0)
    //            {
    //                return sizes{corrected_position, corrected_kernel_size};
    //            }

    //            return sizes{corrected_position, corrected_kernel_size,
    //                dilation_rate - remainder};
    //        }

    //        return sizes{0, kernel_size};
    //    }
    //}

    ///////////////////////////////////////////////////////////////////////////////
    //primitive_argument_type conv1d_operation::conv1d_same(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    auto k_size = static_cast<std::int64_t>(kernel.size());
    //    auto v_size = static_cast<std::int64_t>(arg.size());
    //    std::int64_t pad_left = (k_size - 1) / 2;

    //    blaze::DynamicVector<double> result(v_size);
    //    for (std::size_t i = 0; i != v_size; ++i)
    //    {
    //        auto v_pos = detail::image_subsizes(i, v_size, k_size, pad_left);
    //        auto k_pos = detail::kernel_subsizes(i, v_size, k_size, pad_left);

    //        result[i] = convolve_step(
    //            blaze::subvector(v, v_pos.position_, v_pos.size_),
    //            blaze::subvector(k, k_pos.position_, k_pos.size_));
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    //primitive_argument_type conv1d_operation::conv1d_same(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t strides) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    auto k_size = static_cast<std::int64_t>(kernel.size());
    //    auto v_size = static_cast<std::int64_t>(arg.size());
    //    std::int64_t pad_width;

    //    if (v_size % strides == 0)
    //    {
    //        pad_width =
    //            (blaze::max)(k_size - strides, static_cast<std::int64_t>(0));
    //    }
    //    else
    //    {
    //        pad_width = (blaze::max)(
    //            k_size - (v_size % strides), static_cast<std::int64_t>(0));
    //    }

    //    std::size_t result_size = blaze::ceil(
    //        static_cast<double>(v_size + pad_width - k_size + 1) / strides);

    //    blaze::DynamicVector<double> result(result_size);
    //    std::size_t pad_left = pad_width / 2;
    //    for (std::size_t i = 0; i != result_size; ++i)
    //    {
    //        auto v_pos = detail::image_subsizes_strided(
    //            i, v_size, k_size, pad_left, strides);
    //        auto k_pos = detail::kernel_subsizes_strided(
    //            i, v_size, k_size, pad_left, strides);

    //        result[i] = convolve_step(
    //            blaze::subvector(v, v_pos.position_, v_pos.size_),
    //            blaze::subvector(k, k_pos.position_, k_pos.size_));
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    //primitive_argument_type conv1d_operation::conv1d_same_dilation(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t dilation_rate) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    auto k_size = static_cast<std::int64_t>(kernel.size());
    //    auto v_size = static_cast<std::int64_t>(arg.size());
    //    std::int64_t dilated_k_size = dilation_rate * (k_size - 1) + 1;
    //    std::int64_t pad_left = (dilated_k_size - 1) / 2;

    //    blaze::DynamicVector<double> result(v_size);
    //    for (std::size_t i = 0; i != v_size; ++i)
    //    {
    //        auto v_pos = detail::image_subsizes_dilated(
    //            i, v_size, dilated_k_size, pad_left);
    //        auto k_pos = detail::kernel_subsizes_dilated(
    //            i, v_size, k_size, pad_left, dilated_k_size, dilation_rate);

    //        result[i] = convolve_step(
    //            blaze::subvector(v, v_pos.position_, v_pos.size_),
    //            blaze::subvector(k, k_pos.position_, k_pos.size_),
    //            dilation_rate, k_pos.size_, k_pos.offset_);
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    /////////////////////////////////////////////////////////////////////////////
    //primitive_argument_type conv1d_operation::conv1d_causal(
    //    ir::node_data<double>&& arg,
    //    ir::node_data<double>&& kernel) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    auto k_size = static_cast<std::int64_t>(kernel.size());
    //    auto v_size = static_cast<std::int64_t>(arg.size());
    //    std::int64_t pad_left = k_size - 1; // no pad_right

    //    blaze::DynamicVector<double> result(v_size);
    //    for (std::size_t i = 0; i != v_size; ++i)
    //    {
    //        auto v_pos = detail::image_subsizes_causal(i, k_size, pad_left);
    //        auto k_pos = detail::kernel_subsizes_causal(i, k_size, pad_left);

    //        result[i] = convolve_step(
    //            blaze::subvector(v, v_pos.position_, v_pos.size_),
    //            blaze::subvector(k, k_pos.position_, k_pos.size_));
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    //primitive_argument_type conv1d_operation::conv1d_causal(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t strides) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    auto k_size = static_cast<std::int64_t>(kernel.size());
    //    auto v_size = static_cast<std::int64_t>(arg.size());
    //    std::int64_t pad_left = k_size - 1; // no pad_right, stride has no effect

    //    std::size_t result_size = blaze::ceil(
    //        static_cast<double>(v_size + pad_left - k_size + 1) / strides);

    //    blaze::DynamicVector<double> result(result_size);
    //    for (std::size_t i = 0; i != result_size; ++i)
    //    {
    //        auto v_pos = detail::image_subsizes_causal_strided(
    //            i, k_size, pad_left, strides);
    //        auto k_pos = detail::kernel_subsizes_causal_strided(
    //            i, k_size, pad_left, strides);

    //        result[i] = convolve_step(
    //            blaze::subvector(v, v_pos.position_, v_pos.size_),
    //            blaze::subvector(k, k_pos.position_, k_pos.size_));
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    //primitive_argument_type conv1d_operation::conv1d_causal_dilation(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t dilation_rate) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    auto k_size = static_cast<std::int64_t>(kernel.size());
    //    auto v_size = static_cast<std::int64_t>(arg.size());
    //    std::int64_t dilated_k_size = dilation_rate * (k_size - 1) + 1;
    //    std::int64_t pad_left = dilated_k_size - 1; // no pad_right

    //    blaze::DynamicVector<double> result(v_size);
    //    for (std::size_t i = 0; i != v_size; ++i)
    //    {
    //        auto v_pos = detail::image_subsizes_causal_dilated(
    //            i, v_size, k_size, pad_left, dilated_k_size, dilation_rate);
    //        auto k_pos = detail::kernel_subsizes_causal_dilated(
    //            i, k_size, pad_left, dilated_k_size, dilation_rate);

    //        result[i] = convolve_step(
    //            blaze::subvector(v, v_pos.position_, v_pos.size_),
    //            blaze::subvector(k, k_pos.position_, k_pos.size_),
    //            dilation_rate, k_pos.size_, k_pos.offset_);
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv1d_operation::conv1d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding) const
    {
        if (padding == "valid")
        {
            return conv1d_valid(std::move(arg), std::move(kernel));
        }
        //if (padding == "same")
        //{
        //    return conv1d_same(std::move(arg), std::move(kernel));
        //}

        //// padding == causal
        //return conv1d_causal(std::move(arg), std::move(kernel));
    }

    primitive_argument_type conv1d_operation::conv1d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::int64_t strides) const
    {
        if (padding == "valid")
        {
            return conv1d_valid(std::move(arg), std::move(kernel), strides);
        }
    //    if (padding == "same")
    //    {
    //        return conv1d_same(std::move(arg), std::move(kernel), strides);
    //    }

    //    // padding == causal
    //    return conv1d_causal(std::move(arg), std::move(kernel), strides);
    }

    primitive_argument_type conv1d_operation::conv1d_any_pad_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::int64_t dilation_rate) const
    {
        if (padding == "valid")
        {
            return conv1d_valid_dilation(
                std::move(arg), std::move(kernel), dilation_rate);
        }
    //    if (padding == "same")
    //    {
    //        return conv1d_same_dilation(
    //            std::move(arg), std::move(kernel), dilation_rate);
    //    }

    //    // padding == causal
    //    return conv1d_causal_dilation(
    //        std::move(arg), std::move(kernel), dilation_rate);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> conv1d_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv1d_operation::eval",
                generate_error_message("the conv1d_operation primitive requires "
                                       "between 2 and 5 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "conv1d_operation::eval",
                    generate_error_message(
                        "the conv1d_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::size_t ndim = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (ndim !=
                        extract_numeric_value_dimension(
                            args[1], this_->name_, this_->codename_) ||
                    ndim != 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv1d_operation::eval",
                        this_->generate_error_message(
                            "conv1d operation requires for x and kernel to be "
                            "tensors"));
                }

                std::string padding = "valid";
                padding = extract_string_value(
                    args[2], this_->name_, this_->codename_);

                if (padding != "valid" && padding != "same" &&
                    padding != "causal")
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv1d_operation::eval",
                        this_->generate_error_message(
                            "invalid padding. Padding can be either "
                            "'valid', 'same', or 'causal'"));
                }


                if (padding == "valid")
                {
                    if (extract_numeric_value_dimension(
                            args[0], this_->name_, this_->codename_) <
                        extract_numeric_value_dimension(
                            args[1], this_->name_, this_->codename_))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "the kernel size cannot be greater than the "
                                "array size in the valid padding mode"));
                    }
                }

                std::int64_t strides = 1;
                if (is_list_operand_strict(args[3]))
                {
                    ir::range s = extract_list_value(
                        args[3], this_->name_, this_->codename_);
                    if (s.size() == 1)
                    {
                        strides = extract_scalar_positive_integer_value_strict(
                            *s.begin());
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "conv1d_operation requires the strides to "
                                "be of rank 1"));
                    }
                }
                else
                {
                    strides = extract_scalar_positive_integer_value_strict(
                        args[3], this_->name_, this_->codename_);
                }

                std::int64_t dilation_rate = 1;
                if (is_list_operand_strict(args[4]))
                {
                    ir::range d = extract_list_value(
                        args[4], this_->name_, this_->codename_);
                    if (d.size() == 1)
                    {
                        dilation_rate =
                            extract_scalar_positive_integer_value_strict(
                                *d.begin());
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "conv1d_operation requires the "
                                "dilation_rate to be of rank 1"));
                    }
                }
                else
                {
                    dilation_rate =
                        extract_scalar_positive_integer_value_strict(
                            args[4], this_->name_, this_->codename_);
                }

                if (strides != 1 && dilation_rate != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv1d_operation::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));
                }

                if (strides == 1 && dilation_rate == 1)
                {
                    return this_->conv1d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding));
                }
                if (dilation_rate == 1)
                {
                    return this_->conv1d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), strides);
                }

                // strides == 1 and dilation_rate > 1
                return this_->conv1d_any_pad_dilation(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_),
                    std::move(padding), dilation_rate);
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
