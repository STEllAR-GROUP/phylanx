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

#include <algorithm>
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
            R"(cond, thenf, elsef
            This primitive implements the if statement in Python.
            The statement `thenf` is evaluated if `cond` is True and
            the statement `elsef` is evaluated otherwise.

            Args:

                cond (boolean expression) : a boolean expression
                thenf (statement) : a statement
                elsef (statement, optional) : a statement

            Returns:

              The value returned by the statement that was executed, `nil`
              otherwise)"
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
            if (ndims_cond < ndims_then)
                switch(ndims_then)
                {
                //case 1:
                //case 2:
                //case 3:
                    return true;
                }
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "switch_operation::validate_shapes",
                util::generate_error_message(
                    "the condition shape is not compatible with the "
                    "`then_expression` shape",
                    name_, codename_));
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

        ir::node_data<double> condition =
            extract_value_vector<double>(
                std::move(cond), size, name_, codename_);

        blaze::UniformVector<std::uint8_t> ones(size, true);
        auto c = condition.vector();

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

        blaze::DynamicVector<double> result(size);

        result = c * t + (ones - c) * e;
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

        ir::node_data<double> condition = extract_value_matrix<double>(
            std::move(cond), columns, rows, name_, codename_);

        blaze::UniformMatrix<std::uint8_t> ones(rows, columns, true);
        auto c = blaze::trans(condition.matrix());

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

        blaze::DynamicMatrix<double> result(rows, columns);

        result = c % t + (ones - c) % e;
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

        ir::node_data<double> condition = extract_value_tensor<double>(
            std::move(cond), columns, rows, pages, name_, codename_);

        blaze::UniformTensor<std::uint8_t> ones(pages, rows, columns, true);
        auto c = blaze::trans(condition.tensor());

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

        blaze::DynamicTensor<double> result(pages, rows, columns);

        result = c % t + (ones - c) % e;
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
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {

                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> then_dims =
                    extract_numeric_value_dimensions(
                        std::move(args[1]), this_->name_, this_->codename_);
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> else_dims =
                    extract_numeric_value_dimensions(
                        std::move(args[2]), this_->name_, this_->codename_);

                if (then_dims != else_dims)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "switch_operation::eval",
                        util::generate_error_message(
                            "the `then_expression` and `else_expression` "
                            "should be of the same shape",
                            this_->name_, this_->codename_));

                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> cond_dims =
                    extract_numeric_value_dimensions(
                        std::move(args[0]), this_->name_, this_->codename_);

                if (!this_->validate_shapes(
                        extract_numeric_value_dimension(
                            std::move(args[0]), this_->name_, this_->codename_),
                        extract_numeric_value_dimension(
                            std::move(args[1]), this_->name_, this_->codename_),
                        std::move(cond_dims),
                        std::move(then_dims)))
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "switch_operation::eval",
                        util::generate_error_message(
                            "the condition shape is incompatible with the "
                            "`then_expression` and `else_expression` "
                            "shapes",
                            this_->name_, this_->codename_));

                ir::node_data<std::uint8_t> cond = extract_boolean_value(
                    std::move(args[0]), this_->name_, this_->codename_);
                ir::node_data<double> then_expr = extract_numeric_value(
                    std::move(args[1]), this_->name_, this_->codename_);
                ir::node_data<double> else_expr = extract_numeric_value(
                    std::move(args[2]), this_->name_, this_->codename_);

                switch (extract_numeric_value_dimension(std::move(args[1])))
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
                    util::generate_error_message(
                        "the operand has unsupported number of dimensions",
                        this_->name_, this_->codename_));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }

}}}
