//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>

#include <string>
#include <utility>
#include <vector>

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
    variable::variable(std::string name)
      : name_(std::move(name))
      , evaluated_(false)
    {}

    variable::variable(primitive_argument_type&& operand)
      : data_(std::move(operand))
      , evaluated_(false)
    {}

    variable::variable(std::vector<primitive_argument_type>&& operands)
      : evaluated_(false)
    {
        if (operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                "the variable primitive requires at most one operand");
        }

        if (!operands.empty())
        {
            data_ = std::move(operands[0]);
        }
    }

    variable::variable(primitive_argument_type&& operand, std::string name)
      : data_(std::move(operand))
      , name_(std::move(name))
      , evaluated_(false)
    {}

    variable::variable(std::vector<primitive_argument_type>&& operands,
            std::string name)
      : name_(std::move(name))
      , evaluated_(false)
    {
        if (operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                "the variable primitive requires at most one operand");
        }

        if (!operands.empty())
        {
            data_ = std::move(operands[0]);
        }
    }

    primitive_result_type variable::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        primitive const* p = util::get_if<primitive>(&data_);
        if (!evaluated_ && p != nullptr)
        {
            evaluated_ = true;
            data_ = p->eval_direct(args);
        }
        return data_;
    }

    void variable::store(primitive_result_type && data)
    {
        data_ = std::move(data);
    }

    bool variable::bind(std::vector<primitive_argument_type> const& params)
    {
        primitive* p = util::get_if<primitive>(&data_);
        return (p != nullptr) ? p->bind(hpx::launch::sync, params) : true;
    }
}}}

