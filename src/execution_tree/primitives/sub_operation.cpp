//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/sub_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/blaze.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::sub_operation>
    sub_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    sub_operation_type, phylanx_sub_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(sub_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const sub_operation::match_data =
    {
        hpx::util::make_tuple("sub", "_1 - __2", &create<sub_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    sub_operation::sub_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sub_operation::sub_operation",
                "the sub_operation primitive requires at least two operands");
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands_.size(); ++i)
        {
            if (!valid(operands_[i]))
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
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct sub : std::enable_shared_from_this<sub>
        {
            sub(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type sub0d0d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                if (ops.size() == 2)
                {
                    lhs[0] -= rhs[0];
                    return primitive_result_type(std::move(lhs));
                }

                return primitive_result_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(lhs),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        result[0] -= curr[0];
                        return std::move(result);
                    }));
            }

            primitive_result_type sub0d1d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d1d",
                        "the sub_operation primitive can sub a single value "
                            "to a vector only if there are exactly 2 operands");
                }

                ops[1].matrix() = blaze::map(ops[1].matrix(), [&](double x) { return ops[0][0] - x; });
                return primitive_result_type(std::move(ops[1]));
            }

            primitive_result_type sub0d2d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d2d",
                        "the sub_operation primitive can sub a single value "
                            "to a matrix only if there are exactly 2 operands");
                }

                ops[1].matrix() = blaze::map(ops[1].matrix(), [&](double x) { return ops[0][0] - x; });
                return primitive_result_type(std::move(ops[1]));
            }

            primitive_result_type sub0d(operands_type && ops) const
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
            primitive_result_type sub1d0d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d1d",
                        "the sub_operation primitive can sub a single value "
                            "to a vector only if there are exactly 2 operands");
                }

                ops[0].matrix() = blaze::map(ops[0].matrix(), [&](double x) { return x - ops[1][0]; });
                return primitive_result_type(std::move(ops[0]));
            }

            primitive_result_type sub1d1d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub1d1d",
                        "the dimensions of the operands do not match");
                }

                if (ops.size() == 2)
                {
                    lhs.matrix() -= rhs.matrix();
                    return primitive_result_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_result_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(first_term),
                    [](operand_type& result, operand_type const& curr) -> operand_type
                    {
                        result.matrix() -= curr.matrix();
                        return std::move(result);
                    }));
            }

            primitive_result_type sub1d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();

                switch(rhs_dims)
                {
                case 0:
                    return sub1d0d(std::move(ops));

                case 1:
                    return sub1d1d(std::move(ops));

                case 2: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_result_type sub2d0d(operands_type && ops) const
            {
                if (ops.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub0d2d",
                        "the sub_operation primitive can sub a single value "
                            "to a matrix only if there are exactly 2 operands");
                }

                ops[0].matrix() = blaze::map(ops[0].matrix(), [&](double x) { return x - ops[1][0]; });
                return primitive_result_type(std::move(ops[0]));
            }

            primitive_result_type sub2d2d(operands_type && ops) const
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
                    return primitive_result_type(std::move(lhs));
                }

                operand_type& first_term = *ops.begin();
                return primitive_result_type(std::accumulate(
                    ops.begin() + 1, ops.end(), std::move(first_term),
                    [](operand_type& result, operand_type const& curr)
                    ->  operand_type
                    {
                        result.matrix() -= curr.matrix();
                        return std::move(result);
                    }));
            }

            primitive_result_type sub2d(operands_type && ops) const
            {
                std::size_t rhs_dims = ops[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return sub2d0d(std::move(ops));

                case 2:
                    return sub2d2d(std::move(ops));

                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sub_operation::sub2d",
                        "the operands have incompatible number of dimensions");
                }
            }

        public:
            hpx::future<primitive_result_type> eval() const
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops) -> primitive_result_type
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
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands_, numeric_operand)
                );
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    // implement '-' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> sub_operation::eval() const
    {
        return std::make_shared<detail::sub>(operands_)->eval();
    }
}}}
