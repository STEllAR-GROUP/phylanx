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
#include <phylanx/plugins/dist_matrixops/dist_conv1d.hpp>
#include <phylanx/plugins/dist_matrixops/retiling_calculation_helper.hpp>
#include <phylanx/plugins/dist_matrixops/tile_calculation_helper.hpp>
#include <phylanx/plugins/keras_support/conv_indices_helper.hpp>
#include <phylanx/util/distributed_tensor.hpp>
#include <phylanx/util/index_calculation_helper.hpp>

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
namespace phylanx { namespace dist_matrixops { namespace primitives
{
        ///////////////////////////////////////////////////////////////////////////
    execution_tree::match_pattern_type const dist_conv1d::match_data =
    {
        hpx::util::make_tuple("conv1d_d",
        std::vector<std::string>{R"(
            conv1d_d(_1, _2_kernel,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, 1),
            __arg(_5_dilation_rate, 1),
            __arg(_6_parallelization, "data"))
        )"},
        &create_dist_conv1d, &execution_tree::create_primitive<dist_conv1d>,
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
            parallelization (optional, string) : can be set to exploit `data`,
                `spatial` or both of `data_spatial` parallelization. The
                default mode is `data` parallelization. `data` parallelizes the
                batch dimension and `spatial` parallelization is on the
                in_length dimension.

        Returns:

        1D convolution (or 1D mathematical cross-correlation))")
    };

    ///////////////////////////////////////////////////////////////////////////
    dist_conv1d::dist_conv1d(execution_tree::primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type dist_conv1d::conv1d_all_paddings(
    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    execution_tree::localities_information&& arg_locs,
    execution_tree::localities_information&& kernel_locs,
    std::string&& padding, std::string&& par_mode) const
    {
        using namespace execution_tree;
        std::uint32_t const loc_id = arg_locs.locality_.locality_id_;
        std::uint32_t const num_localities =
            hpx::get_num_localities(hpx::launch::sync);
        auto locality_ann = arg_locs.locality_.as_annotation();
        std::size_t pages_dim = arg_locs.pages(name_, codename_);
        std::size_t rows_dim = arg_locs.rows(name_, codename_);
        std::size_t cols_dim = arg_locs.columns(name_, codename_);
        std::size_t rows_dim_k = kernel_locs.rows(name_, codename_);
        std::size_t cols_dim_k = kernel_locs.columns(name_, codename_);

        if (cols_dim != rows_dim_k)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dist_conv1d::conv1d_all_paddings",
                generate_error_message(
                    "two different input_channels are given. The array's "
                    "number of columns should be the same as the kernel's "
                    "number of rows"));
        }

        // updating the generation of localities annotation
        ++arg_locs.annotation_.generation_;

        // desired annotation information for arg
        std::int64_t des_page_start, des_page_stop, des_row_start, des_row_stop,
            des_col_start, des_col_stop, des_page_size, des_row_size,
            des_col_size;

        // tiling start and stop of the result tensor
        std::int64_t res_page_start, res_page_stop, res_row_start, res_row_stop,
            res_col_start, res_col_stop;

        if (par_mode == "data")
        {
            // batches of data are pages of the arg
            des_row_start = des_col_start = 0;
            des_row_stop = rows_dim;
            des_col_stop = cols_dim;
            std::tie(des_page_start, des_page_size) =
                tile_calculation::tile_calculation_1d(
                    loc_id, pages_dim, num_localities);

            des_page_stop = des_page_start + des_page_size;

            blaze::DynamicTensor<double> local_arg =
                retiling_calculation::retile3d_calculation<double>(
                    std::move(arg), arg_locs, des_page_start, des_page_stop,
                    des_row_start, des_row_stop, des_col_start, des_col_stop,
                    name_, codename_);

            // padding has no effect on data parallelization
            primitive_argument_type local_result = common::conv1d_all_paddings(
                ir::node_data<double>(std::move(local_arg)), std::move(kernel),
                std::move(padding));

            res_page_start = des_page_start;
            res_page_stop = des_page_stop;
            res_row_start = res_col_start = 0;
            res_col_stop = cols_dim_k;
            // getting res_row_stop considering the padding
            res_row_stop = extract_numeric_value_dimensions(
                local_result, name_, codename_)[1];

            tiling_information_3d tile_info = tiling_information_3d(
                tiling_span(res_page_start, res_page_stop),
                tiling_span(res_row_start, res_row_stop),
                tiling_span(res_col_start, res_col_stop));
            auto attached_annotation = std::make_shared<annotation>(
                        localities_annotation(locality_ann,
                            tile_info.as_annotation(name_, codename_),
                            arg_locs.annotation_, name_, codename_));

            // construct new tiling annotation
            local_result.set_annotation(attached_annotation);
            return local_result;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "dist_conv1d::eval",
            generate_error_message(
                "strides > 1 not supported in conjunction with "
                "dilation_rate > 1"));
    }

    execution_tree::primitive_argument_type dist_conv1d::conv1d_all_paddings(
        execution_tree::primitive_argument_type&& arg,
        execution_tree::primitive_argument_type&& kernel,
        std::string&& padding, std::string&& par_mode) const
    {
        using namespace execution_tree;
        if (arg.has_annotation() /*&& kernel.has_annotation()*/)
        {
            localities_information arg_locs =
                extract_localities_information(arg, name_, codename_);
            localities_information kernel_locs =
                extract_localities_information(kernel, name_, codename_);
            return conv1d_all_paddings(
                extract_numeric_value(std::move(arg), name_, codename_),
                extract_numeric_value(std::move(kernel), name_, codename_),
                std::move(arg_locs), std::move(kernel_locs), std::move(padding),
                std::move(par_mode));
        }

        return common::conv1d_all_paddings(
            extract_numeric_value(std::move(arg), name_, codename_),
            extract_numeric_value(std::move(kernel), name_, codename_),
            std::move(padding));
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
            hpx::util::unwrapping([this_ = std::move(this_)](
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

                std::string par_mode = "data";
                if (valid(args[5]))
                {
                    par_mode = extract_string_value_strict(
                        args[5], this_->name_, this_->codename_);

                    if (par_mode != "data" && par_mode != "spatial" &&
                        par_mode != "data_spatial")
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "dist_conv1d::eval",
                            this_->generate_error_message(
                                "invalid parallelization mode. It can be one "
                                "of the `data`, `spatial`, or `data_spatial`"));
                    }
                }

                if (strides == 1 && dilation_rate == 1)
                {
                    return this_->conv1d_all_paddings(std::move(args[0]),
                        std::move(args[1]), std::move(padding),
                        std::move(par_mode));
                }
                //if (dilation_rate == 1) // strides > 1
                //{
                //    return this_->conv1d_any_pad(
                //        extract_numeric_value(
                //            std::move(args[0]), this_->name_, this_->codename_),
                //        extract_numeric_value(
                //            std::move(args[1]), this_->name_, this_->codename_),
                //        std::move(padding), strides);
                //}

                //// strides == 1 and dilation_rate > 1
                //return this_->conv1d_any_pad_dilation(
                //    extract_numeric_value(
                //        std::move(args[0]), this_->name_, this_->codename_),
                //    extract_numeric_value(
                //        std::move(args[1]), this_->name_, this_->codename_),
                //    std::move(padding), dilation_rate);

                                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dist_conv1d::eval",
                        this_->generate_error_message(
                            "strides > 1 not supported in conjunction with "
                            "dilation_rate > 1"));
            }),
            execution_tree::primitives::detail::map_operands(operands,
                execution_tree::functional::value_operand{}, args, name_,
                codename_, std::move(ctx)));
    }
}}}
