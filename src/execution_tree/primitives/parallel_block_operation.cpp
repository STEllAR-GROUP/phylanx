//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/parallel_block_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>

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
    std::vector<match_pattern_type> const parallel_block_operation::match_data =
    {
        hpx::util::make_tuple("parallel_block", "parallel_block(__1)",
            &create<parallel_block_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    parallel_block_operation::parallel_block_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {}

    namespace detail
    {
        struct block : std::enable_shared_from_this<block>
        {
            block() = default;

            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                    std::vector<primitive_argument_type> const& args) const
            {
                if (operands.empty())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "parallel_block_operation::eval",
                        "the parallel_block_operation primitive requires at "
                            "least one argument");
                }

                // evaluate condition of while statement
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](std::vector<primitive_result_type> && ops)
                    {
                        return ops.back();
                    }),
                    detail::map_operands(operands, literal_operand, args)
                );
            }
        };
    }

    // start iteration over given parallel-block statement
    hpx::future<primitive_result_type> parallel_block_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            static std::vector<primitive_argument_type> noargs;
            return std::make_shared<detail::block>()->eval(args, noargs);
        }

        return std::make_shared<detail::block>()->eval(operands_, args);
    }
}}}
