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

#include <array>
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
    match_pattern_type const conv1d_operation::match_data =
    {
        hpx::util::make_tuple("conv1d",
        std::vector<std::string>{"conv1d(_1, _2)", "conv1d(_1, _2, _3)",
        "conv1d(_1, _2, _3, _4)", "conv1d(_1, _2, _3, _4, _5)"},
        &create_conv1d_operation, &create_primitive<conv1d_operation>,
        R"(x, kernel, padding, strides, dilation_rate
        Args:

            x (array) : a vector
            kernel (array) : a vector
            padding (string) : padding mode, `valid` by default. It can be
                either `valid`, `same` or `causal`
            strides (integer) : the step to apply convolution over x
            dilation_rate (integer) : indicates the dilation rate

        Returns:

        1D convolution)")
    };

    ///////////////////////////////////////////////////////////////////////////
    conv1d_operation::conv1d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    //bool conv1d_operation::validate_strides(
    //    std::size_t const& ndim, ir::range& strides) const
    //{
    //    if (ndim != strides.size())
    //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
    //            "conv1d_operation::validate_strides",
    //            generate_error_message(
    //                "the length of strides should be same as array "
    //                "dimensions. For matrices, strides should be a tuple of "
    //                "two positive integers and for tensors strides should be a "
    //                "tuple of three positive integers"));

    //    bool flag = true;
    //    for (auto const it : strides)
    //    {
    //        std::int64_t temp = extract_scalar_integer_value_strict(it);
    //        if (temp <= 0)

    //            HPX_THROW_EXCEPTION(hpx::bad_parameter,
    //                "conv1d_operation::validate_strides",
    //                generate_error_message(
    //                    "the strides on each dimension should be positive"));

    //        if (temp != 1)
    //            flag = false;
    //    }
    //    if (flag == true)
    //        strides = ir::range(0);
    //    return true;
    //}


    ///////////////////////////////////////////////////////////////////////////
    template <typename Vector1, typename Vector2>
    double conv1d_operation::convolve_step(
        const Vector1& v1, const Vector2& v2) const
    {
        return blaze::sum(v1 * v2);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv1d_operation::conv1d_valid(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel) const
    {
        auto v = arg.vector();
        auto k = kernel.vector();
        std::size_t k_size = kernel.size();
        std::size_t result_size = arg.size() - k_size + 1;

        blaze::DynamicVector<double> result(result_size);

        for (std::size_t i = 0; i != result_size; ++i)
            result[i] = convolve_step(blaze::subvector(v, i, k_size), k);

        return primitive_argument_type{std::move(result)};
    }

    //template <typename T>
    //primitive_argument_type conv1d_operation::max_conv1d2d(ir::node_data<T>&& arg,
    //    ir::range&& conv1d_size, ir::range&& strides) const
    //{
    //    auto m = arg.matrix();
    //    auto it = conv1d_size.begin();
    //    std::size_t filter_height = extract_scalar_integer_value_strict(*it);
    //    std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

    //    auto it_s = strides.begin();
    //    std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
    //    std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

    //    blaze::DynamicMatrix<T> result(
    //        blaze::ceil(static_cast<float>((m.rows() - filter_height + 1)) /
    //            static_cast<float>(stride_height)),
    //        blaze::ceil(static_cast<float>((m.columns() - filter_width + 1)) /
    //            static_cast<float>(stride_width)));

    //    for (std::size_t r = 0; r != result.rows(); ++r)
    //        for (std::size_t c = 0; c != result.columns(); ++c)

    //            result(r, c) =
    //                (blaze::max)(blaze::submatrix(m, r * stride_height,
    //                    c * stride_width, filter_height, filter_width));

    //    return primitive_argument_type{std::move(result)};
    //}

    /////////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv1d_operation::conv1d_same(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel) const
    {
        auto v = arg.vector();
        auto k = kernel.vector();
        std::size_t k_size = kernel.size();
        std::size_t v_size = arg.size();
        std::size_t pad_left = (k_size - 1) / 2;
        std::int64_t i_rel;

        blaze::DynamicVector<double> result(v_size);

        for (std::size_t i = 0; i != v_size; ++i)
        {
            i_rel = i - pad_left;
            if (i_rel < 0)
            {
                result[i] =
                    convolve_step(blaze::subvector(v, 0, k_size + i_rel),
                        blaze::subvector(k, -i_rel, k_size + i_rel));
            }
            else if (i_rel > v_size - k_size)
            {
                result[i] =
                    convolve_step(blaze::subvector(v, i_rel, v_size - i_rel),
                        blaze::subvector(k, 0, v_size - i_rel));
            }
            else
            {
                result[i] =
                    convolve_step(blaze::subvector(v, i_rel, k_size), k);
            }
        }

        return primitive_argument_type{std::move(result)};
    }
    //template <typename T>
    //primitive_argument_type conv1d_operation::max_conv1d2d_with_pad(
    //    ir::node_data<T>&& arg,
    //    ir::range&& conv1d_size) const
    //{
    //    auto m = arg.matrix();
    //    auto it = conv1d_size.begin();
    //    std::size_t filter_height = extract_scalar_integer_value_strict(*it);
    //    std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

    //    std::size_t pad_top = (filter_height - 1) / 2;
    //    std::size_t pad_left = (filter_width - 1) / 2;

    //    std::size_t nrows = m.rows();
    //    std::size_t ncolumns = m.columns();

    //    std::int64_t r_rel;    //relative row
    //    std::int64_t c_rel;    //relative column

    //    blaze::DynamicMatrix<T> result(nrows, ncolumns);

    //    for (std::size_t r = 0; r != nrows; ++r)
    //    {
    //        r_rel = r - pad_top;
    //        for (std::size_t c = 0; c != ncolumns; ++c)
    //        {
    //            c_rel = c - pad_left;
    //            if (r_rel < 0)
    //            {
    //                if (c_rel < 0)
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(m, 0, 0,
    //                        filter_height + r_rel, filter_width + c_rel));
    //                }
    //                else if (c_rel > static_cast<std::int64_t>(ncolumns) -
    //                        static_cast<std::int64_t>(filter_width))
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(m, 0,
    //                        c_rel, filter_height + r_rel, ncolumns - c_rel));
    //                }
    //                else
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, 0, c_rel, filter_height + r_rel, filter_width));
    //                }
    //            }
    //            else if (r_rel > static_cast<std::int64_t>(nrows) -
    //                    static_cast<std::int64_t>(filter_height))
    //            {
    //                if (c_rel < 0)
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, 0, nrows - r_rel, filter_width + c_rel));
    //                }
    //                else if (c_rel > static_cast<std::int64_t>(ncolumns) -
    //                        static_cast<std::int64_t>(filter_width))
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, nrows - r_rel, ncolumns - c_rel));
    //                }
    //                else
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, nrows - r_rel, filter_width));
    //                }
    //            }
    //            else
    //            {
    //                if (c_rel < 0)
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, 0, filter_height, filter_width + c_rel));
    //                }
    //                else if (c_rel > static_cast<std::int64_t>(ncolumns) -
    //                        static_cast<std::int64_t>(filter_width))
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, filter_height, ncolumns - c_rel));
    //                }
    //                else
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, filter_height, filter_width));
    //                }
    //            }
    //        }
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    //template <typename T>
    //primitive_argument_type conv1d_operation::max_conv1d2d_with_pad(
    //    ir::node_data<T>&& arg, ir::range&& conv1d_size,
    //    ir::range&& strides) const
    //{
    //    auto m = arg.matrix();
    //    auto it = conv1d_size.begin();
    //    std::size_t filter_height = extract_scalar_integer_value_strict(*it);
    //    std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

    //    auto it_s = strides.begin();
    //    std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
    //    std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

    //    std::size_t pad_top;
    //    std::size_t pad_left;
    //    std::size_t pad_height;
    //    std::size_t pad_width;

    //    std::size_t nrows = m.rows();
    //    std::size_t ncolumns = m.columns();

    //    if (nrows % stride_height == 0)
    //        pad_height = (blaze::max)(
    //            filter_height - stride_height, static_cast<std::size_t>(0));
    //    else
    //        pad_height = (blaze::max)(filter_height - (nrows % stride_height),
    //            static_cast<std::size_t>(0));

    //    if (ncolumns % stride_width == 0)
    //        pad_width = (blaze::max)(
    //            filter_width - stride_width, static_cast<std::size_t>(0));
    //    else
    //        pad_width = (blaze::max)(filter_width - (ncolumns % stride_width),
    //            static_cast<std::size_t>(0));

    //    pad_top = pad_height / 2;
    //    pad_left = pad_width / 2;

    //    std::int64_t r_rel;    //relative row
    //    std::int64_t c_rel;    //relative column

    //    blaze::DynamicMatrix<T> result(
    //        blaze::ceil(
    //            static_cast<float>(nrows + pad_height - filter_height + 1) /
    //            static_cast<float>(stride_height)),
    //        blaze::ceil(
    //            static_cast<float>(ncolumns + pad_width - filter_width + 1) /
    //            static_cast<float>(stride_width)));

    //    for (std::size_t r = 0; r != result.rows(); ++r)
    //    {
    //        r_rel = r * stride_height - pad_top;
    //        for (std::size_t c = 0; c != result.columns(); ++c)
    //        {
    //            c_rel = c * stride_width - pad_left;
    //            if (r_rel < 0)
    //            {
    //                if (c_rel < 0)
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(m, 0, 0,
    //                        filter_height + r_rel, filter_width + c_rel));
    //                }
    //                else if (c_rel > static_cast<std::int64_t>(ncolumns) -
    //                        static_cast<std::int64_t>(filter_width))
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(m, 0,
    //                        c_rel, filter_height + r_rel, ncolumns - c_rel));
    //                }
    //                else
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, 0, c_rel, filter_height + r_rel, filter_width));
    //                }
    //            }
    //            else if (r_rel > static_cast<std::int64_t>(nrows) -
    //                    static_cast<std::int64_t>(filter_height))
    //            {
    //                if (c_rel < 0)
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, 0, nrows - r_rel, filter_width + c_rel));
    //                }
    //                else if (c_rel > static_cast<std::int64_t>(ncolumns) -
    //                        static_cast<std::int64_t>(filter_width))
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, nrows - r_rel, ncolumns - c_rel));
    //                }
    //                else
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, nrows - r_rel, filter_width));
    //                }
    //            }
    //            else
    //            {
    //                if (c_rel < 0)
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, 0, filter_height, filter_width + c_rel));
    //                }
    //                else if (c_rel > static_cast<std::int64_t>(ncolumns) -
    //                        static_cast<std::int64_t>(filter_width))
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, filter_height, ncolumns - c_rel));
    //                }
    //                else
    //                {
    //                    result(r, c) = (blaze::max)(blaze::submatrix(
    //                        m, r_rel, c_rel, filter_height, filter_width));
    //                }
    //            }
    //        }
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv1d_operation::conv1d_causal(
        ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel) const
    {
        auto v = arg.vector();
        auto k = kernel.vector();
        std::size_t k_size = kernel.size();
        std::size_t v_size = arg.size();
        std::size_t pad_left = k_size - 1; //no pad_right
        std::int64_t i_rel;

        blaze::DynamicVector<double> result(v_size);

        for (std::size_t i = 0; i != v_size; ++i)
        {
            i_rel = i - pad_left;
            if (i_rel < 0)
            {
                result[i] =
                    convolve_step(blaze::subvector(v, 0, k_size + i_rel),
                        blaze::subvector(k, -i_rel, k_size + i_rel));
            }
            else
            {
                result[i] =
                    convolve_step(blaze::subvector(v, i_rel, k_size), k);
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type conv1d_operation::conv1d_any_pad(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::string&& padding) const
    {
        if (padding == "valid")
            return conv1d_valid(std::move(arg), std::move(kernel));

        else if (padding == "same")
            return conv1d_same(std::move(arg), std::move(kernel));

        // padding == causal
        return conv1d_causal(std::move(arg), std::move(kernel));
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
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "conv1d_operation::eval",
                    generate_error_message(
                        "the conv1d_operation primitive requires that the "
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
                    ndim != 1)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "conv1d_operation::eval",
                        this_->generate_error_message(
                            "conv1d operation requires for x and kernel to be "
                            "vectors"));

                std::string padding = "valid";
                if (args.size() > 2)
                {
                    padding = extract_string_value(
                        args[2], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same" &&
                        padding != "causal")
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either valid, "
                                "same or causal"));
                }

                if (padding == "valid")
                {
                    if (extract_numeric_value_dimension(
                            args[0], this_->name_, this_->codename_) <
                        extract_numeric_value_dimension(
                            args[1], this_->name_, this_->codename_))
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "the kernel size cannot be greater than the "
                                "array size in the valid padding mode"));
                }

                std::int64_t strides = 1;
                if (args.size() > 3)
                {
                    if (is_list_operand_strict(args[3]))
                    {
                        auto s = extract_list_value(args[3]);
                        strides =
                            extract_scalar_integer_value_strict(*s.begin());
                    }
                    else
                    {
                        strides = extract_scalar_integer_value(
                            args[3], this_->name_, this_->codename_);
                    }

                    if (strides <= 0)
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "conv1d_operation::eval",
                            this_->generate_error_message(
                                "invalid strides. Strides must be positive"));
                }

                if (strides == 1)
                {
                    return this_->conv1d_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(padding));
                }
                // non-default strides and dilation_rates

                    //return this_->avg_conv1d_nd(
                    //    extract_numeric_value(std::move(args[0]),
                    //        this_->name_, this_->codename_),
                    //    std::move(conv1d_size), std::move(padding),
                    //    std::move(strides));


            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
