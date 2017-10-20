//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/unary_not_operation.hpp>
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
    phylanx::execution_tree::primitives::unary_not_operation>
    unary_not_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    unary_not_operation_type, phylanx_unary_not_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(unary_not_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const unary_not_operation::match_data =
    {
        hpx::util::make_tuple("unary_not", "!_1", &create<unary_not_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    unary_not_operation::unary_not_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::unary_not_operation",
                "the unary_not_operation primitive requires exactly one "
                    "operand");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::unary_not_operation",
                "the unary_not_operation primitive requires that the "
                    "argument given by the operands array is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct unary_not : std::enable_shared_from_this<unary_not>
        {
            unary_not(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

        private:
            using operands_type = std::vector<std::uint8_t>;

        public:
            hpx::future<primitive_result_type> eval() const
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type && ops) -> primitive_result_type
                    {
                        return primitive_result_type(ops[0] == 0);
                    }),
                    detail::map_operands(operands_, boolean_operand)
                );
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    // implement unary '!' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> unary_not_operation::eval() const
    {
        return std::make_shared<detail::unary_not>(operands_)->eval();
    }
}}}
