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
            __arg(_4_strides, 1),
            __arg(_5_dilation_rate, 1))
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

    //template <typename Vector1, typename Vector2>
    //double conv2d_operation::convolve_step(const Vector1& v1, const Vector2& v2,
    //    std::int64_t dilation_rate, std::size_t kernel_size) const
    //{
    //    return blaze::sum(blaze::elements(
    //                          v1,
    //                          [dilation_rate, kernel_size](
    //                              std::size_t i) { return i * dilation_rate; },
    //                          kernel_size) *
    //        v2);
    //}

    //template <typename Vector1, typename Vector2>
    //double conv2d_operation::convolve_step(const Vector1& v1, const Vector2& v2,
    //    std::int64_t dilation_rate, std::size_t kernel_size,
    //    std::size_t r) const
    //{
    //    return blaze::sum(blaze::elements(
    //                          v1,
    //                          [dilation_rate, kernel_size, r](std::size_t i) {
    //                              return i * dilation_rate + r;
    //                          },
    //                          kernel_size) *
    //        v2);
    //}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv2d_operation::conv2d_valid(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel) const
    {
        auto m = arg.matrix();
        auto k = kernel.matrix();
        std::size_t krows = k.rows();
        std::size_t kcolumns = k.columns();
        std::size_t result_rows = m.rows() - krows + 1;
        std::size_t result_columns = m.columns() - kcolumns + 1;

        blaze::DynamicMatrix<double> result(result_rows, result_columns);

        for (std::size_t i = 0; i != result_rows; ++i)
            for (std::size_t j = 0; j != result_columns; ++j)
                result(i, j) = convolve_step(
                    blaze::submatrix(m, i, j, krows, kcolumns), k);

        return primitive_argument_type{std::move(result)};
    }

    //primitive_argument_type conv2d_operation::conv2d_valid(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t strides) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    std::size_t k_size = kernel.size();
    //    std::size_t result_size =
    //        blaze::ceil(static_cast<float>(arg.size() - k_size + 1) /
    //            static_cast<float>(strides));

    //    blaze::DynamicVector<double> result(result_size);

    //    for (std::size_t i = 0; i != result_size; ++i)
    //        result[i] =
    //            convolve_step(blaze::subvector(v, i * strides, k_size), k);

    //    return primitive_argument_type{std::move(result)};
    //}

    //primitive_argument_type conv2d_operation::conv2d_valid_dilation(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t dilation_rate) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    std::size_t k_size = kernel.size();
    //    std::size_t dilated_k_size = dilation_rate * (k_size - 1) + 1;
    //    std::size_t result_size = arg.size() - dilated_k_size + 1;

    //    blaze::DynamicVector<double> result(result_size);

    //    for (std::size_t i = 0; i != result_size; ++i)
    //        result[i] = convolve_step(blaze::subvector(v, i, dilated_k_size), k,
    //            dilation_rate, k_size);

    //    return primitive_argument_type{std::move(result)};
    //}

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

    //primitive_argument_type conv2d_operation::conv2d_same(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t strides) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    std::size_t k_size = kernel.size();
    //    std::size_t v_size = arg.size();
    //    std::size_t pad_width;
    //    std::int64_t i_rel;

    //    if (v_size % strides == 0)
    //        pad_width =
    //            (blaze::max)(k_size - strides, static_cast<std::size_t>(0));

    //    else
    //        pad_width = (blaze::max)(
    //            k_size - (v_size % strides), static_cast<std::size_t>(0));

    //    std::size_t result_size = blaze::ceil(
    //        static_cast<float>(v_size + pad_width - k_size + 1) /
    //        static_cast<float>(strides));

    //    blaze::DynamicVector<double> result(result_size);

    //    std::size_t pad_left = pad_width / 2;

    //    for (std::size_t i = 0; i != result_size; ++i)
    //    {
    //        i_rel = i * strides - pad_left;
    //        if (i_rel < 0)
    //        {
    //            result[i] =
    //                convolve_step(blaze::subvector(v, 0, k_size + i_rel),
    //                    blaze::subvector(k, -i_rel, k_size + i_rel));
    //        }
    //        else if (i_rel > static_cast<std::int64_t>(v_size) -
    //                static_cast<std::int64_t>(k_size))
    //        {
    //            result[i] =
    //                convolve_step(blaze::subvector(v, i_rel, v_size - i_rel),
    //                    blaze::subvector(k, 0, v_size - i_rel));
    //        }
    //        else
    //        {
    //            result[i] =
    //                convolve_step(blaze::subvector(v, i_rel, k_size), k);
    //        }
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    //primitive_argument_type conv2d_operation::conv2d_same_dilation(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::int64_t dilation_rate) const
    //{
    //    auto v = arg.vector();
    //    auto k = kernel.vector();
    //    std::size_t k_size = kernel.size();
    //    std::size_t dilated_k_size = dilation_rate * (k_size - 1) + 1;
    //    std::size_t v_size = arg.size();
    //    std::size_t pad_left = (dilated_k_size - 1) / 2;
    //    std::int64_t i_rel;

    //    blaze::DynamicVector<double> result(v_size);

    //    for (std::size_t i = 0; i != v_size; ++i)
    //    {
    //        i_rel = i - pad_left;
    //        if (i_rel < 0)
    //        {
    //            std::size_t kernel_size;
    //            std::size_t remainder = -i_rel % dilation_rate;
    //            if (dilated_k_size + i_rel > v_size)
    //            {
    //                kernel_size =
    //                    blaze::ceil(static_cast<float>(v_size - remainder) /
    //                        static_cast<float>(dilation_rate));
    //                if (remainder == 0)
    //                    result[i] = convolve_step(v,
    //                        blaze::subvector(k,
    //                            blaze::ceil(static_cast<float>(-i_rel) /
    //                                static_cast<float>(dilation_rate)),
    //                            kernel_size),
    //                        dilation_rate, kernel_size);
    //                else
    //                    result[i] = convolve_step(v,
    //                        blaze::subvector(k,
    //                            blaze::ceil(static_cast<float>(-i_rel) /
    //                                static_cast<float>(dilation_rate)),
    //                            kernel_size),
    //                        dilation_rate, kernel_size,
    //                        dilation_rate - remainder);
    //            }
    //            else
    //            {
    //                kernel_size =
    //                blaze::ceil(static_cast<float>(dilated_k_size + i_rel) /
    //                    static_cast<float>(dilation_rate));
    //                if (remainder == 0)
    //                    result[i] = convolve_step(
    //                        blaze::subvector(v, 0, dilated_k_size + i_rel),
    //                        blaze::subvector(k,
    //                            blaze::ceil(static_cast<float>(-i_rel) /
    //                                static_cast<float>(dilation_rate)),
    //                            kernel_size),
    //                        dilation_rate, kernel_size);
    //                else
    //                    result[i] = convolve_step(
    //                        blaze::subvector(v, 0, dilated_k_size + i_rel),
    //                        blaze::subvector(k,
    //                            blaze::ceil(static_cast<float>(-i_rel) /
    //                                static_cast<float>(dilation_rate)),
    //                            kernel_size),
    //                        dilation_rate, kernel_size,
    //                        dilation_rate - remainder);
    //            }
    //        }
    //        else if (i_rel > static_cast<std::int64_t>(v_size) -
    //                static_cast<std::int64_t>(dilated_k_size))
    //        {
    //            std::size_t kernel_size =
    //                blaze::ceil(static_cast<float>(v_size - i_rel) /
    //                    static_cast<float>(dilation_rate));
    //            result[i] = convolve_step(
    //                blaze::subvector(v, i_rel, v_size - i_rel),
    //                blaze::subvector(k, 0, kernel_size),
    //                dilation_rate, kernel_size);
    //        }
    //        else
    //        {
    //            result[i] =
    //                convolve_step(blaze::subvector(v, i_rel, dilated_k_size), k,
    //                    dilation_rate, k_size);
    //        }
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

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

    //primitive_argument_type conv2d_operation::conv2d_any_pad(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::string&& padding, std::int64_t strides) const
    //{
    //    if (padding == "valid")
    //        return conv2d_valid(std::move(arg), std::move(kernel), strides);

    //    else if (padding == "same")
    //        return conv2d_same(std::move(arg), std::move(kernel), strides);

    //    // padding == causal
    //    return conv2d_causal(std::move(arg), std::move(kernel), strides);
    //}

    //primitive_argument_type conv2d_operation::conv2d_any_pad_dilation(
    //    ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
    //    std::string&& padding, std::int64_t dilation_rate) const
    //{
    //    if (padding == "valid")
    //        return conv2d_valid_dilation(
    //            std::move(arg), std::move(kernel), dilation_rate);

    //    else if (padding == "same")
    //        return conv2d_same_dilation(
    //            std::move(arg), std::move(kernel), dilation_rate);

    //    // padding == causal
    //    return conv2d_causal_dilation(
    //        std::move(arg), std::move(kernel), dilation_rate);
    //}

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

                std::string padding = "valid";
                if (args.size() > 2)
                {
                    padding = extract_string_value(
                        args[2], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same" &&
                        padding != "causal")
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv2d_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either valid "
                                "or same"));
                }

                //if (padding == "valid")
                //{
                //    if (extract_numeric_value_dimension(
                //            args[0], this_->name_, this_->codename_) <
                //        extract_numeric_value_dimension(
                //            args[1], this_->name_, this_->codename_))
                //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                //            "conv2d_operation::eval",
                //            this_->generate_error_message(
                //                "the kernel size cannot be greater than the "
                //                "array size in the valid padding mode"));
                //}

                ir::range strides(0);    // an empty range
                if (args.size() > 3)
                {
                    if (is_list_operand_strict(args[3]))
                    {
                        ir::range s = extract_list_value(
                            args[3], this_->name_, this_->codename_);
                        if (s.size() == 1)
                            strides =
                                extract_scalar_integer_value_strict(*s.begin());
                        else
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "conv2d_operation::eval",
                                this_->generate_error_message(
                                    "conv2d_operation requires the strides to "
                                    "be of rank 2"));
                    }
                    //else
                    //    strides = extract_scalar_integer_value_strict(
                    //        args[3], this_->name_, this_->codename_);

                    //if (strides <= 0)
                    //    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    //        "conv2d_operation::eval",
                    //        this_->generate_error_message(
                    //            "invalid strides. Strides must be positive"));
                }

                ir::range dilation_rate(0);    // an empty range
                if (args.size() > 4)
                {
                    //if (is_list_operand_strict(args[4]))
                    //{
                    //    ir::range d = extract_list_value(
                    //        args[4], this_->name_, this_->codename_);
                    //    if (d.size() == 1)
                    //        dilation_rate =
                    //            extract_scalar_integer_value_strict(*d.begin());
                    //    else
                    //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    //            "conv2d_operation::eval",
                    //            this_->generate_error_message(
                    //                "conv2d_operation requires the "
                    //                "dilation_rate to be of rank 1"));
                    //}
                    //else
                    //    dilation_rate = extract_scalar_integer_value_strict(
                    //        args[4], this_->name_, this_->codename_);

                    //if (dilation_rate <= 0)
                    //    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    //        "conv2d_operation::eval",
                    //        this_->generate_error_message(
                    //            "dilation_rate must be positive"));

                    //if (strides != 1 && dilation_rate != 1)
                    //    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    //        "conv2d_operation::eval",
                    //        this_->generate_error_message(
                    //            "strides > 1 not supported in conjunction with "
                    //            "dilation_rate > 1"));
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
                //else if (dilation_rate == 1)
                //{
                //    return this_->conv2d_any_pad(
                //        extract_numeric_value(
                //            std::move(args[0]), this_->name_, this_->codename_),
                //        extract_numeric_value(
                //            std::move(args[1]), this_->name_, this_->codename_),
                //        std::move(padding), strides);
                //}
                //// strides == 1 and dilation_rate > 1
                //return this_->conv2d_any_pad_dilation(
                //    extract_numeric_value(
                //        std::move(args[0]), this_->name_, this_->codename_),
                //    extract_numeric_value(
                //        std::move(args[1]), this_->name_, this_->codename_),
                //    std::move(padding), dilation_rate);

            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
