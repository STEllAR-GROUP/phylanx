//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/apply.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/throw_exception.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::apply>
    apply_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    apply_type, phylanx_apply_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(apply_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const apply::match_data =
    {
        hpx::util::make_tuple("apply",
            std::vector<std::string>{"apply(_1, _2)"},
            &create<apply>)
    };

    ///////////////////////////////////////////////////////////////////////////
    apply::apply(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    primitive_argument_type apply::eval_direct(
        std::vector<primitive_argument_type> const& params) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply::eval_direct",
                "the apply primitive requires exactly two operands");
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "apply::eval_direct",
                "the first argument to apply must be an invocable object");
        }

        return p->eval_direct(list_operand_sync(operands_[1], params));
    }
}}}
