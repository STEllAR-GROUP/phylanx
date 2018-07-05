// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/assert_condition.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_assert_condition(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("assert");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const assert_condition::match_data =
    {
        hpx::util::make_tuple("assert",
            std::vector<std::string>{"assert(_1)"},
            &create_assert_condition, &create_primitive<assert_condition>)
    };

    ///////////////////////////////////////////////////////////////////////////
    assert_condition::assert_condition(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> assert_condition::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "assert_condition",
                execution_tree::generate_error_message(
                    "Assert requires exactly one argument", name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::uint8_t cond)
            -> primitive_argument_type
            {
                if (cond == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "assert_condition",
                        execution_tree::generate_error_message(
                            "Assertion failed",
                            this_->name_, this_->codename_));
                }

                return {};
            }),
            boolean_operand(operands_[0], args, name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> assert_condition::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
