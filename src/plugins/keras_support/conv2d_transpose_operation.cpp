// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/conv2d_transpose_operation.hpp>
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
    match_pattern_type const conv2d_transpose_operation::match_data =
    {
        hpx::util::make_tuple("conv2d_transpose",
        std::vector<std::string>{R"(
            conv2d_transpose(_1,
            _2_kernel,
            _3_output_shape,
            __arg(_4_padding, "valid"),
            __arg(_5_strides, list(1,1)),
            __arg(_6_dilation_rate, list(1,1)))
        )"},
        &create_conv2d_transpose_operation,
        &create_primitive<conv2d_transpose_operation>,
        R"(x, kernel, padding, strides, dilation_rate
        Args:

            x (array) : a 4d array consiting of batch, in_height, in_width and
                in_channels dimensions.
            kernel (array) : a 4d array consisting of filter_height,
                filter_width, out_channels and in_channels dimension. Note that
                the in_channels should be the same in kernel and original array.
            output_shape (vector of 4 integers): shape of the output consisting
                of batch, output_height, output_width, out_channels
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

        2D deconvolution)")
    };

    ///////////////////////////////////////////////////////////////////////////
    conv2d_transpose_operation::conv2d_transpose_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Tensor>
    void conv2d_transpose_operation::flip_kernel(Tensor& t) const
    {
        std::size_t columns = t.columns();
        for (std::size_t c = 0; c != columns; ++c)
        {
            auto slice = blaze::columnslice(t, c);
            slice = blaze::reverse<blaze::rowwise>(slice);
            slice = blaze::reverse<blaze::columnwise>(slice);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_transpose_operation::conv2d_transpose_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::size_t res_height, std::size_t res_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        std::size_t batch = q.quats();
        std::size_t in_height = q.pages();
        std::size_t in_width = q.rows();
        std::size_t in_channels = q.columns();
        std::size_t filter_height = k.quats();
        std::size_t filter_width = k.pages();
        std::size_t out_channels = k.rows();

        std::int64_t pad_top  = filter_height - 1;
        std::int64_t pad_left = filter_width - 1;
        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto a_tensor = blaze::quatslice(q, l);
            auto res_tensor = blaze::quatslice(result, l);

            for (std::size_t o = 0; o != out_channels; ++o)
            {
                auto res_slice = blaze::columnslice(res_tensor, o);
                auto k_tensor =
                    blaze::quatslice(blaze::trans(k, {2, 0, 1, 3}), o);

                blaze::DynamicTensor<double> flipped_kernel = k_tensor;
                flip_kernel(flipped_kernel);

                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes(
                            in_height, filter_height, i - pad_top);
                    for (std::size_t j = 0; j != res_width; ++j)
                    {

                        auto sub_width = conv_indices::get_subsizes(
                            in_width, filter_width, j - pad_left);
                        auto schur_product =
                            blaze::subtensor(a_tensor, sub_height.image_beg_,
                                sub_width.image_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels) %
                            blaze::subtensor(flipped_kernel,
                                sub_height.kernel_beg_, sub_width.kernel_beg_,
                                0, sub_height.size_, sub_width.size_,
                                in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_transpose_operation::conv2d_transpose_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::size_t res_height, std::size_t res_width,
        std::size_t stride_height, std::size_t stride_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        std::size_t batch = q.quats();
        std::size_t in_height = q.pages();
        std::size_t in_width = q.rows();
        std::size_t in_channels = q.columns();
        std::size_t filter_height = k.quats();
        std::size_t filter_width = k.pages();
        std::size_t out_channels = k.rows();

        std::int64_t pad_top  = filter_height - 1;
        std::int64_t pad_left = filter_width - 1;
        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto a_tensor = blaze::quatslice(q, l);
            auto res_tensor = blaze::quatslice(result, l);

            for (std::size_t o = 0; o != out_channels; ++o)
            {
                auto res_slice = blaze::columnslice(res_tensor, o);
                auto k_tensor =
                    blaze::quatslice(blaze::trans(k, {2, 0, 1, 3}), o);

                blaze::DynamicTensor<double> flipped_kernel = k_tensor;
                flip_kernel(flipped_kernel);

                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes_transpose(
                        in_height, filter_height, i - pad_top, stride_height);
                    for (std::size_t j = 0; j != res_width; ++j)
                    {
                        auto sub_width = conv_indices::get_subsizes_transpose(
                            in_width, filter_width, j - pad_left, stride_width);
                        auto schur_product =
                            blaze::subtensor(a_tensor, sub_height.image_beg_,
                                sub_width.image_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels) %
                            blaze::dilatedsubtensor(flipped_kernel,
                                sub_height.kernel_beg_, sub_width.kernel_beg_,
                                0, sub_height.size_, sub_width.size_,
                                in_channels, stride_height, stride_width, 1);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type
    conv2d_transpose_operation::conv2d_transpose_valid_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::size_t res_height, std::size_t res_width,
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
        std::size_t out_channels = k.rows();

        std::int64_t pad_top = dilation_height * (filter_height - 1);
        std::int64_t pad_left = dilation_width * (filter_width - 1);

        blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
            batch, res_height, res_width, out_channels);

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto a_tensor = blaze::quatslice(q, l);
            auto res_tensor = blaze::quatslice(result, l);

            for (std::size_t o = 0; o != out_channels; ++o)
            {
                auto res_slice = blaze::columnslice(res_tensor, o);
                auto k_tensor =
                    blaze::quatslice(blaze::trans(k, {2, 0, 1, 3}), o);

                blaze::DynamicTensor<double> flipped_kernel = k_tensor;
                flip_kernel(flipped_kernel);

                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes_dilated(
                        in_height, filter_height, i - pad_top, dilation_height);
                    for (std::size_t j = 0; j != res_width; ++j)
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
                            blaze::subtensor(flipped_kernel,
                                sub_height.kernel_beg_, sub_width.kernel_beg_,
                                0, sub_height.size_, sub_width.size_,
                                in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_transpose_operation::conv2d_transpose_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::size_t res_height, std::size_t res_width) const
    {
        auto q = arg.quatern();
        auto k = kernel.quatern();
        std::size_t in_height = q.pages();
        std::size_t in_width = q.rows();

        if (in_height != res_height || in_width != res_width)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv2d_transpose_operation::conv2d_transpose_same",
                generate_error_message(
                    "At least one of the output height or width are not the "
                    "same as the height or width of the input array"));
        }
        std::size_t batch = q.quats();
        std::size_t in_channels = q.columns();
        std::size_t filter_height = k.quats();
        std::size_t filter_width = k.pages();
        std::size_t out_channels = k.rows();

        std::size_t pad_top =
            blaze::ceil(static_cast<double>(filter_height - 1) / 2.);
        std::size_t pad_left =
            blaze::ceil(static_cast<double>(filter_width - 1) / 2.);
        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto a_tensor = blaze::quatslice(q, l);
            auto res_tensor = blaze::quatslice(result, l);

            for (std::size_t o = 0; o != out_channels; ++o)
            {
                auto res_slice = blaze::columnslice(res_tensor, o);
                auto k_tensor =
                    blaze::quatslice(blaze::trans(k, {2, 0, 1, 3}), o);

                blaze::DynamicTensor<double> flipped_kernel = k_tensor;
                flip_kernel(flipped_kernel);

                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes(
                            in_height, filter_height, i - pad_top);
                    for (std::size_t j = 0; j != res_width; ++j)
                    {

                        auto sub_width = conv_indices::get_subsizes(
                            in_width, filter_width, j - pad_left);
                        auto schur_product =
                            blaze::subtensor(a_tensor, sub_height.image_beg_,
                                sub_width.image_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels) %
                            blaze::subtensor(flipped_kernel,
                                sub_height.kernel_beg_, sub_width.kernel_beg_,
                                0, sub_height.size_, sub_width.size_,
                                in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type conv2d_transpose_operation::conv2d_transpose_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::size_t res_height, std::size_t res_width,
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
        std::size_t out_channels = k.rows();
        std::int64_t pad_height =
            res_height - (in_height - 1) * stride_height + filter_height - 2;
        std::int64_t pad_width =
            res_width - (in_width - 1) * stride_width + filter_width - 2;
        std::size_t pad_top = blaze::ceil(static_cast<double>(pad_height) / 2.);
        std::size_t pad_left = blaze::ceil(static_cast<double>(pad_width) / 2.);

        blaze::DynamicArray<4UL, double> result(
            batch, res_height, res_width, out_channels);

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto a_tensor = blaze::quatslice(q, l);
            auto res_tensor = blaze::quatslice(result, l);

            for (std::size_t o = 0; o != out_channels; ++o)
            {
                auto res_slice = blaze::columnslice(res_tensor, o);
                auto k_tensor =
                    blaze::quatslice(blaze::trans(k, {2, 0, 1, 3}), o);

                blaze::DynamicTensor<double> flipped_kernel = k_tensor;
                flip_kernel(flipped_kernel);

                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes_transpose(
                        in_height, filter_height, i - pad_top, stride_height);
                    for (std::size_t j = 0; j != res_width; ++j)
                    {
                        auto sub_width = conv_indices::get_subsizes_transpose(
                            in_width, filter_width, j - pad_left, stride_width);
                        auto schur_product =
                            blaze::subtensor(a_tensor, sub_height.image_beg_,
                                sub_width.image_beg_, 0, sub_height.size_,
                                sub_width.size_, in_channels) %
                            blaze::dilatedsubtensor(flipped_kernel,
                                sub_height.kernel_beg_, sub_width.kernel_beg_,
                                0, sub_height.size_, sub_width.size_,
                                in_channels, stride_height, stride_width, 1);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type
    conv2d_transpose_operation::conv2d_transpose_same_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::size_t res_height, std::size_t res_width,
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
        std::size_t out_channels = k.rows();

        std::int64_t pad_top = blaze::ceil(
            static_cast<double>(dilation_height * (filter_height - 1)) / 2.);
        std::int64_t pad_left = blaze::ceil(
            static_cast<double>(dilation_width * (filter_width - 1)) / 2.);

        blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
            batch, res_height, res_width, out_channels);

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto a_tensor = blaze::quatslice(q, l);
            auto res_tensor = blaze::quatslice(result, l);

            for (std::size_t o = 0; o != out_channels; ++o)
            {
                auto res_slice = blaze::columnslice(res_tensor, o);
                auto k_tensor =
                    blaze::quatslice(blaze::trans(k, {2, 0, 1, 3}), o);

                blaze::DynamicTensor<double> flipped_kernel = k_tensor;
                flip_kernel(flipped_kernel);

                for (std::size_t i = 0; i != res_height; ++i)
                {
                    auto sub_height = conv_indices::get_subsizes_dilated(
                        in_height, filter_height, i - pad_top, dilation_height);
                    for (std::size_t j = 0; j != res_width; ++j)
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
                            blaze::subtensor(flipped_kernel,
                                sub_height.kernel_beg_, sub_width.kernel_beg_,
                                0, sub_height.size_, sub_width.size_,
                                in_channels);

                        res_slice(i, j) = blaze::sum(schur_product);
                    }
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type
    conv2d_transpose_operation::conv2d_transpose_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::size_t res_height,
        std::size_t res_width) const
    {
        if (padding == "valid")
        {
            return conv2d_transpose_valid(
                std::move(arg), std::move(kernel), res_height, res_width);
        }

        // padding == "same"
        return conv2d_transpose_same(
            std::move(arg), std::move(kernel), res_height, res_width);
    }

    primitive_argument_type
    conv2d_transpose_operation::conv2d_transpose_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::size_t res_height, std::size_t res_width,
        std::size_t stride_height, std::size_t stride_width) const
    {
        if (padding == "valid")
        {
            return conv2d_transpose_valid(std::move(arg), std::move(kernel),
                res_height, res_width, stride_height, stride_width);
        }

        // padding == "same"
        return conv2d_transpose_same(std::move(arg), std::move(kernel),
            res_height, res_width, stride_height, stride_width);
    }

    primitive_argument_type
    conv2d_transpose_operation::conv2d_transpose_any_pad_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding, std::size_t res_height, std::size_t res_width,
        std::int64_t dilation_height, std::int64_t dilation_width) const
    {
        if (padding == "valid")
        {
            return conv2d_transpose_valid_dilation(std::move(arg),
                std::move(kernel), res_height, res_width, dilation_height,
                dilation_width);
        }

        // padding == "same"
        return conv2d_transpose_same_dilation(std::move(arg), std::move(kernel),
            res_height, res_width, dilation_height, dilation_width);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> conv2d_transpose_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 3 || operands.size() > 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "conv2d_transpose_operation::eval",
                generate_error_message(
                    "the conv2d_transpose_operation primitive requires "
                    "between 3 and 6 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "conv2d_transpose_operation::eval",
                    generate_error_message(
                        "the conv2d_transpose_operation requires that the "
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
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims =
                    extract_numeric_value_dimensions(
                        args[0], this_->name_, this_->codename_);

                if (ndim !=
                        extract_numeric_value_dimension(
                            args[1], this_->name_, this_->codename_) ||
                    ndim != 4)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_transpose_operation::eval",
                        this_->generate_error_message(
                            "conv2d_transpose operation requires for x and "
                            "kernel to be 4d arrays"));
                }

                std::size_t batch;
                std::size_t out_height;
                std::size_t out_width;
                std::size_t out_channels;
                ir::range out_shape = extract_list_value_strict(
                    args[2], this_->name_, this_->codename_);
                if (out_shape.size() != 4)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_transpose_operation::eval",
                        this_->generate_error_message(
                            "conv2d_transpose requires output_shape to be a "
                            "vector of 4 integers"));
                }

                auto it_o = out_shape.begin();
                batch = extract_scalar_positive_integer_value_strict(*it_o);
                out_height =
                    extract_scalar_positive_integer_value_strict(*++it_o);
                out_width =
                    extract_scalar_positive_integer_value_strict(*++it_o);
                out_channels =
                    extract_scalar_positive_integer_value_strict(*++it_o);

                if (batch != dims[0] || out_channels == dims[3])
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv2d_transpose_operation::eval",
                        this_->generate_error_message(
                            "invalid output_shape is specified for the "
                            "conv2d_transpose"));
                }


                std::string padding = "valid";
                if (args.size() > 3)
                {
                    padding = extract_string_value_strict(
                        args[3], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_transpose_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either "
                                "'valid' or 'same'"));
                    }
                }


                //if (padding == "valid")
                //{
                //    if (extract_numeric_value_dimensions(
                //            args[0], this_->name_, this_->codename_)[1] <
                //        extract_numeric_value_dimensions(
                //            args[1], this_->name_, this_->codename_)[0] ||
                //        extract_numeric_value_dimensions(
                //            args[0], this_->name_, this_->codename_)[2] <
                //        extract_numeric_value_dimensions(
                //            args[1], this_->name_, this_->codename_)[1])
                //    {
                //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                //            "conv2d_transpose_operation::eval",
                //            this_->generate_error_message(
                //                "the kernel size cannot be greater than the "
                //                "array size in the valid padding mode"));
                //    }
                //}

                ir::range strides(0); // an empty range
                std::size_t stride_height;
                std::size_t stride_width;
                if (args.size() > 4)
                {
                    strides = extract_list_value_strict(
                        args[4], this_->name_, this_->codename_);
                    if (strides.size() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_transpose_operation::eval",
                            this_->generate_error_message(
                                "conv2d_transpose requires strides to be a "
                                "tuple of 2 integers"));
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
                std::size_t dilation_height;
                std::size_t dilation_width;
                if (args.size() > 5)
                {
                    dilation_rate = extract_list_value_strict(
                        args[5], this_->name_, this_->codename_);
                    if (dilation_rate.size() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_transpose_operation::eval",
                            this_->generate_error_message(
                                "conv2d_transpose requires dilation_rate to be "
                                "a tuple of 2 integers"));
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
                        "conv2d_transpose_operation::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));
                }

                if (strides.empty() && dilation_rate.empty())
                {
                    return this_->conv2d_transpose_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), out_height, out_width);
                }
                if (dilation_rate.empty()) // strides != (1,1)
                {
                    return this_->conv2d_transpose_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), out_height, out_width,
                        stride_height, stride_width);
                }

                // strides == (1,1) and dilation_rate != (1,1)
                return this_->conv2d_transpose_any_pad_dilation(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_),
                    std::move(padding), out_height, out_width, dilation_height,
                    dilation_width);
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
