// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2017 Alireza Kheirkhahan
// Copyright (c) 2017-2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/mul_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const mul_operation::match_data =
    {
        hpx::util::make_tuple("__mul",
            std::vector<std::string>{"_1 * __2", "__mul(_1, __2)"},
            &create_mul_operation, &create_primitive<mul_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    mul_operation::mul_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type mul_operation::mul0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            return mul0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return mul0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return mul0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type mul_operation::mul0d(operands_type && ops) const
    {
        switch (ops[1].num_dimensions())
        {
        case 0:
            return mul0d0d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type mul_operation::mul0d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        lhs.scalar() *= rhs.scalar();
        return primitive_argument_type{ std::move(lhs) };
    }

    primitive_argument_type mul_operation::mul0d0d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        return primitive_argument_type{
            std::accumulate(
                ops.begin() + 1, ops.end(), std::move(lhs),
                [this](operand_type& result, operand_type const& curr)
                {
                    if (curr.num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "mul_operation::mul0d0d",
                            execution_tree::generate_error_message(
                                "all operands must be scalars",
                                name_, codename_));
                    }
                    result.scalar() *= curr.scalar();
                    return std::move(result);
                })
            };
    }

    primitive_argument_type mul_operation::mul0d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.vector() * lhs.scalar();
        }
        else
        {
            rhs.vector() *= lhs.scalar();
        }
        return primitive_argument_type{std::move(rhs)};
    }

    primitive_argument_type mul_operation::mul0d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = rhs.matrix() * lhs.scalar();
        }
        else
        {
            rhs.matrix() *= lhs.scalar();
        }
        return primitive_argument_type{std::move(rhs)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type mul_operation::mul1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            return mul1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return mul1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return mul1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type mul_operation::mul1d(operands_type && ops) const
    {
        switch (ops[1].num_dimensions())
        {
        case 1:
            return mul1d1d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type mul_operation::mul1d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.vector() * rhs.scalar();
        }
        else
        {
            lhs.vector() *= rhs.scalar();
        }
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type mul_operation::mul1d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.vector() * rhs.vector();
        }
        else
        {
            lhs.vector() *= rhs.vector();
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type mul_operation::mul1d1d(operands_type && ops) const
    {
        return primitive_argument_type{
            std::accumulate(
                ops.begin() + 1, ops.end(), std::move(ops[0]),
                [this](operand_type& result, operand_type const& curr)
                ->  operand_type
                {
                    if (curr.num_dimensions() != 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "mul_operation::mul1d1d",
                            execution_tree::generate_error_message(
                                "all operands must be vectors",
                                name_, codename_));
                    }

                    if (result.is_ref())
                    {
                        result = result.vector() * curr.vector();
                    }
                    else
                    {
                        result.vector() *= curr.vector();
                    }
                    return std::move(result);
                })
            };
    }

    primitive_argument_type mul_operation::mul1d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul1d2d",
                "vector size does not match number of matrix columns");
        }

        // TODO: Blaze does not support broadcasting
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cv.size()};
            for (std::size_t i = 0; i < cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::trans(cv) * blaze::row(cm, i);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i < rhs.matrix().rows(); ++i)
        {
            blaze::row(cm, i) = blaze::trans(cv) * blaze::row(cm, i);
        }
        return primitive_argument_type{std::move(rhs)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type mul_operation::mul2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            return mul2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return mul2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return mul2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type mul_operation::mul2d(operands_type && ops) const
    {
        switch (ops[1].num_dimensions())
        {
        case 2:
            return mul2d2d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type mul_operation::mul2d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.matrix() * rhs.scalar();
        }
        else
        {
            lhs.matrix() *= rhs.scalar();
        }
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type mul_operation::mul2d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul2d1d",
                "vector size does not match number of matrix columns");
        }

        // TODO: Blaze does not support broadcasting
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cv.size()};
            for (std::size_t i = 0; i < cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::row(cm, i) * blaze::trans(cv);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i < cm.rows(); ++i)
        {
            blaze::row(cm, i) *= blaze::trans(cv);
        }
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type mul_operation::mul2d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = lhs.matrix() % rhs.matrix();
        }
        else
        {
            lhs.matrix() %= rhs.matrix();
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type mul_operation::mul2d2d(operands_type && ops) const
    {
        return primitive_argument_type{
            std::accumulate(
                ops.begin() + 1, ops.end(), std::move(ops[0]),
                [this](operand_type& result, operand_type const& curr)
                ->  operand_type
                {
                    if (curr.num_dimensions() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "mul_operation::mul2d2d",
                            execution_tree::generate_error_message(
                                "all operands must be matrices",
                                name_, codename_));
                    }

                    if (result.is_ref())
                    {
                        result = result.matrix() % curr.matrix();
                    }
                    else
                    {
                        result.matrix() %= curr.matrix();
                    }
                    return std::move(result);
                })
            };
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> mul_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul_operation",
                execution_tree::generate_error_message(
                    "the mul_operation primitive requires at least "
                        "two operands",
                    name_, codename_));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "mul_operation::mul_operation",
                execution_tree::generate_error_message(
                    "the mul_operation primitive requires that "
                        "the arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](operand_type&& lhs, operand_type&& rhs)
                ->  primitive_argument_type
                {
                    std::size_t lhs_dims = lhs.num_dimensions();
                    switch (lhs_dims)
                    {
                    case 0:
                        return this_->mul0d(std::move(lhs), std::move(rhs));

                    case 1:
                        return this_->mul1d(std::move(lhs), std::move(rhs));

                    case 2:
                        return this_->mul2d(std::move(lhs), std::move(rhs));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "mul_operation::eval",
                            execution_tree::generate_error_message(
                                "left hand side operand has unsupported "
                                    "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
                numeric_operand(operands[0], args, name_, codename_),
                numeric_operand(operands[1], args, name_, codename_));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operands_type&& ops) -> primitive_argument_type
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return this_->mul0d(std::move(ops));

                case 1:
                    return this_->mul1d(std::move(ops));

                case 2:
                    return this_->mul2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    //////////////////////////////////////////////////////////////////////////
    // Implement '*' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> mul_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
