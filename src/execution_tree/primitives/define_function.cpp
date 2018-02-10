//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/define_function.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const define_function::match_data =
    {
        hpx::util::make_tuple("define-function",
            std::vector<std::string>{},
            nullptr, &create_primitive_with_name<define_function>)
    };

    ///////////////////////////////////////////////////////////////////////////
    define_function::define_function(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name)
      : primitive_component_base(std::move(operands))
      , name_(name)
    {
        if (!operands_.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "define_function::define_function",
                "the define_function primitive is expected to be initialized "
                    "without arguments");
        }
        operands_.resize(1);
    }

    primitive_argument_type define_function::eval_direct(
        std::vector<primitive_argument_type> const& args) const
    {
        // body is assumed to be operands_[0]
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_function::eval_direct",
                "expression representing the function body was not "
                    "initialized yet");
        }
        return operands_[0];
    }

    void define_function::set_body(primitive_argument_type&& body)
    {
        if (valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_function::set_body",
                "expression representing the function body was already "
                    "initialized");
        }
        operands_[0] = std::move(body);
    }

    topology define_function::expression_topology() const
    {
        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "define_function::expression_topology",
                "expression representing the function body was not "
                    "initialized yet");
        }

        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p != nullptr)
        {
            return p->expression_topology(hpx::launch::sync);
        }

        return {};
    }
}}}

