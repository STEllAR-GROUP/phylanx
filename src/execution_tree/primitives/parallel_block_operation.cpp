//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/parallel_block_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/optional.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/optional.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::parallel_block_operation>
    parallel_block_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    parallel_block_operation_type, phylanx_parallel_block_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(parallel_block_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const parallel_block_operation::match_data =
    {
        "parallel_block(__1)", &create<parallel_block_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    parallel_block_operation::parallel_block_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() <= 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "parallel_block_operation::parallel_block_operation",
                "the parallel_block_operation primitive requires at least one "
                    "argument");
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
                "phylanx::execution_tree::primitives::"
                    "parallel_block_operation::parallel_block_operation",
                "the parallel_block_operation primitive requires that the "
                    "arguments given by the operands array are valid");
        }
    }

    namespace detail
    {
        struct block : std::enable_shared_from_this<block>
        {
            block(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

            hpx::future<primitive_result_type> eval() const
            {
                // evaluate condition of while statement
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](std::vector<primitive_result_type> && ops)
                    {
                        return ops.back();
                    }),
                    detail::map_operands(operands_, literal_operand)
                );
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    // start iteration over given while statement
    hpx::future<primitive_result_type> parallel_block_operation::eval() const
    {
        return std::make_shared<detail::block>(operands_)->eval();
    }
}}}
