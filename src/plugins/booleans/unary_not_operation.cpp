//  Copyright (c) 2017-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/booleans/unary_not_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    static constexpr char const* const help_string = R"(
        a
        Args:

            a (boolean) : a boolean argument

        Returns:

        The opposite of `a`.
    )";

    match_pattern_type const unary_not_operation::match_data[2] =
    {
        match_pattern_type("__not",
            std::vector<std::string>{"!_1", "__not(_1)"},
            &create_unary_not_operation,
            &create_primitive<unary_not_operation>, help_string),

        match_pattern_type("logical_not",
            std::vector<std::string>{"logical_not(_1)"},
            &create_unary_not_operation,
            &create_primitive<unary_not_operation>, help_string)
    };

    ///////////////////////////////////////////////////////////////////////////
    unary_not_operation::unary_not_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type unary_not_operation::unary_not_all(
        ir::node_data<T> && ops) const
    {
        std::size_t dims = ops.num_dimensions();
        switch (dims)
        {
        case 0:
            return primitive_argument_type{
                ir::node_data<std::uint8_t>{ops.scalar() == T(0)}};

        case 1:
            // TODO: SIMD functionality should be added, blaze implementation
            // is not currently available
            return primitive_argument_type{
                ir::node_data<std::uint8_t>{blaze::map(ops.vector(),
                    [](T x) -> std::uint8_t { return x == T(0); })}};

        case 2:
            // TODO: SIMD functionality should be added, blaze implementation
            // is not currently available
            return primitive_argument_type{
                ir::node_data<std::uint8_t>{blaze::map(ops.matrix(),
                    [](T x) -> std::uint8_t { return x == T(0); })}};

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            // TODO: SIMD functionality should be added, blaze implementation
            // is not currently available
            return primitive_argument_type{
                ir::node_data<std::uint8_t>{blaze::map(ops.tensor(),
                    [](T x) -> std::uint8_t { return x == T(0); })}};
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::eval",
                generate_error_message(
                    "operand has unsupported number of dimensions"));
        }
    }

    struct unary_not_operation::visit_unary_not
    {
        template <typename T>
        primitive_argument_type operator()(T&& ops) const
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not::eval",
                that_.generate_error_message(
                    "operand has unsupported type"));
        }

        primitive_argument_type operator()(ast::nil&&) const
        {
            return primitive_argument_type{true};    // not None
        }

        template <typename T>
        primitive_argument_type operator()(ir::node_data<T>&& ops) const
        {
            return that_.unary_not_all(std::move(ops));
        }

        unary_not_operation const& that_;
    };

    hpx::future<primitive_argument_type> unary_not_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // TODO: support for operands.size() > 1
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::unary_not_operation",
                generate_error_message(
                    "the unary_not_operation primitive requires "
                        "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "unary_not_operation::unary_not_operation",
                generate_error_message(
                    "the unary_not_operation primitive requires that "
                        "the argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& op)
            -> primitive_argument_type
            {
                return primitive_argument_type(
                    util::visit(visit_unary_not{*this_},
                        std::move(op.get().variant())));

            },
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
