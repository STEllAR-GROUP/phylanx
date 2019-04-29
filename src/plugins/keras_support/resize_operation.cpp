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
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cmath>
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
    match_pattern_type const resize_operation::match_data = {
        hpx::util::make_tuple("resize_images",
            std::vector<std::string>{"resize_images(_1, _2, _3, _4)"},
            &create_resize_operation, &create_primitive<resize_operation>,
            R"(a
            Args:

                a (array_like) : input array
                height_factor (integer) : enlargement factor for height
                width_factor (integer) : enlargement factor for width
                interpolation (string) : interpolation type, "bilinear" or "nearest"

            Returns:

            Returns a resized version of the array with height_factor and width_factor
            using the specified interpolation .)")};

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
        auto m = arg.matrix();
        blaze::DynamicMatrix<T> result(
            m.rows() * height_factor, m.columns() * width_factor);

        std::size_t num_elements =
            m.rows() * height_factor * m.columns() * width_factor;
        for (std::size_t i = 0; i < num_elements; ++i)
        {
            std::size_t fx = i / result.columns();
            std::size_t dx = fx / height_factor;
            std::size_t fy = i - fx * result.columns();
            std::size_t dy = fy / width_factor;

            result(fx, fy) = m(dx, dy);
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type resize_operation::bilinear(
        ir::node_data<double>&& arg, std::int64_t height_factor,
        std::int64_t width_factor, std::string interpolation) const
    {
        auto m = arg.matrix();
        blaze::DynamicMatrix<double> result(
            m.rows() * height_factor, m.columns() * width_factor, 0.);

        std::size_t num_elements =
            m.rows() * height_factor * m.columns() * width_factor;
        for (std::size_t i = 0; i < num_elements; ++i)
        {
            std::size_t fx = i / result.columns();
            std::size_t fy = i - fx * result.columns();

            std::size_t dx = fx / height_factor;
            std::size_t rx = fx % height_factor;
            std::size_t dy = fy / width_factor;
            std::size_t ry = fy % width_factor;

            std::size_t bx = 1;
            std::size_t by = 1;
            if (dx == m.rows() - 1)
                bx = 0;
            if (dy == m.columns() - 1)
                by = 0;

            double ul = m(dx, dy);
            double ur = m(dx, dy + by);
            double ll = m(dx + bx, dy);
            double lr = m(dx + bx, dy + by);
            double tmp = ry * (ur - ul) / width_factor;

            result(fx, fy) = ul + tmp +
                rx * (ll - ul - tmp + ry * (lr - ll) / width_factor) /
                    height_factor;
        }
        return primitive_argument_type{std::move(result)};
    }

    hpx::future<primitive_argument_type> resize_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
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

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);
        auto&& op1 = scalar_integer_operand_strict(
            operands[1], args, name_, codename_, ctx);
        auto&& op2 = scalar_integer_operand_strict(
            operands[2], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& arg,
                std::int64_t height_factor, std::int64_t width_factor,
                std::string interpolation)
            -> primitive_argument_type
            {
                if (height_factor < 0 || width_factor < 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "resize_operation::eval",
                        this_->generate_error_message(
                            "scaling factor should be positive"));
                }

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
                        HPX_FALLTHROUGH;
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
                else if (interpolation == "bilinear")
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
                        "interpolation type not supported"));
            }),
            std::move(op0), std::move(op1), std::move(op2),
            string_operand(operands[3], args, name_, codename_, std::move(ctx)));
    }
}}}
