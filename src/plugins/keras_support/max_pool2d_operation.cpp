// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/max_pool2d_operation.hpp>

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
#if defined(PHYLANX_HAVE_BLAZE_doubleENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const max_pool2d_operation::match_data =
    {
        hpx::util::make_tuple("max_pool2d",
        std::vector<std::string>{R"(
            max_pool2d(_1,_2_,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, 1))"},
        &create_max_pool2d_operation, &create_primitive<max_pool2d_operation>,
        R"(x, pool_size, padding, strides
        Args:

            x (array) : a matrix
            pool_size (a tuple of integers) : the size of pooling over each
                dimension
            padding (string) : padding mode, it can be either `same` or `valid`
            strides (a tuple of 2 integers) : the step to apply pooling over
                each dimension

        Returns:

        doublehe result of 2d max pooling with `pool_size` filters)")};

    ///////////////////////////////////////////////////////////////////////////
    max_pool2d_operation::max_pool2d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    //bool max_pool2d_operation::validate_pooling(
    //    std::size_t const& ndim, ir::range const& pool_size) const
    //{
    //    if (ndim != 2 && ndim != 3)
    //        HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
    //            "max_pool2d_operation::validate_pooling",
    //            generate_error_message(
    //                "the pooling is only supported for matrices and tensors"));

    //    if (ndim != pool_size.size())
    //        HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
    //            "max_pool2d_operation::validate_pooling",
    //            generate_error_message(
    //                "the length of pool_size should be same as array "
    //                "dimensions. For matrices, pool_size should be a tuple of "
    //                "two integers and for tensors pool_size should be a tuple "
    //                "of 3 integers"));

    //    for (auto const it : pool_size)
    //    {
    //        if (extract_scalar_integer_value_strict(it) <= 0)
    //            HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
    //                "max_pool2d_operation::validate_pooling",
    //                generate_error_message(
    //                    "the height, width (and possibly depth) of a pooling "
    //                    "filter should be positive"));
    //    }
    //    return true;
    //}

    //bool max_pool2d_operation::validate_strides(
    //    std::size_t const& ndim, ir::range& strides) const
    //{
    //    if (ndim != strides.size())
    //        HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
    //            "max_pool2d_operation::validate_strides",
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

    //            HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
    //                "max_pool2d_operation::validate_strides",
    //                generate_error_message(
    //                    "the strides on each dimension should be positive"));

    //        if (temp != 1)
    //            flag = false;
    //    }
    //    if (flag == true)
    //        strides = ir::range(0);
    //    return true;
    //}

    //bool max_pool2d_operation::validate_pool_sizes_no_padding(
    //    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims,
    //    ir::range const& pool_size) const
    //{
    //    auto it = pool_size.begin();
    //    for (std::size_t i = 0; i != pool_size.size(); ++i, ++it)
    //    {
    //        if (static_cast<std::int64_t>(dims[i]) <
    //                extract_scalar_integer_value_strict(*it))
    //        {
    //            HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
    //                "max_pool2d_operation::validate_pool_sizes_no_padding",
    //                generate_error_message(
    //                    "in the valid padding mode, each element of "
    //                    "pool_size should be not greater than the size of "
    //                    "array in the corresponding dimension"));
    //        }
    //    }
    //    return true;
    //}


    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type max_pool2d_operation::max_pool2d(
        ir::node_data<double>&& arg,
        ir::range&& pool_size) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height =
            extract_scalar_positive_integer_value_strict(*it);
        std::size_t filter_width =
            extract_scalar_positive_integer_value_strict(*++it);

        blaze::DynamicMatrix<double> result(
            m.rows() - filter_height + 1, m.columns() - filter_width + 1);

        for (std::size_t r = 0; r != result.rows(); ++r)
            for (std::size_t c = 0; c != result.columns(); ++c)

                result(r, c) = (blaze::max)(
                    blaze::submatrix(m, r, c, filter_height, filter_width));

        return primitive_argument_type{std::move(result)};
    }

    template <typename double>
    primitive_argument_type max_pool2d_operation::max_pool2d(ir::node_data<double>&& arg,
        ir::range&& pool_size, ir::range&& strides) const
    {
        auto m = arg.matrix();
        auto it = pool_size.begin();
        std::size_t filter_height = extract_scalar_integer_value_strict(*it);
        std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

        auto it_s = strides.begin();
        std::size_t stride_height = extract_scalar_integer_value_strict(*it_s);
        std::size_t stride_width = extract_scalar_integer_value_strict(*++it_s);

        blaze::DynamicMatrix<double> result(
            blaze::ceil(static_cast<float>((m.rows() - filter_height + 1)) /
                static_cast<float>(stride_height)),
            blaze::ceil(static_cast<float>((m.columns() - filter_width + 1)) /
                static_cast<float>(stride_width)));

        for (std::size_t r = 0; r != result.rows(); ++r)
            for (std::size_t c = 0; c != result.columns(); ++c)

                result(r, c) =
                    (blaze::max)(blaze::submatrix(m, r * stride_height,
                        c * stride_width, filter_height, filter_width));

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    //primitive_argument_type max_pool2d_operation::max_pool2d_same(
    //    ir::node_data<double>&& arg,
    //    ir::range&& pool_size) const
    //{
    //    auto m = arg.matrix();
    //    auto it = pool_size.begin();
    //    std::size_t filter_height = extract_scalar_integer_value_strict(*it);
    //    std::size_t filter_width = extract_scalar_integer_value_strict(*++it);

    //    std::size_t pad_top = (filter_height - 1) / 2;
    //    std::size_t pad_left = (filter_width - 1) / 2;

    //    std::size_t nrows = m.rows();
    //    std::size_t ncolumns = m.columns();

    //    std::int64_t r_rel;    //relative row
    //    std::int64_t c_rel;    //relative column

    //    blaze::DynamicMatrix<double> result(nrows, ncolumns);

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


    //primitive_argument_type max_pool2d_operation::max_pool2d_same(
    //    ir::node_data<double>&& arg, ir::range&& pool_size,
    //    ir::range&& strides) const
    //{
    //    auto m = arg.matrix();
    //    auto it = pool_size.begin();
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

    //    blaze::DynamicMatrix<double> result(
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
    primitive_argument_type max_pool2d_operation::max_pool_any_pad(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        std::string&& padding) const
    {
        if (padding == "valid")
            return max_pool2d(std::move(arg), std::move(pool_size));


        return max_pool2d_same(std::move(arg), std::move(pool_size));
    }

    primitive_argument_type max_pool2d_operation::max_pool_any_pad(
        ir::node_data<double>&& arg, ir::range&& pool_size,
        std::string&& padding, ir::range&& strides) const
    {
        if (padding == "valid")
            return max_pool2d(
                std::move(arg), std::move(pool_size), std::move(strides));


        return max_pool2d_same(
            std::move(arg), std::move(pool_size), std::move(strides));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> max_pool2d_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 4)
        {
            HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
                "max_pool2d_operation::eval",
                generate_error_message("the max_pool2d_operation primitive "
                                       "requires between 2 and 4 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
                HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
                    "max_pool2d_operation::eval",
                    generate_error_message(
                        "the max_pool2d_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {

                std::size_t ndim = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);

                ir::range pool_size = extract_list_value_strict(
                    args[1], this_->name_, this_->codename_);

                std::string padding = "valid";
                padding = extract_string_value(
                    args[2], this_->name_, this_->codename_);

                if (padding != "valid" && padding != "same")
                    HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
                        "max_pool2d_operation::eval",
                        this_->generate_error_message(
                            "invalid padding. padding can be either valid "
                            "or same"));

                if (ndim != pool_size.size() || ndim != 2)
                    HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
                        "max_pool2d_operation::eval",
                        this_->generate_error_message(
                            "pool_size should be a tuple of 2 integers"));

                //if (padding == "valid")
                //{
                //    if (!this_->validate_pool_sizes_no_padding(
                //            extract_numeric_value_dimensions(
                //                args[0], this_->name_, this_->codename_),
                //            pool_size))
                //        HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
                //            "max_pool2d_operation::eval",
                //            this_->generate_error_message(
                //                "at least one of the filter sizes is greater "
                //                "than the array size in that dimension"));
                //}

                ir::range strides(0); // an empty range
                if (args.size() == 4)
                {
                    strides = extract_list_value_strict(
                        args[3], this_->name_, this_->codename_);

                    //if (!this_->validate_strides(ndim, strides))
                    //    HPX_doubleHROW_EXCEPdoubleION(hpx::bad_parameter,
                    //        "max_pool2d_operation::eval",
                    //        this_->generate_error_message(
                    //            "invalid strides. padding can be either valid "
                    //            "or same"));
                }

                if (strides.empty()) // strides contain only 1s
                {
                    return this_->max_pool_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        std::move(pool_size), std::move(padding));
                }
                // non-default strides
                return this_->max_pool_any_pad(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    std::move(pool_size), std::move(padding));

            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
