//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/parallel_block_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_parallel_block_operation(
        hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("parallel_block");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const parallel_block_operation::match_data =
    {
        hpx::util::make_tuple("parallel_block",
            std::vector<std::string>{"parallel_block(__1)"},
            &create_parallel_block_operation,
            &create_primitive<parallel_block_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    parallel_block_operation::parallel_block_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    namespace detail
    {
        struct block : std::enable_shared_from_this<block>
        {
            block() = default;

            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args,
                std::string const& name, std::string const& codename) const
            {
                if (operands.empty())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::"
                            "parallel_block_operation::eval",
                        generate_error_message(
                            "the parallel_block_operation primitive "
                                "requires at least one argument",
                            name, codename));
                }

                // evaluate condition of while statement
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](std::vector<primitive_argument_type> && ops)
                    ->  primitive_argument_type
                    {
                        return ops.back();
                    }),
                    detail::map_operands(
                        operands, functional::value_operand{}, args,
                        name, codename));
            }
        };
    }

    // start iteration over given parallel-block statement
    hpx::future<primitive_argument_type> parallel_block_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::block>()->eval(
                args, noargs, name_, codename_);
        }

        return std::make_shared<detail::block>()->eval(
            operands_, args, name_, codename_);
    }
}}}
