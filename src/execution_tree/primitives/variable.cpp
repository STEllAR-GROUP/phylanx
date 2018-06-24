//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/variable.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/unlock_guard.hpp>

#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_variable(hpx::id_type const& locality,
        primitive_argument_type&& operand, std::string const& name,
        std::string const& codename)
    {
        static std::string type("variable");
        return create_primitive_component(
            locality, type, std::move(operand), name, codename);
    }

    match_pattern_type const variable::match_data =
    {
        hpx::util::make_tuple("variable",
            std::vector<std::string>{},
            nullptr, &create_primitive<variable>)
    };

    ///////////////////////////////////////////////////////////////////////////
    variable::variable(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                execution_tree::generate_error_message(
                    "the variable primitive requires exactly one operand",
                    name_, codename_));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    hpx::future<primitive_argument_type> variable::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        return hpx::make_ready_future(value_);
    }

    primitive_argument_type variable::bind(
        std::vector<primitive_argument_type> const& args) const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::bind",
                execution_tree::generate_error_message(
                    "the expression representing the variable target "
                        "has not been initialized",
                    name_, codename_));
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            value_ = p->bind(args);
        }
        else
        {
            value_ = extract_ref_value(operands_[0]);
        }
        return {};
    }

    void variable::store(primitive_argument_type && data)
    {
        operands_[0] = extract_copy_value(std::move(data));
    }

    topology variable::expression_topology(std::set<std::string>&&) const
    {
        return topology{};
    }
}}}

