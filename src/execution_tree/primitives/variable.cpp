//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::variable>
    literal_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    literal_type, phylanx_literal_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(literal_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    variable::variable(primitive_argument_type&& operand)
      : data_(extract_literal_value(operand))
    {}

    variable::variable(std::vector<primitive_argument_type>&& operands)
    {
        if (operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                "the variable primitive requires at most one operand");
        }

        if (!operands.empty())
        {
            data_ = extract_literal_value(operands[0]);
        }
    }

    hpx::future<primitive_result_type> variable::eval() const
    {
        return hpx::make_ready_future(data_);
    }

    void variable::store(primitive_result_type const& data)
    {
        data_ = data;
    }
}}}

