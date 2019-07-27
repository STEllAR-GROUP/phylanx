// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/max_pool2d_operation.hpp>
#include <phylanx/plugins/keras_support/pool_indices_helper.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
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
    match_pattern_type const max_pool2d_operation::match_data =
    {
        hpx::util::make_tuple("max_pool2d",
        std::vector<std::string>{R"(
            max_pool2d(_1,_2_pool_size,
            __arg(_3_padding, "valid"),
            __arg(_4_strides, list(1,1))))"},
        &create_max_pool2d_operation, &create_primitive<max_pool2d_operation>,
        R"(x, pool_size, padding, strides
        Args:

            x (array) : a matrix
            pool_size (a tuple of integers) : the size of pooling over each
                dimension
            padding (optional, string) : padding mode, it can be either `same`
                or `valid`. `valid` by default.
            strides (optional, a tuple of 2 integers) : the step to apply
                pooling over each dimension. `(1, 1)` by default.

        Returns:

        The result of 2d max pooling with `pool_size` filters)")};

    ///////////////////////////////////////////////////////////////////////////
    max_pool2d_operation::max_pool2d_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type max_pool2d_operation::max_pool2d(
        ir::node_data<double>&& arg, std::size_t filter_height,
        std::size_t filter_width) const
    {
        auto m = arg.matrix();

        std::size_t result_height = m.rows() - filter_height + 1;
        std::size_t result_width = m.columns() - filter_width + 1;

        blaze::DynamicMatrix<double> result(result_height, result_width);

        for (std::size_t r = 0; r != result_height; ++r)
        {
            for (std::size_t c = 0; c != result_width; ++c)
            {
                result(r, c) = (blaze::max)(
                    blaze::submatrix(m, r, c, filter_height, filter_width));
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type max_pool2d_operation::max_pool2d(
        ir::node_data<double>&& arg, std::size_t filter_height,
        std::size_t filter_width, std::size_t stride_height,
        std::size_t stride_width) const
    {
        auto m = arg.matrix();

        std::size_t result_height = blaze::ceil(
            static_cast<double>(m.rows() - filter_height + 1) / stride_height);
        std::size_t result_width =
            blaze::ceil(static_cast<double>(m.columns() - filter_width + 1) /
                stride_width);

        blaze::DynamicMatrix<double> result(result_height, result_width);

        for (std::size_t r = 0; r != result_height; ++r)
        {
            for (std::size_t c = 0; c != result_width; ++c)
            {
                result(r, c) =
                    (blaze::max)(blaze::submatrix(m, r * stride_height,
                        c * stride_width, filter_height, filter_width));
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type max_pool2d_operation::max_pool2d_same(
        ir::node_data<double>&& arg,
         std::size_t filter_height, std::size_t filter_width) const
    {
        auto m = arg.matrix();

        std::int64_t pad_top = (filter_height - 1) / 2;
        std::int64_t pad_left = (filter_width - 1) / 2;

        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

        blaze::DynamicMatrix<double> result(nrows, ncolumns);

        for (std::size_t r = 0; r != nrows; ++r)
        {
            auto sub_row =
                pool_indices::get_subsizes(nrows, filter_height, r - pad_top);
            for (std::size_t c = 0; c != ncolumns; ++c)
            {
                auto sub_column = pool_indices::get_subsizes(
                    ncolumns, filter_width, c - pad_left);

                result(r, c) =
                    (blaze::max)(blaze::submatrix(m, sub_row.image_beg_,
                    sub_column.image_beg_, sub_row.size_, sub_column.size_));
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type max_pool2d_operation::max_pool2d_same(
        ir::node_data<double>&& arg, std::size_t filter_height,
        std::size_t filter_width, std::size_t stride_height,
        std::size_t stride_width) const
    {
        auto m = arg.matrix();

        std::size_t pad_height;
        std::size_t pad_width;

        std::size_t nrows = m.rows();
        std::size_t ncolumns = m.columns();

        if (nrows % stride_height == 0)
            pad_height = (blaze::max)(
                filter_height - stride_height, static_cast<std::size_t>(0));
        else
            pad_height = (blaze::max)(filter_height - (nrows % stride_height),
                static_cast<std::size_t>(0));

        if (ncolumns % stride_width == 0)
            pad_width = (blaze::max)(
                filter_width - stride_width, static_cast<std::size_t>(0));
        else
            pad_width = (blaze::max)(filter_width - (ncolumns % stride_width),
                static_cast<std::size_t>(0));

        std::size_t pad_top  = pad_height / 2;
        std::size_t pad_left = pad_width  / 2;

        std::size_t result_height = blaze::ceil(
            static_cast<double>(nrows + pad_height - filter_height + 1) /
            stride_height);
        std::size_t result_width = blaze::ceil(
            static_cast<double>(ncolumns + pad_width - filter_width + 1) /
            stride_width);

        blaze::DynamicMatrix<double> result(result_height, result_width);

        for (std::size_t r = 0; r != result_height; ++r)
        {
            auto sub_row = pool_indices::get_subsizes(
                nrows, filter_height, r * stride_height - pad_top);
            for (std::size_t c = 0; c != result_width; ++c)
            {
                auto sub_column = pool_indices::get_subsizes(
                    ncolumns, filter_width, c * stride_width - pad_left);

                result(r, c) =
                    (blaze::max)(blaze::submatrix(m, sub_row.image_beg_,
                    sub_column.image_beg_, sub_row.size_, sub_column.size_));
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type max_pool2d_operation::max_pool_any_pad(
        ir::node_data<double>&& arg, std::size_t filter_height,
        std::size_t filter_width, std::string&& padding) const
    {
        if (padding == "valid")
            return max_pool2d(std::move(arg), filter_height, filter_width);

        // padding == "same"
        return max_pool2d_same(std::move(arg), filter_height, filter_width);
    }

    primitive_argument_type max_pool2d_operation::max_pool_any_pad(
        ir::node_data<double>&& arg, std::size_t filter_height,
        std::size_t filter_width, std::string&& padding,
        std::size_t stride_height, std::size_t stride_width) const
    {
        if (padding == "valid")
            return max_pool2d(std::move(arg), filter_height, filter_width,
                stride_height, stride_width);

        // padding == "same"
        return max_pool2d_same(std::move(arg), filter_height, filter_width,
            stride_height, stride_width);
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> max_pool2d_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "max_pool2d_operation::eval",
                generate_error_message("the max_pool2d_operation primitive "
                                       "requires between 2 and 4 operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
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

                if (pool_size.size() != ndim || ndim != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "max_pool2d_operation::eval",
                        this_->generate_error_message(
                            "the length of pool_size should be same as array "
                            "dimensions. pool2d operates on matrices and "
                            "requires a 2d pool_size"));
                }

                auto it = pool_size.begin();
                std::size_t filter_height =
                    extract_scalar_positive_integer_value_strict(*it);
                std::size_t filter_width =
                    extract_scalar_positive_integer_value_strict(*++it);

                std::string padding = "valid";
                if (args.size() > 2)
                {
                    padding = extract_string_value(
                        args[2], this_->name_, this_->codename_);

                    if (padding != "valid" && padding != "same")
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "max_pool2d_operation::eval",
                            this_->generate_error_message(
                                "invalid padding. Padding can be either "
                                "`valid` or `same`"));
                }

                if (padding == "valid")
                {
                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims =
                        extract_numeric_value_dimensions(
                            args[0], this_->name_, this_->codename_);
                    if (filter_height > dims[0] || filter_width > dims[1])
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "max_pool2d_operation::eval",
                            this_->generate_error_message(
                                "at least one of the filter sizes is greater "
                                "than the array size in the corresponding "
                                "dimension"));
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
                            "max_pool2d_operation::eval",
                            this_->generate_error_message(
                                "pool2d requires strides to be a tuple of 2 "
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

                if (strides.empty()) // strides contain only 1s
                {
                    return this_->max_pool_any_pad(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        filter_height, filter_width, std::move(padding));
                }

                // non-default strides
                return this_->max_pool_any_pad(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    filter_height, filter_width, std::move(padding),
                    stride_height, stride_width);

            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
