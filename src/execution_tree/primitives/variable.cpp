//  Copyright (c) 2017-2018 Hartmut Kaiser
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
      : data_(std::move(name))
      , evaluated_(false)
    {}

    variable::variable(primitive_argument_type&& operand)
      : data_(extract_copy_value(std::move(operand)))
      , evaluated_(true)
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
            data_ = extract_copy_value(std::move(operands[0]));
            evaluated_ = true;
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
            data_ = extract_copy_value(std::move(operands[0]));
            evaluated_ = true;
        }
    }

    primitive_result_type variable::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        if (!evaluated_)
        {
            primitive const* p = util::get_if<primitive>(&data_);
            if (p != nullptr)
            {
                data_ = extract_copy_value(p->eval_direct(args));
                evaluated_ = true;
            }
        }
        return extract_ref_value(data_);
    }

    void variable::store(primitive_result_type && data)
    {
        data_ = extract_copy_value(std::move(data));
        evaluated_ = true;
    }

    bool variable::bind(std::vector<primitive_argument_type> const& params)
    {
        primitive* p = util::get_if<primitive>(&data_);
        return (p != nullptr) ? p->bind(hpx::launch::sync, params) : true;
    }
}}}

