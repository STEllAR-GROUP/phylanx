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
      : operands_(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
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
                    lhs[0] /= rhs[0];
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

                ops[1].matrix() = blaze::map(
                        ops[1].matrix(),
                        [&](double x) { return ops[0][0] / x; });
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

                ops[1].matrix() = blaze::map(
                        ops[1].matrix(),
                        [&](double x) { return ops[0][0] / x; });
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

                ops[0].matrix() = blaze::map(
                        ops[0].matrix(),
                        [&](double x) { return x / ops[1][0]; });
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
                    lhs.matrix() = blaze::map(
                            lhs.matrix(),
                            rhs.matrix(),
                            [](double x1, double x2) { return x1 / x2; });
                    return primitive_result_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_result_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(first_term),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        result.matrix() = blaze::map(
                                result.matrix(),
                                curr.matrix(),
                                [](double x1, double x2) { return x1 / x2; });
                        return std::move(result);
                    }));
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

                case 2: HPX_FALLTHROUGH;
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

                ops[0].matrix() = blaze::map(
                        ops[0].matrix(),
                        [&](double x) { return x / ops[1][0]; });
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
                    lhs.matrix() = blaze::map(
                            lhs.matrix(),
                            rhs.matrix(),
                            [](double x1, double x2) { return x1 / x2; });
                    return primitive_result_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_result_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(first_term),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        result.matrix() = blaze::map(
                                result.matrix(),
                                curr.matrix(),
                                [](double x1, double x2) { return x1 / x2; });
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

                case 1: HPX_FALLTHROUGH;
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
            static std::vector<primitive_argument_type> noargs;
            return std::make_shared<detail::div>()->eval(args, noargs);
        }

        return std::make_shared<detail::div>()->eval(operands_, args);
    }
}}}
