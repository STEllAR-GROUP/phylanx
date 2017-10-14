//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/unary_minus_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/eigen.hpp>
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
    phylanx::execution_tree::primitives::unary_minus_operation>
    unary_minus_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    unary_minus_operation_type, phylanx_unary_minus_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(unary_minus_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const unary_minus_operation::match_data =
    {
        {"-_1", &create<unary_minus_operation>}
    };

    ///////////////////////////////////////////////////////////////////////////
    unary_minus_operation::unary_minus_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_minus_operation::unary_minus_operation",
                "the unary_minus_operation primitive requires exactly one "
                    "operand");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_minus_operation::unary_minus_operation",
                "the unary_minus_operation primitive requires that the "
                    "argument given by the operands array is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct unary_minus : std::enable_shared_from_this<unary_minus>
        {
            unary_minus(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type neg0d(operands_type&& ops) const
            {
                ops[0][0] = -ops[0][0];
                return primitive_result_type(std::move(ops[0]));
            }

            primitive_result_type negxd(operands_type&& ops) const
            {
                ops[0].matrix().array() = -ops[0].matrix().array();
                return primitive_result_type(std::move(ops[0]));
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
                            return this_->neg0d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        case 2:
                            return this_->negxd(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "unary_minus_operation::eval",
                                "operand has unsupported number of dimensions");
                        }
                    }),
                    detail::map_operands(operands_, numeric_operand)
                );
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    // implement unary '-' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> unary_minus_operation::eval() const
    {
        return std::make_shared<detail::unary_minus>(operands_)->eval();
    }
}}}
