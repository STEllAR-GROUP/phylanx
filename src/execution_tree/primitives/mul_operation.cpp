//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Alireza Kheirkhahan
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/mul_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_mul_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__mul");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const mul_operation::match_data =
    {
        hpx::util::make_tuple("__mul",
            std::vector<std::string>{"_1 * __2"},
            &create_mul_operation, &create_primitive<mul_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    mul_operation::mul_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct mul : std::enable_shared_from_this<mul>
        {
            mul(std::string const& name, std::string const& codename)
              : name_(name)
              , codename_(codename)
            {
            }

        protected:
            std::string name_;
            std::string codename_;

        private:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

        private:
            primitive_argument_type mul0d(operands_type && ops) const
            {
                switch (ops[1].num_dimensions())
                {
                case 0:
                    return mul0d0d(std::move(ops));

                case 1:
                    return mul0d1d(std::move(ops));

                case 2:
                    return mul0d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d",
                        generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

            primitive_argument_type mul0d0d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops.size() == 2)
                {
                    lhs.scalar() *= rhs.scalar();
                    return primitive_argument_type{ std::move(lhs) };
                }

                return primitive_argument_type{
                    std::accumulate(
                        ops.begin() + 1, ops.end(), std::move(lhs),
                        [this](operand_type& result, operand_type const& curr)
                        {
                            if (curr.num_dimensions() != 0)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "mul_operation::mul0d0d",
                                    generate_error_message(
                                        "all operands must be scalars",
                                        name_, codename_));
                            }
                            result.scalar() *= curr.scalar();
                            return std::move(result);
                        })
                    };
            }

            primitive_argument_type mul0d1d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d1d",
                        generate_error_message(
                            "can't handle more than two operands",
                            name_, codename_));
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops[1].is_ref())
                {
                    rhs = rhs.vector() * lhs.scalar();
                }
                else
                {
                    rhs.vector() *= lhs.scalar();
                }
                return primitive_argument_type{ std::move(rhs) };
            }

            primitive_argument_type mul0d2d(operands_type && ops) const
            {

                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul0d2d",
                        generate_error_message(
                            "can't handle more than two operands",
                            name_, codename_));
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops[1].is_ref())
                {
                    rhs = rhs.matrix() * lhs.scalar();
                }
                else
                {
                    rhs.matrix() *= lhs.scalar();
                }
                return primitive_argument_type{ std::move(rhs) };
            }

            ///////////////////////////////////////////////////////////////////
            primitive_argument_type mul1d(operands_type && ops) const
            {
                switch (ops[1].num_dimensions())
                {
                case 0:
                    return mul1d0d(std::move(ops));

                case 1:
                    return mul1d1d(std::move(ops));

                case 2:
                    return mul1d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul1d",
                        generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

            primitive_argument_type mul1d0d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul1d0d",
                        generate_error_message(
                            "can't handle more than two operands",
                            name_, codename_));
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops[0].is_ref())
                {
                    lhs = lhs.vector() * rhs.scalar();
                }
                else
                {
                    lhs.vector() *= rhs.scalar();
                }
                return primitive_argument_type{std::move(lhs)};
            }

            primitive_argument_type mul1d1d(operands_type && ops) const
            {
                if (ops.size() == 2)
                {
                    if (ops[0].is_ref())
                    {
                        ops[0] = ops[0].vector() * ops[1].vector();
                    }
                    else
                    {
                        ops[0].vector() *= ops[1].vector();
                    }
                    return primitive_argument_type{ std::move(ops[0]) };
                }

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
                                    generate_error_message(
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

            primitive_argument_type mul1d2d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul1d2d",
                        generate_error_message(
                            "can't handle more than two operands",
                            name_, codename_));
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

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

            primitive_argument_type mul2d(operands_type && ops) const
            {
                switch (ops[1].num_dimensions())
                {
                case 0:
                    return mul2d0d(std::move(ops));

                case 1:
                    return mul2d1d(std::move(ops));

                case 2:
                    return mul2d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul2d",
                        generate_error_message(
                            "the operands have incompatible number of "
                                "dimensions",
                            name_, codename_));
                }
            }

            primitive_argument_type mul2d0d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul2d0d",
                        generate_error_message(
                            "can't handle more than two operands",
                            name_, codename_));
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

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

            primitive_argument_type mul2d1d(operands_type && ops) const
            {
                if (ops.size() > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul2d1d",
                        generate_error_message(
                            "can't handle more than two operands",
                            name_, codename_));
                }

                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

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

            primitive_argument_type mul2d2d(operands_type && ops) const
            {
                if (ops.size() == 2)
                {
                    if (ops[0].is_ref())
                    {
                        ops[0] = ops[0].matrix() % ops[1].matrix();
                    }
                    else
                    {
                        ops[0].matrix() %= ops[1].matrix();
                    }
                    return primitive_argument_type{std::move(ops[0])};
                }

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
                                    generate_error_message(
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

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "mul_operation::mul_operation",
                        generate_error_message(
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
                        generate_error_message(
                            "the mul_operation primitive requires that "
                                "the arguments given by the operands array "
                                "are valid",
                            name_, codename_));
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
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
                                generate_error_message(
                                    "left hand side operand has unsupported "
                                        "number of dimensions",
                                    this_->name_, this_->codename_));
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args,
                        name_, codename_));
            }
        };
    }

    // implement '*' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> mul_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::mul>(name_, codename_)
                ->eval(args, noargs);
        }
        return std::make_shared<detail::mul>(name_, codename_)
            ->eval(operands_, args);
    }
}}}
