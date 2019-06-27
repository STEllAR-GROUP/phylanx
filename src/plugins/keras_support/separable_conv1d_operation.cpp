// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/conv_indices_helper.hpp>
#include <phylanx/plugins/keras_support/separable_conv1d_operation.hpp>

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
    match_pattern_type const separable_conv1d_operation::match_data = {
        hpx::util::make_tuple("separable_conv1d", std::vector<std::string>{R"(
            separable_conv1d(_1,
            _2_depthwise_kernel,
            _3_pointwise_kernel,
            __arg(_4_padding, "valid"),
            __arg(_5_strides, 1),
            __arg(_6_dilation_rate, 1))
        )"},
            &create_separable_conv1d_operation,
            &create_primitive<separable_conv1d_operation>,
            R"(x, depthwise_kernel, pointwise_kernel, padding, strides,
            dilation_rate
        Args:

            x (array) : a stream of 3d data: batch, input_length, in_channels
            depthwise_kernel (array) : a 3d filter which dimensions are:
                filter_length, depth_in_channels, depth_out_channels
            pointwise_kernel (array) : a 3d filter which dimensions are: 1,
                point_in_channels, point_out_channels. point_in_channels should
                be depth_in_channels * depth_out_channels
            padding (optional, string) : padding mode, `valid` by default. It
                can be either `valid` or `same`. `vaild` means no padding.
                `same` results the output with the same shape as original array
                 in case of unit strides.
            strides (optional, integer) : the step to apply convolution over
                array. It sets to 1 by default.
            dilation_rate (optional, integer) : indicates the dilation rate,
                the rate to sample the array in each step of convolution, 1
                by default.

        Returns:

        1D convolution with separable filters)")};

    ///////////////////////////////////////////////////////////////////////////
    separable_conv1d_operation::separable_conv1d_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type separable_conv1d_operation::sep_conv1d_valid(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel) const
    {
        auto a = arg.tensor();
        auto dk = depth_kernel.tensor();
        auto pk = point_kernel.tensor();

        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t dk_length = dk.pages();
        std::size_t dk_out_channels = dk.columns();
        std::size_t pk_out_channels = pk.columns();
        std::size_t result_length = a.rows() - dk_length + 1;

        blaze::DynamicTensor<double> result(
            batch, result_length, pk_out_channels, 0.);

        for (std::size_t i = 0; i != in_channels; ++i)
        {
            for (std::size_t j = 0; j != result_length; ++j)
            {
                blaze::rowslice(result, j) +=
                    blaze::trans(blaze::submatrix(blaze::columnslice(a, i), 0,
                                     j, batch, dk_length) *
                        blaze::trans(blaze::rowslice(dk, i)) *
                        blaze::submatrix(blaze::pageslice(pk, 0),
                            dk_out_channels * i, 0, dk_out_channels,
                            pk_out_channels));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type separable_conv1d_operation::sep_conv1d_valid(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel, std::int64_t strides) const
    {
        auto a = arg.tensor();
        auto dk = depth_kernel.tensor();
        auto pk = point_kernel.tensor();

        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t dk_length = dk.pages();
        std::size_t dk_out_channels = dk.columns();
        std::size_t pk_out_channels = pk.columns();
        std::size_t result_length = blaze::ceil(
            static_cast<double>(a.rows() - dk_length + 1) / strides);

        blaze::DynamicTensor<double> result(
            batch, result_length, pk_out_channels, 0.);

        for (std::size_t i = 0; i != in_channels; ++i)
        {
            for (std::size_t j = 0; j != result_length; ++j)
            {
                blaze::rowslice(result, j) +=
                    blaze::trans(blaze::submatrix(blaze::columnslice(a, i), 0,
                                     j * strides, batch, dk_length) *
                        blaze::trans(blaze::rowslice(dk, i)) *
                        blaze::submatrix(blaze::pageslice(pk, 0),
                            dk_out_channels * i, 0, dk_out_channels,
                            pk_out_channels));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type
    separable_conv1d_operation::sep_conv1d_valid_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel, std::int64_t dilation_rate) const
    {
        auto a = arg.tensor();
        auto dk = depth_kernel.tensor();
        auto pk = point_kernel.tensor();

        auto dk_length = static_cast<std::int64_t>(dk.pages());
        auto data_length = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t dk_out_channels = dk.columns();
        std::size_t pk_out_channels = pk.columns();
        std::int64_t result_length =
            data_length - dilation_rate * (dk_length - 1);

        if(result_length <= 0)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sep_conv1d_operation::eval",
                generate_error_message(
                    "this dilation_rate causes non-positive "
                    "result_length where padding is valid"));

        blaze::DynamicTensor<double> result(
            batch, result_length, pk_out_channels, 0.);

        for (std::size_t i = 0; i != in_channels; ++i)
        {
            for (std::size_t j = 0; j != result_length; ++j)
            {
                blaze::rowslice(result, j) += blaze::trans(
                    blaze::dilatedsubmatrix(blaze::columnslice(a, i), 0, j,
                        batch, dk_length, 1, dilation_rate) *
                    blaze::trans(blaze::rowslice(dk, i)) *
                    blaze::submatrix(blaze::pageslice(pk, 0),
                        dk_out_channels * i, 0, dk_out_channels,
                        pk_out_channels));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type separable_conv1d_operation::sep_conv1d_same(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel) const
    {
        auto a = arg.tensor();
        auto dk = depth_kernel.tensor();
        auto pk = point_kernel.tensor();

        auto dk_length = static_cast<std::int64_t>(dk.pages());
        auto data_length = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t dk_out_channels = dk.columns();
        std::size_t pk_out_channels = pk.columns();
        std::int64_t pad_left = (dk_length - 1) / 2;

        blaze::DynamicTensor<double> result(
            batch, data_length, pk_out_channels, 0.);

        for (std::size_t i = 0; i != in_channels; ++i)
        {
            for (std::size_t j = 0; j != data_length; ++j)
            {
                auto sub = get_subsizes(
                    data_length, dk_length, j - pad_left);
                blaze::rowslice(result, j) +=
                    blaze::trans(blaze::submatrix(blaze::columnslice(a, i), 0,
                                     sub.image_beg_, batch, sub.size_) *
                        blaze::trans(blaze::submatrix(blaze::rowslice(dk, i), 0,
                            sub.kernel_beg_, dk_out_channels, sub.size_)) *
                        blaze::submatrix(blaze::pageslice(pk, 0),
                            dk_out_channels * i, 0, dk_out_channels,
                            pk_out_channels));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type separable_conv1d_operation::sep_conv1d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel, std::int64_t strides) const
    {
        auto a = arg.tensor();
        auto dk = depth_kernel.tensor();
        auto pk = point_kernel.tensor();

        auto dk_length = static_cast<std::int64_t>(dk.pages());
        auto data_length = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t dk_out_channels = dk.columns();
        std::size_t pk_out_channels = pk.columns();
        std::int64_t pad_width;

        if (data_length % strides == 0)
        {
            pad_width =
                (blaze::max)(dk_length - strides, static_cast<std::int64_t>(0));
        }
        else
        {
            pad_width = (blaze::max)(dk_length - (data_length % strides),
                static_cast<std::int64_t>(0));
        }

        std::size_t result_length = blaze::ceil(
            static_cast<double>(data_length + pad_width - dk_length + 1) /
            strides);

        blaze::DynamicTensor<double> result(
            batch, result_length, pk_out_channels, 0.);
        std::size_t pad_left = pad_width / 2;

        for (std::size_t i = 0; i != in_channels; ++i)
        {
            for (std::size_t j = 0; j != result_length; ++j)
            {
                auto sub = get_subsizes(
                    data_length, dk_length, j * strides - pad_left);
                blaze::rowslice(result, j) +=
                    blaze::trans(blaze::submatrix(blaze::columnslice(a, i), 0,
                                     sub.image_beg_, batch, sub.size_) *
                        blaze::trans(blaze::submatrix(blaze::rowslice(dk, i), 0,
                            sub.kernel_beg_, dk_out_channels, sub.size_)) *
                        blaze::submatrix(blaze::pageslice(pk, 0),
                            dk_out_channels * i, 0, dk_out_channels,
                            pk_out_channels));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type separable_conv1d_operation::sep_conv1d_same_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel, std::int64_t dilation_rate) const
    {
        auto a = arg.tensor();
        auto dk = depth_kernel.tensor();
        auto pk = point_kernel.tensor();

        auto dk_length = static_cast<std::int64_t>(dk.pages());
        auto data_length = static_cast<std::int64_t>(a.rows());
        std::size_t batch = a.pages();
        std::size_t in_channels = a.columns();
        std::size_t dk_out_channels = dk.columns();
        std::size_t pk_out_channels = pk.columns();
        std::int64_t pad_left = (dilation_rate * (dk_length - 1) ) / 2;

        blaze::DynamicTensor<double> result(
            batch, data_length, pk_out_channels, 0.);

        for (std::size_t i = 0; i != in_channels; ++i)
        {
            for (std::size_t j = 0; j != data_length; ++j)
            {
                auto sub = get_subsizes_dilated(
                    data_length, dk_length, j - pad_left, dilation_rate);

                if (sub.size_ == 0)
                    continue;

                blaze::rowslice(result, j) += blaze::trans(
                    blaze::dilatedsubmatrix(blaze::columnslice(a, i), 0,
                        sub.image_beg_, batch, sub.size_, 1, dilation_rate) *
                    blaze::trans(blaze::submatrix(blaze::rowslice(dk, i), 0,
                        sub.kernel_beg_, dk_out_channels, sub.size_)) *
                    blaze::submatrix(blaze::pageslice(pk, 0),
                        dk_out_channels * i, 0, dk_out_channels,
                        pk_out_channels));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type separable_conv1d_operation::sep_conv1d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& depth_kernel,
            ir::node_data<double>&& point_kernel,
        std::string&& padding) const
    {
        if (padding == "valid")
            return sep_conv1d_valid(std::move(arg), std::move(depth_kernel),
                std::move(point_kernel));

        // padding == "same"
        return sep_conv1d_same(std::move(arg), std::move(depth_kernel),
                std::move(point_kernel));
    }

    primitive_argument_type separable_conv1d_operation::sep_conv1d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel, std::string&& padding,
        std::int64_t strides) const
    {
        if (padding == "valid")
            return sep_conv1d_valid(std::move(arg), std::move(depth_kernel),
                std::move(point_kernel), strides);

        // padding == "same"
        return sep_conv1d_same(std::move(arg), std::move(depth_kernel),
            std::move(point_kernel), strides);
    }

    primitive_argument_type
    separable_conv1d_operation::sep_conv1d_any_pad_dilation(
        ir::node_data<double>&& arg, ir::node_data<double>&& depth_kernel,
        ir::node_data<double>&& point_kernel, std::string&& padding,
        std::int64_t dilation_rate) const
    {
        if (padding == "valid")
            return sep_conv1d_valid_dilation(std::move(arg),
                std::move(depth_kernel), std::move(point_kernel),
                dilation_rate);

         //padding == "same"
        return sep_conv1d_same_dilation(std::move(arg), std::move(depth_kernel),
            std::move(point_kernel), dilation_rate);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> separable_conv1d_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "separable_conv1d_operation::eval",
                generate_error_message("the separable_conv1d_operation requires "
                                       "between 2 and 6 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "separable_conv1d_operation::eval",
                    generate_error_message(
                        "the separable_conv1d primitive requires that the "
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
                    ndim !=
                        extract_numeric_value_dimension(
                            args[2], this_->name_, this_->codename_) ||
                    ndim != 3)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "separable_conv1d_operation::eval",
                        this_->generate_error_message(
                            "separable_conv1d operation requires for a and "
                            "kernels to be of rank 3"));

                if (extract_numeric_value_dimensions(
                    args[2], this_->name_, this_->codename_)[0] != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "separable_conv1d_operation::eval",
                        this_->generate_error_message(
                            "the pointwise kernel should have only 1 page"));
                }

                std::string padding = "valid";
                if (args.size() > 3)
                {
                    padding = extract_string_value(
                        args[3], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same")
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "separable_conv1d_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either "
                                "`valid`, or `same`"));
                }

                if (padding == "valid")
                {
                    if (extract_numeric_value_dimensions(
                            args[0], this_->name_, this_->codename_)[1] <
                        extract_numeric_value_dimensions(
                            args[1], this_->name_, this_->codename_)[0])
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "separable_conv1d_operation::eval",
                            this_->generate_error_message(
                                "the depthwise kernel length (dim 0) cannot be "
                                "greater than the data length (dim 1) in the "
                                "valid padding mode"));
                }

                std::int64_t strides = 1;
                if (args.size() > 4)
                {
                    if (is_list_operand_strict(args[4]))
                    {
                        ir::range s = extract_list_value(
                            args[4], this_->name_, this_->codename_);
                        if (s.size() == 1)
                        {
                            strides =
                                extract_scalar_positive_integer_value_strict(
                                    *s.begin());
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "separable_conv1d_operation::eval",
                                this_->generate_error_message(
                                    "separable_conv1d requires the strides to "
                                    "be of rank 1"));
                        }
                    }
                    else
                    {
                        strides = extract_scalar_positive_integer_value_strict(
                            args[4], this_->name_, this_->codename_);
                    }
                }

                std::int64_t dilation_rate = 1;
                if (args.size() > 5)
                {
                    if (is_list_operand_strict(args[5]))
                    {
                        ir::range d = extract_list_value(
                            args[5], this_->name_, this_->codename_);
                        if (d.size() == 1)
                        {
                            dilation_rate =
                                extract_scalar_positive_integer_value_strict(
                                    *d.begin());
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "separable_conv1d_operation::eval",
                                this_->generate_error_message(
                                    "separable_conv1d requires the "
                                    "dilation_rate to be of rank 1"));
                        }
                    }
                    else
                    {
                        dilation_rate =
                            extract_scalar_positive_integer_value_strict(
                                args[5], this_->name_, this_->codename_);
                    }
                }

                if (strides != 1 && dilation_rate != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "separable_conv1d_operation::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));
                }

                if (strides == 1 && dilation_rate == 1)
                {
                    return this_->sep_conv1d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[2]), this_->name_, this_->codename_),
                        std::move(padding));
                }
                else if (dilation_rate == 1) // strides > 1
                {
                    return this_->sep_conv1d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[2]), this_->name_, this_->codename_),
                        std::move(padding), strides);
                }
                // strides == 1 and dilation_rate > 1
                return this_->sep_conv1d_any_pad_dilation(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[2]), this_->name_, this_->codename_),
                    std::move(padding), dilation_rate);

            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
