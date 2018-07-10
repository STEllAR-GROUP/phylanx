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

#include <cstddef>
#include <memory>
#include <mutex>
#include <set>
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
      , value_set_(false)
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                generate_error_message(
                    "the variable primitive requires exactly one operand"));
        }

        if (valid(operands_[0]))
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
            value_set_ = true;
        }
    }

    hpx::future<primitive_argument_type> variable::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        primitive_argument_type const& target =
            valid(bound_value_) ? bound_value_ : operands_[0];
        return hpx::make_ready_future(extract_ref_value(target));
    }

    bool variable::bind(std::vector<primitive_argument_type> const& args) const
    {
        if (!value_set_)
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "variable::bind",
                generate_error_message(
                    "the expression representing the variable target "
                        "has not been initialized"));
        }

        bound_value_ = primitive_argument_type{};

        // evaluation of the define-function yields the function body
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            if (p->bind(args))
            {
                bound_value_ = p->eval(hpx::launch::sync, args);
                return true;
            }
            return false;
        }
        return true;
    }

    void variable::store(primitive_argument_type&& data)
    {
        if (!value_set_)
        {
            operands_[0] = extract_copy_value(std::move(data));
            value_set_ = true;
        }
        else
        {
            bound_value_ = extract_copy_value(std::move(data));
        }
    }

    void variable::set_num_arguments(std::size_t num_args)
    {
    }

    topology variable::expression_topology(
        std::set<std::string>&& functions) const
    {
        if (functions.find(name_) != functions.end())
        {
            return {};      // avoid recursion
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            functions.insert(name_);
            return p->expression_topology(
                hpx::launch::sync, std::move(functions));
        }
        return {};
    }
}}}

