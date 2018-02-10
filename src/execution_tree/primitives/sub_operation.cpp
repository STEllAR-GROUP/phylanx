//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/sub_operation.hpp>
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
    primitive create_sub_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("__sub");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const sub_operation::match_data =
    {
        hpx::util::make_tuple("__sub",
            std::vector<std::string>{"_1 - __2"},
            &create_sub_operation, &create_primitive<sub_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    sub_operation::sub_operation(
            std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
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

        ///////////////////////////////////////////////////////////////////////
        struct sub : std::enable_shared_from_this<sub>
        {
            sub() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_argument_type sub0d0d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops.size() == 2)
                {
                    lhs.scalar() -= rhs.scalar();
                    return primitive_argument_type(std::move(lhs));
                }

                return primitive_argument_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(lhs),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        result.scalar() -= curr.scalar();
                        return std::move(result);
                    }));
            }

            primitive_argument_type sub0d1d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d1d",
                        "the sub_operation primitive can sub a single value "
                        "to a vector only if there are exactly 2 operands");
                }

                ops[1].vector() =
                    blaze::map(ops[1].vector(), sub0dnd_simd(ops[0].scalar()));
                return primitive_argument_type(std::move(ops[1]));
            }

            primitive_argument_type sub0d2d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d2d",
                        "the sub_operation primitive can sub a single value "
                        "to a matrix only if there are exactly 2 operands");
                }

                ops[1].matrix() =
                    blaze::map(ops[1].matrix(), sub0dnd_simd(ops[0].scalar()));
                return primitive_argument_type(std::move(ops[1]));
            }

            primitive_argument_type sub0d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return sub0d0d(std::move(ops));

                case 1:
                    return sub0d1d(std::move(ops));

                case 2:
                    return sub0d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_argument_type sub1d0d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d1d",
                        "the sub_operation primitive can sub a single value "
                        "to a vector only if there are exactly 2 operands");
                }

                ops[0].vector() =
                    blaze::map(ops[0].vector(), subnd0d_simd(ops[1].scalar()));
                return primitive_argument_type(std::move(ops[0]));
            }

            primitive_argument_type sub1d1d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (lhs.size() != rhs.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub1d1d",
                        "the dimensions of the operands do not match");
                }

                if (ops.size() == 2)
                {
                    lhs.vector() -= rhs.vector();
                    return primitive_argument_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_argument_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(first_term),
                    [](operand_type& result,
                        operand_type const& curr) -> operand_type {
                        result.vector() -= curr.vector();
                        return std::move(result);
                    }));
            }

            primitive_argument_type sub1d2d(operands_type&& ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub1d2d",
                        "the sub_operation primitive can sub a vector "
                        "from a matrix only if there are exactly 2 operands");
                }

                if (ops[0].vector().size() != ops[1].matrix().columns())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub1d2d",
                        "vector size does not match number of matrix columns");
                }
                // TODO: Blaze does not support broadcasting
                for (size_t i = 0UL; i < ops[1].matrix().rows(); ++i)
                {
                    blaze::row(ops[1].matrix(), i) =
                        blaze::trans(ops[0].vector()) -
                        blaze::row(ops[1].matrix(), i);
                }

                return primitive_argument_type(std::move(ops[1]));
            }

            primitive_argument_type sub1d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();

                switch(rhs_dims)
                {
                case 0:
                    return sub1d0d(std::move(ops));

                case 1:
                    return sub1d1d(std::move(ops));

                case 2:
                    return sub1d2d(std::move(ops));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_argument_type sub2d0d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d2d",
                        "the sub_operation primitive can sub a single value "
                        "to a matrix only if there are exactly 2 operands");
                }

                ops[0].matrix() =
                    blaze::map(ops[0].matrix(), subnd0d_simd(ops[1].scalar()));
                return primitive_argument_type(std::move(ops[0]));
            }

            primitive_argument_type sub2d1d(operands_type&& ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub2d1d",
                        "the sub_operation primitive can sub a vector "
                        "from a matrix only if there are exactly 2 operands");
                }

                if (ops[1].vector().size() != ops[0].matrix().columns())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub2d1d",
                        "vector size does not match number of matrix columns");
                }
                // TODO: Blaze does not support broadcasting
                for (size_t i = 0UL; i < ops[0].matrix().rows(); ++i)
                {
                    blaze::row(ops[0].matrix(), i) -=
                        blaze::trans(ops[1].vector());
                }

                return primitive_argument_type(std::move(ops[0]));
            }

            primitive_argument_type sub2d2d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub2d2d",
                        "the dimensions of the operands do not match");
                }

                if (ops.size() == 2)
                {
                    lhs.matrix() -= rhs.matrix();
                    return primitive_argument_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_argument_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(first_term),
                    [](operand_type& result,
                        operand_type const& curr) -> operand_type {
                        result.matrix() -= curr.matrix();
                        return std::move(result);
                    }));
            }

            primitive_argument_type sub2d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return sub2d0d(std::move(ops));

                case 2:
                    return sub2d2d(std::move(ops));

                case 1:
                    return sub2d1d(std::move(ops));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub2d",
                        "the operands have incompatible number of dimensions");
                }
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub_operation",
                        "the sub_operation primitive requires at least two operands");
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
                        "the sub_operation primitive requires that the arguments given "
                            "by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(
                    hpx::util::unwrapping([this_](operands_type&& ops)
                                              -> primitive_argument_type {
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
                                "left hand side operand has unsupported "
                                "number of dimensions");
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    // implement '-' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> sub_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::sub>()->eval(args, noargs);
        }

        return std::make_shared<detail::sub>()->eval(operands_, args);
    }
}}}
