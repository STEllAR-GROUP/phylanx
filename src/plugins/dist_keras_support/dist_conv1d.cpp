// Copyright (c) 2020 Bita Hasheminezhad
// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/tiling_annotations.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/conv1d_all_paddings.hpp>
#include <phylanx/plugins/dist_keras_support/dist_conv1d.hpp>
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

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace dist_keras_support { namespace primitives
{
        ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_conv1d::match_data =
    {
        hpx::make_tuple("conv1d_d",
        std::vector<std::string>{R"(
            conv1d_d(_1, _2_kernel,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, 1),
            __arg(_5_dilation_rate, 1),
            __arg(_6_name, ""))
        )"},
        &create_dist_conv1d, &execution_tree::create_primitive<dist_conv1d>,
        R"(x, kernel, padding, strides, dilation_rate, name
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
            name (optional, string): the result name. If not given it will be
                the same as the next generation of the original distributed
                array.
        Returns:
        1D convolution (or 1D mathematical cross-correlation))")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_conv1d::dist_conv1d(
        execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        execution_tree::primitive_argument_type conv1d_pad_top_bottom(
            ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
            std::size_t pad, bool mode, std::string const& name,
            std::string const& codename)
        {
            if (execution_tree::extract_numeric_value_dimensions(
                    arg, name, codename)[2] !=
                execution_tree::extract_numeric_value_dimensions(
                    kernel, name, codename)[1])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "conv1d_all_paddings::conv1d_pad_top_bottom",
                    util::generate_error_message(
                        "input depth must be evenly divisible by filter depth. "
                        "Number of input channels is not the same",
                        name, codename));
            }

            auto a = arg.tensor();
            auto k = kernel.tensor();
            auto filter_length = static_cast<std::int64_t>(k.pages());
            auto data_length = static_cast<std::int64_t>(a.rows());
            std::size_t batch = a.pages();
            std::size_t in_channels = a.columns();
            std::size_t out_channels = k.columns();
            std::size_t result_length = pad + data_length - filter_length + 1;

            blaze::DynamicTensor<double> result(
                batch, result_length, out_channels);

            if (mode)    // top padding
            {
                for (std::size_t c = 0; c != out_channels; ++c)
                {
                    auto kslice = blaze::columnslice(k, c);
                    for (std::size_t i = 0; i != result_length; ++i)
                    {
                        auto sub = conv_indices::get_subsizes_causal(
                            filter_length, i - pad);
                        auto schur_product =
                            blaze::subtensor(a, 0, sub.image_beg_, 0, batch,
                                sub.size_, in_channels) %
                            blaze::submatrix(kslice, sub.kernel_beg_, 0,
                                sub.size_, in_channels);
                        for (std::size_t p = 0; p != batch; ++p)
                        {
                            auto pslice = blaze::pageslice(schur_product, p);
                            result(p, i, c) = blaze::sum(pslice);
                        }
                    }
                }
            }
            else    // bottom padding
            {
                for (std::size_t c = 0; c != out_channels; ++c)
                {
                    auto kslice = blaze::columnslice(k, c);
                    for (std::size_t i = 0; i != result_length; ++i)
                    {
                        auto sub = conv_indices::get_subsizes(
                            data_length, filter_length, i);
                        auto schur_product =
                            blaze::subtensor(a, 0, sub.image_beg_, 0, batch,
                                sub.size_, in_channels) %
                            blaze::submatrix(kslice, sub.kernel_beg_, 0,
                                sub.size_, in_channels);
                        for (std::size_t p = 0; p != batch; ++p)
                        {
                            auto pslice = blaze::pageslice(schur_product, p);
                            result(p, i, c) = blaze::sum(pslice);
                        }
                    }
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type dist_conv1d::conv1d_all_paddings(
    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    execution_tree::localities_information&& arg_locs,
    execution_tree::localities_information&& kernel_locs,
    std::string&& padding, std::string&& given_name) const
    {
        using namespace execution_tree;
        std::uint32_t const loc_id = arg_locs.locality_.locality_id_;
        std::uint32_t const numtiles = arg_locs.locality_.num_localities_;
        std::uint32_t const numtiles_k = kernel_locs.locality_.num_localities_;

        // tiling start and stop of the result tensor
        std::int64_t res_page_start, res_page_stop, res_row_start, res_row_stop,
            res_col_start, res_col_stop;
        primitive_argument_type local_result;

        if (numtiles > 1 && numtiles_k == 1)
        {
            std::size_t filter_length = kernel.tensor().pages();

            // parallelization mode is data, spatial or a combination of both
            std::string base_name =
                given_name.empty() ? arg_locs.annotation_.name_ : given_name;

            // updating the generation of localities annotation            ;
            annotation_information ann_info(
                std::move(base_name), ++arg_locs.annotation_.generation_);

            auto locality_ann = arg_locs.locality_.as_annotation();

            res_col_start = 0;
            res_col_stop = kernel_locs.columns(name_, codename_);

            tiling_information_3d tile_info(
                arg_locs.tiles_[loc_id], name_, codename_);
            res_page_start = tile_info.spans_[0].start_;
            res_page_stop = tile_info.spans_[0].stop_;
            std::int64_t arg_row_start = tile_info.spans_[1].start_;
            std::int64_t arg_row_stop = tile_info.spans_[1].stop_;

            if (padding == "valid" || arg_locs.is_page_tiled(name_, codename_))
            {
                res_row_start = arg_row_start;
                local_result = common::conv1d_all_paddings(
                    ir::node_data<double>(std::move(arg)), std::move(kernel),
                    std::move(padding), name_, codename_);

                // getting res_row_stop considering the padding
                res_row_stop = padding == "valid" ?
                    arg_row_stop - filter_length + 1 :
                    arg_row_stop;
            }
            else
            {
                // spatial parallelization where padding is same or causal
                std ::size_t pad_top;
                if (padding == "same")
                {
                    pad_top = (filter_length - 1) / 2;
                }
                else    // causal
                {
                    pad_top = filter_length - 1;
                }
                if (arg_row_start == 0)
                {
                    // one-sided pad from top
                    // same or causal padding
                    local_result = detail::conv1d_pad_top_bottom(
                        ir::node_data<double>(std::move(arg)),
                        std::move(kernel), pad_top, true, name_, codename_);
                    res_row_start = 0;
                    res_row_stop = pad_top + arg_row_stop - filter_length + 1;
                }
                else
                {
                    res_row_start = pad_top + arg_row_start;
                    if (padding != "causal" &&
                        arg_row_stop == arg_locs.rows(name_, codename_))
                    {
                        // one-sided pad from bottom only in same padding
                        local_result = detail::conv1d_pad_top_bottom(
                            ir::node_data<double>(std::move(arg)),
                            std::move(kernel), filter_length - 1 - pad_top,
                            false, name_, codename_);
                        res_row_stop = res_row_start + arg_row_stop -
                            arg_row_start - pad_top;
                    }
                    else
                    {
                        // no padding (valid)
                        // middle parts of the array for the same padding
                        // non-top part of the array for the causal padding
                        local_result = common::conv1d_all_paddings(
                            ir::node_data<double>(std::move(arg)),
                            std::move(kernel), "valid", name_, codename_);
                        res_row_stop = res_row_start + arg_row_stop -
                            arg_row_start - filter_length + 1;
                    }
                }
            }

            tiling_information_3d res_tile_info = tiling_information_3d(
                tiling_span(res_page_start, res_page_stop),
                tiling_span(res_row_start, res_row_stop),
                tiling_span(res_col_start, res_col_stop));
            auto attached_annotation =
                std::make_shared<annotation>(localities_annotation(locality_ann,
                    res_tile_info.as_annotation(name_, codename_), ann_info,
                    name_, codename_));

            // construct new tiling annotation
            local_result.set_annotation(attached_annotation);
            return local_result;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "dist_conv1d::conv1d_all_paddings",
            generate_error_message("at this point only data and spatial "
                                   "parallelizans are supported"));
    }

    execution_tree::primitive_argument_type dist_conv1d::conv1d_all_paddings(
        execution_tree::primitive_argument_type&& arg,
        execution_tree::primitive_argument_type&& kernel,
        std::string&& padding, std::string&& given_name) const
    {
        using namespace execution_tree;
        if (arg.has_annotation() || kernel.has_annotation())
        {
            localities_information arg_locs =
                extract_localities_information(arg, name_, codename_);
            localities_information kernel_locs =
                extract_localities_information(kernel, name_, codename_);
            return conv1d_all_paddings(
                extract_numeric_value(std::move(arg), name_, codename_),
                extract_numeric_value(std::move(kernel), name_, codename_),
                std::move(arg_locs), std::move(kernel_locs), std::move(padding),
                std::move(given_name));
        }

        return common::conv1d_all_paddings(
            extract_numeric_value(std::move(arg), name_, codename_),
            extract_numeric_value(std::move(kernel), name_, codename_),
            std::move(padding), name_, codename_);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<execution_tree::primitive_argument_type> dist_conv1d::eval(
        execution_tree::primitive_arguments_type const& operands,
        execution_tree::primitive_arguments_type const& args,
        execution_tree::eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_conv1d::eval",
                generate_error_message("the dist_conv1d primitive requires "
                                       "between 2 and 6 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_conv1d::eval",
                    generate_error_message(
                        "the conv1d_d primitive requires that the arguments "
                        "given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::unwrapping([this_ = std::move(this_)](
                              execution_tree::primitive_arguments_type&& args)
                                  -> execution_tree::primitive_argument_type
            {
                using namespace execution_tree;

                std::size_t ndim = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                if (ndim !=
                        extract_numeric_value_dimension(
                            args[1], this_->name_, this_->codename_) ||
                    ndim != 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_conv1d::eval",
                        this_->generate_error_message(
                            "conv1d_d operation requires for x and kernel to be "
                            "tensors"));
                }

                std::string padding = "valid";
                if (valid(args[2]))
                {
                    padding = extract_string_value_strict(
                        args[2], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same" &&
                        padding != "causal")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_conv1d::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either "
                                "`valid`, `same`, or `causal`"));
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
                            "dist_conv1d::eval",
                            this_->generate_error_message(
                                "the kernel size cannot be greater than the "
                                "array size in the valid padding mode"));
                    }
                }

                std::int64_t strides = 1;
                if (valid(args[3]))
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
                                "dist_conv1d::eval",
                                this_->generate_error_message(
                                    "conv1d_d requires the strides to "
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
                if (valid(args[4]))
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
                                "dist_conv1d::eval",
                                this_->generate_error_message(
                                    "conv1d_d requires the dilation_rate to be "
                                    "of rank 1"));
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
                        "dist_conv1d::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));
                }

                std::string given_name = "";
                if (valid(args[5]))
                {
                    given_name = extract_string_value(std::move(args[5]),
                            this_->name_, this_->codename_);
                }

                if (strides == 1 && dilation_rate == 1)
                {
                    return this_->conv1d_all_paddings(std::move(args[0]),
                        std::move(args[1]), std::move(padding),
                        std::move(given_name));
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_conv1d::eval",
                    this_->generate_error_message(
                        "strides > 1 or dilation_rate > 1 are not currently "
                        "supported"));
            }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}
