// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/conv1d_all_paddings.hpp>
#include <phylanx/plugins/keras_support/conv1d_operation.hpp>
#include <phylanx/plugins/keras_support/conv_indices_helper.hpp>

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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const conv1d_operation::match_data =
    {
        hpx::make_tuple("conv1d",
        std::vector<std::string>{R"(
            conv1d(_1, _2_kernel,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, 1),
            __arg(_5_dilation_rate, 1))
        )"},
        &create_conv1d_operation, &create_primitive<conv1d_operation>,
        R"(x, kernel, padding, strides, dilation_rate
        Args:

            x (array) : a 3d array consiting of batch, in_length and
                in_channels dimensions.
            kernel (array) : a 3d array consisting of filter_length,
                in_channels and out_channels dimension. Note that the
                in_channels should be the same in kernel and original array.
            padding (optional, string) : padding mode, `valid` by default. It
                can be either `valid`, `same` or `causal`. `vaild` means no
                padding. `same` results the output with the same shape as
                original array in case of unit strides. `causal` zero pads the
                array in a way that no output element depend on the input
                elements of its future.
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
                if (args.size() > 2)
                {
                    padding = extract_string_value_strict(
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
                }


                if (padding == "valid")
                {
                    if (extract_numeric_value_dimensions(
                            args[0], this_->name_, this_->codename_)[1] <
                        extract_numeric_value_dimensions(
                            args[1], this_->name_, this_->codename_)[0])
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "the kernel size cannot be greater than the "
                                "array size in the valid padding mode"));
                    }
                }

                std::int64_t strides = 1;
                if (args.size() > 3)
                {
                    if (is_list_operand_strict(args[3]))
                    {
                        ir::range s = extract_list_value(
                            args[3], this_->name_, this_->codename_);
                        if (s.size() == 1)
                        {
                            strides =
                                extract_scalar_positive_integer_value_strict(
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
                }

                std::int64_t dilation_rate = 1;
                if (args.size() > 4)
                {
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
                    return common::conv1d_all_paddings(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), this_->name_, this_->codename_);
                }
                if (dilation_rate == 1) // strides > 1
                {
                    return common::conv1d_all_paddings(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding), strides, this_->name_,
                        this_->codename_);
                }

                // strides == 1 and dilation_rate > 1
                return common::conv1d_all_paddings_dilation(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    extract_numeric_value(
                        std::move(args[1]), this_->name_, this_->codename_),
                    std::move(padding), dilation_rate, this_->name_,
                    this_->codename_);
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
