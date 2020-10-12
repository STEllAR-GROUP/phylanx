// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/assert_condition.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/iostream.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    primitive create_assert_condition(hpx::id_type const& locality,
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
    {
        static std::string type("assert");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const assert_condition::match_data = {
        hpx::make_tuple("assert",
            std::vector<std::string>{"assert(_1_test, __arg(_2_msg, nil))"},
            &create_assert_condition, &create_primitive<assert_condition>,
            R"(cond, msg
            Args:

                cond (boolean expression) : if not true, raise an AssertionError
                msg (any expression) : if given, the evaluated result is printed

            Returns:)")};

    ///////////////////////////////////////////////////////////////////////////
    assert_condition::assert_condition(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    hpx::future<primitive_argument_type> assert_condition::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "assert_condition",
                generate_error_message(
                    "Assert requires exactly one or two argument(s)",
                    std::move(ctx)));
        }

        auto arg =
            scalar_boolean_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        if (operands.size() == 1 || !valid(operands[1]))
        {
            return hpx::dataflow(
                hpx::launch::sync,
                [this_ = std::move(this_), ctx = std::move(ctx)](
                    hpx::future<std::uint8_t>&& cond) mutable
                    -> primitive_argument_type {
                    if (cond.get() == 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "assert_condition",
                            this_->generate_error_message(
                                "Assertion failed", std::move(ctx)));
                    }
                    return {};
                },
                std::move(arg));
        }

        auto msg = value_operand(operands[1], args, name_, codename_, ctx);

        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                hpx::future<std::uint8_t>&& cond,
                hpx::future<primitive_argument_type>&& msg) mutable
                -> primitive_argument_type {
                if (cond.get() == 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "assert_condition",
                        this_->generate_error_message(
                            hpx::util::format(
                                "Assertion failed: {}", to_string(msg.get())),
                            std::move(ctx)));
                }
                return {};
            },
            std::move(arg), std::move(msg));
    }
}}}    // namespace phylanx::execution_tree::primitives
