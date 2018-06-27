//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/synchronize.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/unlock_guard.hpp>

#include <mutex>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_synchronize(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        return create_primitive_component(
            locality, "synchronize", std::move(operands), name, codename);
    }

    match_pattern_type const synchronize::match_data =
    {
        hpx::util::make_tuple("synchronize",
            std::vector<std::string>{"synchronize(_1)"},
            &create_synchronize, &create_primitive<synchronize>)
    };

    ///////////////////////////////////////////////////////////////////////////
    synchronize::synchronize(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {
    }

    hpx::future<primitive_argument_type> synchronize::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "synchronize::synchronize",
                execution_tree::generate_error_message(
                    "the synchronize primitive requires exactly one operand",
                    name_, codename_));
        }

        std::lock_guard<mutex_type> l(mtx_);

        return value_operand(operands[0], args, name_, codename_);
    }

    hpx::future<primitive_argument_type> synchronize::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}

