//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/div_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::div_operation>
    div_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    div_operation_type, phylanx_div_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(div_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const div_operation::match_data =
    {
        hpx::util::make_tuple("div", "_1 / __2", &create<div_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    div_operation::div_operation(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct divndnd_simd
        {
            divndnd_simd() = default;

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a, T const& b) const
            ->  decltype(a / b)
            {
                return a / b;
            }

            template <typename T1, typename T2>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDDiv<T1, T2>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(
                T const& a, T const& b) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a / b;
            }
        };

        struct divnd0d_simd
        {
        public:
            explicit divnd0d_simd(double scalar)
              : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            ->  decltype(a / std::declval<double>())
            {
                return a / scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDDiv<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a / blaze::set(scalar_);
            }

        private:
            double scalar_;
        };

        struct div0dnd_simd
        {
        public:
            explicit div0dnd_simd(double scalar)
              : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
            ->  decltype(std::declval<double>() / a)
            {
                return scalar_ / a;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDDiv<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return blaze::set(scalar_) / a;
            }

        private:
            double scalar_;
        };

        ///////////////////////////////////////////////////////////////////////
        struct div : std::enable_shared_from_this<div>
        {
            div() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type div0d0d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops.size() == 2)
                {
                    lhs.scalar() /= rhs.scalar();
                    return primitive_result_type(std::move(lhs));
                }

                return primitive_result_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(lhs),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        result[0] /= curr[0];
                        return std::move(result);
                    }));
            }

            primitive_result_type div0d1d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div0d1d",
                        "the div_operation primitive can div a single value "
                            "to a vector only if there are exactly 2 operands");
                }

                ops[1].vector() =
                    blaze::map(ops[1].vector(), div0dnd_simd(ops[0].scalar()));
                return primitive_result_type(std::move(ops[1]));
            }

            primitive_result_type div0d2d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div0d2d",
                        "the div_operation primitive can div a single value "
                            "to a matrix only if there are exactly 2 operands");
                }

                ops[1].matrix() =
                    blaze::map(ops[1].matrix(), div0dnd_simd(ops[0].scalar()));
                return primitive_result_type(std::move(ops[1]));
            }

            primitive_result_type div0d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return div0d0d(std::move(ops));

                case 1:
                    return div0d1d(std::move(ops));

                case 2:
                    return div0d2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_result_type div1d0d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div0d1d",
                        "the div_operation primitive can div a single value "
                            "to a vector only if there are exactly 2 operands");
                }

                ops[0].vector() =
                    blaze::map(ops[0].vector(), divnd0d_simd(ops[1].scalar()));
                return primitive_result_type(std::move(ops[0]));
            }

            primitive_result_type div1d1d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size  != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div1d1d",
                        "the dimensions of the operands do not match");
                }

                if (ops.size() == 2)
                {
                    lhs.vector() =
                        blaze::map(lhs.vector(), rhs.vector(), divndnd_simd());
                    return primitive_result_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_result_type(std::accumulate(ops.begin() + 1,
                    ops.end(), std::move(first_term),
                    [](operand_type& result,
                        operand_type const& curr) -> operand_type {
                        result.vector() = blaze::map(
                            result.vector(), curr.vector(), divndnd_simd());
                        return std::move(result);
                    }));
            }

            primitive_result_type div1d2d(operands_type&& ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div1d2d",
                        "the div_operation primitive can divide a vector "
                        "to a matrix only if there are exactly 2 operands");
                }

                if (ops[0].vector().size() != ops[1].matrix().columns())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div1d2d",
                        "vector size does not match number of matrix columns");
                }
                // TODO: Blaze does not support broadcasting
                for (size_t i = 0UL; i < ops[1].matrix().rows(); ++i)
                {
                    blaze::row(ops[1].matrix(), i) =
                        blaze::trans(ops[0].vector()) /
                        blaze::row(ops[1].matrix(), i);
                }

                return primitive_result_type(std::move(ops[1]));
            }

            primitive_result_type div1d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();

                switch(rhs_dims)
                {
                case 0:
                    return div1d0d(std::move(ops));

                case 1:
                    return div1d1d(std::move(ops));

                case 2:
                    return div1d2d(std::move(ops));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_result_type div2d0d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div0d2d",
                        "the div_operation primitive can div a single value "
                            "to a matrix only if there are exactly 2 operands");
                }

                ops[0].matrix() =
                    blaze::map(ops[0].matrix(), divnd0d_simd(ops[1].scalar()));
                return primitive_result_type(std::move(ops[0]));
            }

            primitive_result_type div2d1d(operands_type&& ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div2d1d",
                        "the div_operation primitive can divide a matrix "
                        "to a vector only if there are exactly 2 operands");
                }

                if (ops[1].vector().size() != ops[0].matrix().columns())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div1d2d",
                        "vector size does not match number of matrix columns");
                }
                // TODO: Blaze does not support broadcasting
                for (size_t i = 0UL; i < ops[0].matrix().rows(); ++i)
                {
                    blaze::row(ops[0].matrix(), i) /=
                        blaze::trans(ops[1].vector());
                }

                return primitive_result_type(std::move(ops[0]));
            }

            primitive_result_type div2d2d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div2d2d",
                        "the dimensions of the operands do not match");
                }

                if (ops.size() == 2)
                {
                    lhs.matrix() =
                        blaze::map(lhs.matrix(), rhs.matrix(), divndnd_simd());
                    return primitive_result_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_result_type(std::accumulate(ops.begin() + 1,
                    ops.end(), std::move(first_term),
                    [](operand_type& result,
                        operand_type const& curr) -> operand_type {
                        result.matrix() = blaze::map(
                            result.matrix(), curr.matrix(), divndnd_simd());
                        return std::move(result);
                    }));
            }

            primitive_result_type div2d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return div2d0d(std::move(ops));

                case 2:
                    return div2d2d(std::move(ops));

                case 1:
                    return div2d1d(std::move(ops));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::div2d",
                        "the operands have incompatible number of dimensions");
                }
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::eval",
                        "the div_operation primitive requires at least two "
                            "operands");
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
                        "div_operation::eval",
                        "the div_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops) -> primitive_result_type
                    {
                        std::size_t lhs_dims = ops[0].num_dimensions();
                        switch (lhs_dims)
                        {
                        case 0:
                            return this_->div0d(std::move(ops));

                        case 1:
                            return this_->div1d(std::move(ops));

                        case 2:
                            return this_->div2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "div_operation::eval",
                                "left hand side operand has unsupported number "
                                "of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    // implement '/' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> div_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::div>()->eval(args, noargs);
        }

        return std::make_shared<detail::div>()->eval(operands_, args);
    }
}}}
