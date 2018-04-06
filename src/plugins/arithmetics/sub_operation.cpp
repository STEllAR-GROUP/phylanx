// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/sub_operation.hpp>
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
    match_pattern_type const sub_operation::match_data =
    {
        hpx::util::make_tuple("__sub",
            std::vector<std::string>{"_1 - __2", "__sub(_1, __2)"},
            &create_sub_operation, &create_primitive<sub_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    sub_operation::sub_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct sub0dnd_simd
        {
        public:
            explicit sub0dnd_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            ->  decltype(std::declval<double>() - a)
            {
                return scalar_ - a;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDSub<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return blaze::set(scalar_) - a;
            }

        private:
            double scalar_;
        };

        struct subnd0d_simd
        {
        public:
            explicit subnd0d_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            ->  decltype(a - std::declval<double>())
            {
                return a - scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDSub<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a - blaze::set(scalar_);
            }

        private:
            double scalar_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type sub_operation::sub0d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        lhs.scalar() -= rhs.scalar();
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub0d0d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        return primitive_argument_type(std::accumulate(
            ops.begin() + 1, ops.end(), std::move(lhs),
            [](operand_type& result, operand_type const& curr)
            ->  operand_type
            {
                result.scalar() -= curr.scalar();
                return std::move(result);
            }));
    }

    primitive_argument_type sub_operation::sub0d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(), detail::sub0dnd_simd(lhs.scalar()));
        }
        else
        {
            rhs.vector() =
                blaze::map(rhs.vector(), detail::sub0dnd_simd(lhs.scalar()));
        }

        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type sub_operation::sub0d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(), detail::sub0dnd_simd(lhs.scalar()));
        }
        else
        {
            rhs.matrix() =
                blaze::map(rhs.matrix(), detail::sub0dnd_simd(lhs.scalar()));
        }

        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type sub_operation::sub0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return sub0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return sub0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return sub0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type sub_operation::sub0d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return sub0d0d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type sub_operation::sub1d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), detail::subnd0d_simd(rhs.scalar()));
        }
        else
        {
            lhs.vector() =
                blaze::map(lhs.vector(), detail::subnd0d_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub1d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        if (lhs.is_ref())
        {
            lhs = lhs.vector() - rhs.vector();
        }
        else
        {
            lhs.vector() -= rhs.vector();
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type sub_operation::sub1d1d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        if (lhs.size() != rhs.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        operand_type& first_term = *ops.begin();
        return primitive_argument_type(std::accumulate(
            ops.begin() + 1, ops.end(), std::move(first_term),
            [](operand_type& result,
                operand_type const& curr) -> operand_type
            {
                if (result.is_ref())
                {
                    result = result.vector() - curr.vector();
                }
                else
                {
                    result.vector() -= curr.vector();
                }
                return std::move(result);
            }));
    }

    primitive_argument_type sub_operation::sub1d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d2d",
                execution_tree::generate_error_message(
                    "vector size does not match number of matrix columns",
                    name_, codename_));
        }

        // TODO: Blaze does not support broadcasting
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};
            for (std::size_t i = 0; i < cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::trans(cv) - blaze::row(cm, i);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i < cm.rows(); ++i)
        {
            blaze::row(cm, i) = blaze::trans(cv) - blaze::row(cm, i);
        }
        return primitive_argument_type{std::move(rhs)};
    }

    primitive_argument_type sub_operation::sub1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();

        switch(rhs_dims)
        {
        case 0:
            return sub1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return sub1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return sub1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type sub_operation::sub1d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();

        switch(rhs_dims)
        {
        case 1:
            return sub1d1d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type sub_operation::sub2d0d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), detail::subnd0d_simd(rhs.scalar()));
        }
        else
        {
            lhs.matrix() =
                blaze::map(lhs.matrix(), detail::subnd0d_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type sub_operation::sub2d1d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d1d",
                execution_tree::generate_error_message(
                    "vector size does not match number of matrix columns",
                    name_, codename_));
        }

        // TODO: Blaze does not support broadcasting
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};
            for (std::size_t i = 0; i < cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::row(cm, i) - blaze::trans(cv);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i < cm.rows(); ++i)
        {
            blaze::row(cm, i) -= blaze::trans(cv);
        }
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type sub_operation::sub2d2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        if (lhs.is_ref())
        {
            lhs = lhs.matrix() - rhs.matrix();
        }
        else
        {
            lhs.matrix() -= rhs.matrix();
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type sub_operation::sub2d2d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        operand_type& first_term = *ops.begin();
        return primitive_argument_type(std::accumulate(
            ops.begin() + 1, ops.end(), std::move(first_term),
            [](operand_type& result, operand_type const& curr) -> operand_type
            {
                if (result.is_ref())
                {
                    result = result.matrix() - curr.matrix();
                }
                else
                {
                    result.matrix() -= curr.matrix();
                }
                return std::move(result);
            }));
    }

    primitive_argument_type sub_operation::sub2d(
        operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return sub2d0d(std::move(lhs), std::move(rhs));

        case 1:
            return sub2d1d(std::move(lhs), std::move(rhs));

        case 2:
            return sub2d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type sub_operation::sub2d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return sub2d2d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> sub_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub_operation",
                execution_tree::generate_error_message(
                    "the sub_operation primitive requires at least two operands",
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
                "sub_operation::sub_operation",
                execution_tree::generate_error_message(
                    "the sub_operation primitive requires that the "
                        "arguments given by the operands array "
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
                        return this_->sub0d(std::move(lhs), std::move(rhs));

                    case 1:
                        return this_->sub1d(std::move(lhs), std::move(rhs));

                    case 2:
                        return this_->sub2d(std::move(lhs), std::move(rhs));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "sub_operation::eval",
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
                    return this_->sub0d(std::move(ops));

                case 1:
                    return this_->sub1d(std::move(ops));

                case 2:
                    return this_->sub2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::eval",
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
    // Implement '-' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> sub_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
