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
      : primitive_component_base(std::move(operands), name, codename)
      , evaluated_(false)
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "variable::variable",
                execution_tree::generate_error_message(
                    "the variable primitive requires exactly one operand",
                    name_, codename_));
        }

        if (!operands_.empty())
        {
            operands_[0] = extract_copy_value(std::move(operands_[0]));
        }
    }

    primitive_argument_type variable::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        using lock_type = std::unique_lock<mutex_type>;
        lock_type l(mtx_);

        if (!evaluated_)
        {
            primitive const* p = util::get_if<primitive>(&operands_[0]);
            if (p != nullptr)
            {
                primitive_argument_type result;
                {
                    hpx::util::unlock_guard<lock_type> ul(l);
                    result = extract_copy_value(p->eval_direct(args));
                }
                operands_[0] = std::move(result);
            }
            evaluated_ = true;
        }

        return extract_ref_value(operands_[0]);
    }

    void variable::store(primitive_argument_type && data)
    {
        std::lock_guard<mutex_type> l(mtx_);
        operands_[0] = extract_copy_value(std::move(data));
        evaluated_ = true;
    }

    topology variable::expression_topology(std::set<std::string>&&) const
    {
        return topology{};
    }
}}}

