//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/apply.hpp>

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
    primitive create_apply(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("apply");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const apply::match_data =
    {
        hpx::util::make_tuple("apply",
            std::vector<std::string>{"apply(_1, _2)"},
            &create_apply, &create_primitive<apply>)
    };

    ///////////////////////////////////////////////////////////////////////////
    apply::apply(std::vector<primitive_argument_type>&& operands)
      : primitive_component_base(std::move(operands))
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
