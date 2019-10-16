// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/conv2d_operation.hpp>
#include <phylanx/plugins/keras_support/conv_indices_helper.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

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

            x (array) : a 4d array consiting of batch, in_height, in_width and
                in_channels dimensions.
            kernel (array) : a 4d array consisting of filter_height,
                filter_width, in_channels and out_channels dimension. Note that
                the in_channels should be the same in kernel and original array.
            padding (optional, string) : padding mode, `valid` by default. It
                can be either `valid` or `same`. `vaild` means no padding.
                `same` results the output with the same shape as original array
                in case of unit strides.
            strides (optional, a tuple of two integers) : the steps to apply
                convolution over height and width of the array. It sets to
                (1,1) by default.
            dilation_rate (optional, a tuple of two integers) : indicates the
                dilation rate, the rate to sample the height and the width of
                the array in each step of convolution, (1,1) by default.

        Returns:

        2D convolution (or 2D mathematical cross-correlation))")
    };

    ///////////////////////////////////////////////////////////////////////////
    conv2d_operation::conv2d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        std::size_t batch = q.quats();
        std::size_t in_height = q.pages();
        std::size_t in_width = q.rows();
        std::size_t in_channels = q.columns();
        std::size_t filter_height = k.quats();
        std::size_t filter_width = k.pages();
        std::size_t out_channels = k.columns();

        std::size_t res_height = in_height - filter_height + 1;
        std::size_t res_width = in_width - filter_width + 1;
        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto k_tensor = blaze::quatslice(blaze::trans(k, {3, 0, 1, 2}), c);
            auto res_tensor =
                blaze::quatslice(blaze::trans(result, {3, 0, 1, 2}), c);
            for (std::size_t l = 0; l != batch; ++l)
            {
                auto a_tensor = blaze::quatslice(q, l);
                auto res_slice = blaze::pageslice(res_tensor, l);
                for (std::size_t i = 0; i != res_height; ++i)
                {
                    for (std::size_t j = 0; j != res_width; ++j)
                    {
                        auto schur_product =
                            blaze::subtensor(a_tensor, i, j, 0, filter_height,
                                filter_width, in_channels) %
                            k_tensor;

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t stride_height, std::int64_t stride_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        std::size_t batch = q.quats();
        std::size_t in_height = q.pages();
        std::size_t in_width = q.rows();
        std::size_t in_channels = q.columns();
        std::size_t filter_height = k.quats();
        std::size_t filter_width = k.pages();
        std::size_t out_channels = k.columns();

        std::size_t res_height = blaze::ceil(
            static_cast<double>(in_height - filter_height + 1) / stride_height);
        std::size_t res_width = blaze::ceil(
            static_cast<double>(in_width - filter_width + 1) / stride_width);

        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto k_tensor = blaze::quatslice(blaze::trans(k, {3, 0, 1, 2}), c);
            auto res_tensor =
                blaze::quatslice(blaze::trans(result, {3, 0, 1, 2}), c);
            for (std::size_t l = 0; l != batch; ++l)
            {
                auto a_tensor = blaze::quatslice(q, l);
                auto res_slice = blaze::pageslice(res_tensor, l);
                for (std::size_t i = 0; i != res_height; ++i)
                {
                    for (std::size_t j = 0; j != res_width; ++j)
                    {
                        auto schur_product =
                            blaze::subtensor(a_tensor, i * stride_height,
                                j * stride_width, 0, filter_height,
                                filter_width, in_channels) %
                            k_tensor;

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_valid_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t dilation_height, std::int64_t dilation_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        auto filter_height = static_cast<std::int64_t>(k.quats());
        auto filter_width= static_cast<std::int64_t>(k.pages());
        auto in_height = static_cast<std::int64_t>(q.pages());
        auto in_width  = static_cast<std::int64_t>(q.rows());
        std::size_t batch = q.quats();
        std::size_t in_channels = q.columns();
        std::size_t out_channels = k.columns();

        std::int64_t res_height =
            in_height - dilation_height * (filter_height - 1);
        std::int64_t res_width = in_width - dilation_width * (filter_width - 1);

        if(res_height <= 0 || res_width <= 0)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv2d_operation::eval",
                generate_error_message("this dilation_rate causes non-positive "
                                       "result_length where padding is valid"));

        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto k_tensor = blaze::quatslice(blaze::trans(k, {3, 0, 1, 2}), c);
            auto res_tensor =
                blaze::quatslice(blaze::trans(result, {3, 0, 1, 2}), c);
            for (std::size_t l = 0; l != batch; ++l)
            {
                auto a_tensor = blaze::quatslice(q, l);
                auto res_slice = blaze::pageslice(res_tensor, l);
                for (std::size_t i = 0; i != res_height; ++i)
                {
                    for (std::size_t j = 0; j != res_width; ++j)
                    {
                        auto schur_product =
                            blaze::dilatedsubtensor(a_tensor, i, j, 0,
                                filter_height, filter_width, in_channels,
                                dilation_height, dilation_width, 1) %
                            k_tensor;

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        auto filter_height = static_cast<std::int64_t>(k.quats());
        auto filter_width= static_cast<std::int64_t>(k.pages());
        auto in_height = static_cast<std::int64_t>(q.pages());
        auto in_width  = static_cast<std::int64_t>(q.rows());
        std::size_t batch = q.quats();
        std::size_t in_channels = q.columns();
        std::size_t out_channels = k.columns();

        std::int64_t pad_top = (filter_height - 1) / 2;
        std::int64_t pad_left = (filter_width - 1) / 2;

        blaze::DynamicArray<4UL, double> result(
            batch, in_height, in_width, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto k_tensor = blaze::quatslice(blaze::trans(k, {3, 0, 1, 2}), c);
            auto res_tensor =
                blaze::quatslice(blaze::trans(result, {3, 0, 1, 2}), c);
            for (std::size_t l = 0; l != batch; ++l)
            {
                auto a_tensor = blaze::quatslice(q, l);
                auto res_slice = blaze::pageslice(res_tensor, l);
                for (std::size_t i = 0; i != in_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes(
                        in_height, filter_height, i - pad_top);
                    for (std::size_t j = 0; j != in_width; ++j)
                    {
                        auto sub_width = conv_indices::get_subsizes(
                            in_width, filter_width, j - pad_left);
                        auto schur_product =
                            blaze::subtensor(a_tensor, sub_height.image_beg_,
                                sub_width.image_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels) %
                            blaze::subtensor(k_tensor, sub_height.kernel_beg_,
                                sub_width.kernel_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t stride_height, std::int64_t stride_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        auto filter_height = static_cast<std::int64_t>(k.quats());
        auto filter_width= static_cast<std::int64_t>(k.pages());
        auto in_height = static_cast<std::int64_t>(q.pages());
        auto in_width  = static_cast<std::int64_t>(q.rows());
        std::size_t batch = q.quats();
        std::size_t in_channels = q.columns();
        std::size_t out_channels = k.columns();
        std::int64_t pad_height;
        std::int64_t pad_width;

        if (in_width % stride_width == 0)
        {
            pad_width = filter_width > stride_width ?
                filter_width - stride_width :
                static_cast<std::int64_t>(0);
        }
        else
        {
            pad_width = filter_width > (in_width % stride_width) ?
                filter_width - (in_width % stride_width) :
                static_cast<std::int64_t>(0);
        }

        if (in_height % stride_height == 0)
        {
            pad_height = filter_height > stride_height ?
                filter_height - stride_height :
                static_cast<std::int64_t>(0);
        }
        else
        {
            pad_height = filter_height > (in_height % stride_height) ?
                filter_height - (in_height % stride_height) :
                static_cast<std::int64_t>(0);
        }

        std::size_t res_width = blaze::ceil(
            static_cast<double>(in_width + pad_width - filter_width + 1) /
            stride_width);
        std::size_t res_height = blaze::ceil(
            static_cast<double>(in_height + pad_height - filter_height + 1) /
            stride_height);

        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);
        std::int64_t pad_top  = pad_height / 2;
        std::int64_t pad_left = pad_width / 2;

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto k_tensor = blaze::quatslice(blaze::trans(k, {3, 0, 1, 2}), c);
            auto res_tensor =
                blaze::quatslice(blaze::trans(result, {3, 0, 1, 2}), c);
            for (std::size_t l = 0; l != batch; ++l)
            {
                auto a_tensor = blaze::quatslice(q, l);
                auto res_slice = blaze::pageslice(res_tensor, l);
                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes(
                        in_height, filter_height, i * stride_height - pad_top);
                    for (std::size_t j = 0; j != res_width; ++j)
                    {
                        auto sub_width = conv_indices::get_subsizes(in_width,
                            filter_width, j * stride_width - pad_left);
                        auto schur_product =
                            blaze::subtensor(a_tensor, sub_height.image_beg_,
                                sub_width.image_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels) %
                            blaze::subtensor(k_tensor, sub_height.kernel_beg_,
                                sub_width.kernel_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_operation::conv2d_same_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t dilation_height, std::int64_t dilation_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        auto filter_height = static_cast<std::int64_t>(k.quats());
        auto filter_width= static_cast<std::int64_t>(k.pages());
        auto in_height = static_cast<std::int64_t>(q.pages());
        auto in_width  = static_cast<std::int64_t>(q.rows());
        std::size_t batch = q.quats();
        std::size_t in_channels = q.columns();
        std::size_t out_channels = k.columns();

        std::int64_t pad_top = (dilation_height * (filter_height - 1)) / 2;
        std::int64_t pad_left = (dilation_width * (filter_width - 1)) / 2;

        blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
            batch, in_height, in_width, out_channels);

        for (std::size_t c = 0; c != out_channels; ++c)
        {
            auto k_tensor = blaze::quatslice(blaze::trans(k, {3, 0, 1, 2}), c);
            auto res_tensor =
                blaze::quatslice(blaze::trans(result, {3, 0, 1, 2}), c);
            for (std::size_t l = 0; l != batch; ++l)
            {
                auto a_tensor = blaze::quatslice(q, l);
                auto res_slice = blaze::pageslice(res_tensor, l);
                for (std::size_t i = 0; i != in_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes_dilated(
                        in_height, filter_height, i - pad_top, dilation_height);
                    for (std::size_t j = 0; j != in_width; ++j)
                    {
                        auto sub_width =
                            conv_indices::get_subsizes_dilated(in_width,
                                filter_width, j - pad_left, dilation_width);
                        if (sub_height.size_ == 0 || sub_width.size_ == 0)
                            continue;

                        auto schur_product =
                            blaze::dilatedsubtensor(a_tensor,
                                sub_height.image_beg_, sub_width.image_beg_, 0,
                                sub_height.size_, sub_width.size_, in_channels,
                                dilation_height, dilation_width, 1) %
                            blaze::subtensor(k_tensor, sub_height.kernel_beg_,
                                sub_width.kernel_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding) const
    {
        if (padding == "valid")
        {
            return conv2d_valid(std::move(arg), std::move(kernel));
        }

        // padding == "same"
        return conv2d_same(std::move(arg), std::move(kernel));
    }

    primitive_argument_type conv2d_operation::conv2d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::int64_t stride_height,
        std::int64_t stride_width) const
    {
        if (padding == "valid")
        {
            return conv2d_valid(
                std::move(arg), std::move(kernel), stride_height, stride_width);
        }

        // padding == "same"
        return conv2d_same(
            std::move(arg), std::move(kernel), stride_height, stride_width);
    }

    primitive_argument_type conv2d_operation::conv2d_any_pad_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::int64_t dilation_height,
            std::int64_t dilation_width) const
    {
        if (padding == "valid")
        {
            return conv2d_valid_dilation(std::move(arg), std::move(kernel),
                dilation_height, dilation_width);
        }

        // padding == "same"
        return conv2d_same_dilation(
            std::move(arg), std::move(kernel), dilation_height, dilation_width);
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
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "conv2d_operation::eval",
                    generate_error_message(
                        "the conv2d_operation primitive requires that the "
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
                    ndim != 4)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "conv2d operation requires for x and kernel to be "
                            "4d arrays"));
                }

                if (extract_numeric_value_dimensions(
                        args[0], this_->name_, this_->codename_)[3] !=
                    extract_numeric_value_dimensions(
                        args[1], this_->name_, this_->codename_)[2])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_transpose_operation::eval",
                        this_->generate_error_message(
                            "image's input channels does not match filters' "
                            "input channels"));
                }

                std::string padding = "valid";
                if (args.size() > 2)
                {
                    padding = extract_string_value_strict(
                        args[2], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either "
                                "'valid' or 'same'"));
                    }
                }


                if (padding == "valid")
                {
                    if (extract_numeric_value_dimensions(
                            args[0], this_->name_, this_->codename_)[1] <
                        extract_numeric_value_dimensions(
                            args[1], this_->name_, this_->codename_)[0] ||
                        extract_numeric_value_dimensions(
                            args[0], this_->name_, this_->codename_)[2] <
                        extract_numeric_value_dimensions(
                            args[1], this_->name_, this_->codename_)[1])
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "the kernel size cannot be greater than the "
                                "array size in the valid padding mode"));
                    }
                }

                ir::range strides(0); // an empty range
                std::size_t stride_height;
                std::size_t stride_width;
                if (args.size() > 3)
                {
                    strides = extract_list_value_strict(
                        args[3], this_->name_, this_->codename_);
                    if (strides.size() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "conv2d requires strides to be a tuple of 2 "
                                "integers"));
                    }

                    auto it_s = strides.begin();
                    stride_height =
                        extract_scalar_positive_integer_value_strict(*it_s);
                    stride_width =
                        extract_scalar_positive_integer_value_strict(*++it_s);

                    if (stride_height == 1 && stride_width == 1)
                    {
                        strides = ir::range(0);
                    }
                }

                ir::range dilation_rate(0); // an empty range
                std::int64_t dilation_height;
                std::int64_t dilation_width;
                if (args.size() > 4)
                {
                    dilation_rate = extract_list_value_strict(
                        args[4], this_->name_, this_->codename_);
                    if (dilation_rate.size() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "conv2d requires dilation_rate to be a tuple of"
                                "2 integers"));
                    }

                    auto it_d = dilation_rate.begin();
                    dilation_height =
                        extract_scalar_positive_integer_value_strict(*it_d);
                    dilation_width =
                        extract_scalar_positive_integer_value_strict(*++it_d);

                    if (dilation_height == 1 && dilation_width == 1)
                    {
                        dilation_rate = ir::range(0);
                    }
                }

                if (!strides.empty() && !dilation_rate.empty())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_operation::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));
                }

                if (strides.empty() && dilation_rate.empty())
                {
                    return this_->conv2d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding));
                }
                if (dilation_rate.empty()) // strides != (1,1)
                {
                    return this_->conv2d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), stride_height, stride_width);
                }

                // strides == (1,1) and dilation_rate != (1,1)
                return this_->conv2d_any_pad_dilation(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_),
                    std::move(padding), dilation_height, dilation_width);
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
