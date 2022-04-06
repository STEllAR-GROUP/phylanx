// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/resize_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <algorithm>
#include <cmath>
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
    match_pattern_type const resize_operation::match_data = {
        hpx::make_tuple("resize_images",
            std::vector<std::string>{R"(
                resize_images(_1,
                _2_height_factor,
                _3_width_factor,
                __arg(_4_interpolation,"nearest")))"},
            &create_resize_operation, &create_primitive<resize_operation>,
            R"(a
            Args:

                a (array_like) : input 4d array
                height_factor (integer) : enlargement factor for height
                width_factor (integer) : enlargement factor for width
                interpolation (string) : interpolation type, "bilinear" or
                    "nearest"

            Returns:

            Returns a resized version of the array with height_factor and
            width_factor using the specified interpolation .)")};

    ///////////////////////////////////////////////////////////////////////////
    resize_operation::resize_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type resize_operation::nearest(ir::node_data<T>&& arg,
        std::int64_t height_factor, std::int64_t width_factor,
        std::string interpolation) const
    {
        auto q = arg.quatern();

        std::size_t batch = q.quats();
        std::size_t channels = q.columns();
        std::size_t height = q.pages();
        std::size_t width = q.rows();
        std::size_t result_height = height * height_factor;
        std::size_t result_width = width * width_factor;

        blaze::DynamicArray<4UL, T> result(
            batch, result_height, result_width, channels);
        std::size_t num_elements = result_height * result_width;
        std::size_t fx;
        std::size_t dx;
        std::size_t fy;
        std::size_t dy;

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto res_tensor = blaze::quatslice(result, l);
            auto t = blaze::quatslice(q, l);
            for (std::size_t j = 0; j != channels; ++j)
            {
                auto res_slice = blaze::columnslice(res_tensor, j);
                auto m = blaze::columnslice(t, j);
                for (std::size_t i = 0; i != num_elements; ++i)
                {
                    fx = i / result_width;
                    dx = fx / height_factor;
                    fy = i - fx * result_width;
                    dy = fy / width_factor;

                    res_slice(fx, fy) = m(dx, dy);
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type resize_operation::bilinear(
        ir::node_data<double>&& arg, std::int64_t height_factor,
        std::int64_t width_factor, std::string interpolation) const
    {
        auto q = arg.quatern();

        std::size_t batch = q.quats();
        std::size_t channels = q.columns();
        std::size_t height = q.pages();
        std::size_t width = q.rows();
        std::size_t result_height = height * height_factor;
        std::size_t result_width = width * width_factor;

        blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
            batch, result_height, result_width, channels);

        std::size_t num_elements = result_height * result_width;
        std::size_t fx;
        std::size_t dx;
        std::size_t rx;
        std::size_t fy;
        std::size_t dy;
        std::size_t ry;
        std::size_t bx;
        std::size_t by;
        double ul;
        double ur;
        double ll;
        double lr;
        double tmp;

        for (std::size_t l = 0; l != batch; ++l)
        {
            auto res_tensor = blaze::quatslice(result, l);
            auto t = blaze::quatslice(q, l);
            for (std::size_t j = 0; j != channels; ++j)
            {
                auto res_slice = blaze::columnslice(res_tensor, j);
                auto m = blaze::columnslice(t, j);
                for (std::size_t i = 0; i < num_elements; ++i)
                {
                    fx = i / result_width;
                    fy = i - fx * result_width;

                    dx = fx / height_factor;
                    rx = fx % height_factor;
                    dy = fy / width_factor;
                    ry = fy % width_factor;

                    bx = 1;
                    by = 1;
                    if (dx == height - 1)
                        bx = 0;
                    if (dy == width - 1)
                        by = 0;

                    ul = m(dx, dy);
                    ur = m(dx, dy + by);
                    ll = m(dx + bx, dy);
                    lr = m(dx + bx, dy + by);
                    tmp = ry * (ur - ul) / width_factor;

                    res_slice(fx, fy) = ul + tmp +
                        rx * (ll - ul - tmp + ry * (lr - ll) / width_factor) /
                            height_factor;
                }
            }
        }
        return primitive_argument_type{std::move(result)};
    }

    hpx::future<primitive_argument_type> resize_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() != 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "resize_operation::eval",
                generate_error_message(
                    "the resize_operation primitive requires exactly "
                    "four operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "resize_operation::eval",
                generate_error_message(
                    "the resize_operation primitive requires that the "
                    "argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& arg,
                    std::int64_t height_factor, std::int64_t width_factor,
                    std::string interpolation) -> primitive_argument_type {
                    if (interpolation != "nearest" &&
                        interpolation != "bilinear")
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "resize_operation::eval",
                            this_->generate_error_message(
                                "interpolation type not supported"));
                    if (height_factor < 0 || width_factor < 0)
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "resize_operation::eval",
                            this_->generate_error_message(
                                "scaling factor should be positive"));

                    if (interpolation == "nearest")
                    {
                        switch (extract_common_type(arg))
                        {
                        case node_data_type_bool:
                            return this_->nearest(
                                extract_boolean_value(std::move(arg),
                                    this_->name_, this_->codename_),
                                std::move(height_factor),
                                std::move(width_factor),
                                std::move(interpolation));

                        case node_data_type_int64:
                            return this_->nearest(
                                extract_integer_value(std::move(arg),
                                    this_->name_, this_->codename_),
                                std::move(height_factor),
                                std::move(width_factor),
                                std::move(interpolation));

                        case node_data_type_unknown:
                            [[fallthrough]];
                        case node_data_type_double:
                            return this_->nearest(
                                extract_numeric_value(std::move(arg),
                                    this_->name_, this_->codename_),
                                std::move(height_factor),
                                std::move(width_factor),
                                std::move(interpolation));

                        default:
                            break;
                        }
                    }
                    else
                    {
                        return this_->bilinear(
                            extract_numeric_value(
                                std::move(arg), this_->name_, this_->codename_),
                            std::move(height_factor), std::move(width_factor),
                            std::move(interpolation));
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "resize_operation::eval",
                        this_->generate_error_message(
                            "the resize_operation primitive requires for all "
                            "arguments to "
                            "be numeric data types"));
                }),
            value_operand(operands[0], args, name_, codename_, ctx),
            scalar_integer_operand_strict(
                operands[1], args, name_, codename_, ctx),
            scalar_integer_operand_strict(
                operands[2], args, name_, codename_, ctx),
            string_operand(operands[3], args, name_, codename_, ctx));
    }
}}}
