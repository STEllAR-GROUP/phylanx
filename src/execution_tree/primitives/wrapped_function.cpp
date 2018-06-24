//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/wrapped_function.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const wrapped_function::match_data =
    {
        hpx::util::make_tuple("call-function",
            std::vector<std::string>{},
            nullptr, &create_primitive<wrapped_function>)
    };

    ///////////////////////////////////////////////////////////////////////////
    std::string extract_function_name(std::string const& name)
    {
        if (name.find("define-") == 0)
        {
            return name.substr(7);
        }
        return name;
    }

    wrapped_function::wrapped_function(
            std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
        // the first entry of operands represents the target
        if (operands_.empty() || !valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "wrapped_function::wrapped_function",
                execution_tree::generate_error_message(
                    "no target given",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> wrapped_function::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        std::vector<primitive_argument_type> fargs;
        fargs.reserve(operands_.size() - 1);

        // pass along pre-bound arguments
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            fargs.push_back(value_operand_sync(*it, params, name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](primitive_argument_type&& func,
                    std::vector<primitive_argument_type>&& args)
                {
                    return value_operand_sync(std::move(func), std::move(args),
                        this_->name_, this_->codename_);
                }),
            value_operand(operands_[0], params, name_, codename_),
            std::move(fargs));
    }

    primitive_argument_type wrapped_function::bind(
        std::vector<primitive_argument_type> const& params) const
    {
        // evaluation of the define-function yields the function body
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            std::vector<primitive_argument_type> fargs;
            fargs.reserve(operands_.size() - 1);

            // handle pre-bound arguments
            for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
            {
                // bind pre-bound arguments using actual ones
                primitive const* arg = util::get_if<primitive>(&*it);
                if (arg != nullptr)
                {
                    fargs.push_back(arg->bind(params));
                }
                else
                {
                    fargs.push_back(extract_ref_value(*it));
                }
            }

            p->bind(std::move(fargs));
        }
        return {};
    }

    topology wrapped_function::expression_topology(
        std::set<std::string>&& functions) const
    {
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(
                hpx::launch::sync, std::move(functions));
        }
        return {};
    }
}}}

