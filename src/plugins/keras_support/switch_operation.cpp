//  Copyright (c) 2019 Bita Hasheminezhad
//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/switch_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
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
    match_pattern_type const switch_operation::match_data =
    {
        hpx::util::make_tuple("switch",
            std::vector<std::string>{"switch(_1, _2, _3)"},
            &create_switch_operation, &create_primitive<switch_operation>,
            R"(condition, then_expression, else_expression
            Args:

                condition (an array) : a tensor
                then_expression (an array) : either a tensor, or a callable
                    that returns a tensor.
                else_expression (an array) : either a tensor, or a callable
                    that returns a tensor.

            Returns:

            The selected tensor based on each element of condition. Note that
            both `then_expression` and `else_expression` should be symbolic
            tensors of the same shape. Condition cannot have a bigger rank than
            `then_expression`)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    switch_operation::switch_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    bool switch_operation::validate_shapes(std::size_t const& ndims_cond,
        std::size_t const& ndims_then,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_cond,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>&& dims_then) const
    {
        if (ndims_cond <= ndims_then)
        {
            if (dims_cond == dims_then)
                return true;

            if (ndims_cond == ndims_then)
            {
                for (std::size_t i = 0; i != ndims_cond; ++i)
                    if (dims_cond[i] != dims_then[i])
                        if (dims_cond[i] != 1)
                            return false;
                return true;
            }

            if (ndims_cond < ndims_then)
            {
                if (ndims_cond == 0)
                    return true;

                switch (ndims_then)
                {
                case 1:
                {
                    if (dims_cond[0] == 1)
                        return true;
                    break;
                }

                case 2:
                {
                    if (ndims_cond == 1)
                        if (dims_cond[0] == dims_then[0] || dims_cond[0] == 1)
                            return true;
                    break;
                }

                case 3:
                    if (ndims_cond == 1)
                    {
                        if (dims_cond[0] == dims_then[0] || dims_cond[0] == 1)
                            return true;
                    }
                    else if (ndims_cond == 2)
                    {
                        if ((dims_cond[0] == dims_then[0] ||
                                dims_cond[0] == 1) &&
                            (dims_cond[1] == dims_then[1] || dims_cond[1] == 1))
                            return true;
                    }
                    break;

                default:
                    break;
                }
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "switch_operation::validate_shapes",
                    util::generate_error_message(
                        "the condition shape is not compatible with the "
                        "`then_expression` shape",
                        name_, codename_));
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "switch_operation::validate_shapes",
            util::generate_error_message(
                "the condition rank should be less than or equal to the "
                "`then_expression` rank",
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type switch_operation::switch0d(
        ir::node_data<std::uint8_t>&& cond,
        ir::node_data<double>&& then_expr,
        ir::node_data<double>&& else_expr) const
    {
        return primitive_argument_type{
            cond.scalar() ? std::move(then_expr) : std::move(else_expr)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type switch_operation::switch1d(
        ir::node_data<std::uint8_t>&& cond,
        ir::node_data<double>&& then_expr,
        ir::node_data<double>&& else_expr) const
    {
        auto t = then_expr.vector();
        auto e = else_expr.vector();
        std::size_t size = t.size();

        ir::node_data<std::uint8_t> condition =
            extract_value_vector<std::uint8_t>(
                std::move(cond), size, name_, codename_);

        blaze::UniformVector<bool> ones(size, true);
        blaze::DynamicVector<bool> c = condition.vector();

        if (!then_expr.is_ref())
        {
            t = c * t + (ones - c) * e;
            return primitive_argument_type{std::move(then_expr)};
        }
        else if (!else_expr.is_ref())
        {
            e = c * t + (ones - c) * e;
            return primitive_argument_type{std::move(else_expr)};
        }

        blaze::DynamicVector<double> result = c * t + (ones - c) * e;

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type switch_operation::switch2d(
        ir::node_data<std::uint8_t>&& cond,
        ir::node_data<double>&& then_expr,
        ir::node_data<double>&& else_expr) const
    {
        auto t = then_expr.matrix();
        auto e = else_expr.matrix();
        std::size_t rows = t.rows();
        std::size_t columns = t.columns();

        blaze::DynamicMatrix<bool> c(rows, columns);

        ir::node_data<std::uint8_t> condition;
        if (cond.num_dimensions() == 2)
        {
            condition = extract_value_matrix<std::uint8_t>(
                std::move(cond), rows, columns, name_, codename_);
            c = condition.matrix();
        }
        else
        {
            condition = extract_value_matrix<std::uint8_t>(
                std::move(cond), columns, rows, name_, codename_);
            c = blaze::trans(condition.matrix());
        }

        blaze::UniformMatrix<bool> ones(rows, columns, true);

        if (!then_expr.is_ref())
        {
            t = c % t + (ones - c) % e;
            return primitive_argument_type{std::move(then_expr)};
        }
        else if (!else_expr.is_ref())
        {
            e = c % t + (ones - c) % e;
            return primitive_argument_type{std::move(else_expr)};
        }

        blaze::DynamicMatrix<double> result = c % t + (ones - c) % e;

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type switch_operation::switch3d(
        ir::node_data<std::uint8_t>&& cond,
        ir::node_data<double>&& then_expr,
        ir::node_data<double>&& else_expr) const
    {
        auto t = then_expr.tensor();
        auto e = else_expr.tensor();

        std::size_t pages = t.pages();
        std::size_t rows = t.rows();
        std::size_t columns = t.columns();

        blaze::DynamicTensor<bool> c(pages, rows, columns);

        ir::node_data<std::uint8_t> condition;
        if (cond.num_dimensions() == 3)
        {
            condition = extract_value_tensor<std::uint8_t>(
                std::move(cond), pages, rows, columns, name_, codename_);
            c = condition.tensor();
        }
        else if (cond.num_dimensions() == 2)
        {
            condition = extract_value_tensor<std::uint8_t>(
                std::move(cond), columns, pages, rows, name_, codename_);
            c = blaze::trans(condition.tensor(), {1, 2, 0});
        }
        else
        {
            condition = extract_value_tensor<std::uint8_t>(
                std::move(cond), columns, rows, pages, name_, codename_);
            c = blaze::trans(condition.tensor());
        }

        blaze::UniformTensor<bool> ones(pages, rows, columns, true);


        if (!then_expr.is_ref())
        {
            t = c % t + (ones - c) % e;
            return primitive_argument_type{std::move(then_expr)};
        }
        else if (!else_expr.is_ref())
        {
            e = c % t + (ones - c) % e;
            return primitive_argument_type{std::move(else_expr)};
        }

        blaze::DynamicTensor<double> result = c % t + (ones - c) % e;

        return primitive_argument_type{std::move(result)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> switch_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands_.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "switch_operation::eval",
                util::generate_error_message(
                    "the switch primitive requires exactly three "
                    "operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "switch_operation::eval",
                    util::generate_error_message(
                        "the switch primitive requires that the arguments "
                        "given by the operands array are valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> cond_dims =
                    extract_numeric_value_dimensions(
                        args[0], this_->name_, this_->codename_);
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> then_dims =
                    extract_numeric_value_dimensions(
                        args[1], this_->name_, this_->codename_);
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> else_dims =
                    extract_numeric_value_dimensions(
                        args[2], this_->name_, this_->codename_);

                if (then_dims != else_dims)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "switch_operation::eval",
                        util::generate_error_message(
                            "the `then_expression` and `else_expression` "
                            "should be of the same shape",
                            this_->name_, this_->codename_));
                }

                std::size_t dim0 = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);
                std::size_t dim1 = extract_numeric_value_dimension(
                    args[1], this_->name_, this_->codename_);

                if (!this_->validate_shapes(
                        dim0, dim1, std::move(cond_dims), std::move(then_dims)))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "switch_operation::eval",
                        this_->generate_error_message(
                            "the condition shape is incompatible with the "
                            "`then_expression` and `else_expression` shapes"));
                }

                ir::node_data<std::uint8_t> cond = extract_boolean_value(
                    std::move(args[0]), this_->name_, this_->codename_);
                ir::node_data<double> then_expr = extract_numeric_value(
                    std::move(args[1]), this_->name_, this_->codename_);
                ir::node_data<double> else_expr = extract_numeric_value(
                    std::move(args[2]), this_->name_, this_->codename_);

                switch (dim1)
                {
                case 0:
                    return this_->switch0d(std::move(cond),
                        std::move(then_expr), std::move(else_expr));

                case 1:
                    return this_->switch1d(std::move(cond),
                        std::move(then_expr), std::move(else_expr));

                case 2:
                    return this_->switch2d(std::move(cond),
                        std::move(then_expr), std::move(else_expr));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->switch3d(std::move(cond),
                        std::move(then_expr), std::move(else_expr));
#endif
                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "switch_operation::eval",
                    this_->generate_error_message(
                        "the operand has unsupported number of dimensions"));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }

}}}
