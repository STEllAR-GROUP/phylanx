// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/add_operation.hpp>
#include <phylanx/plugins/arithmetics/numeric_impl.hpp>

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
    namespace detail
    {
        struct add_op
        {
            template <typename T1, typename T2>
            auto operator()(T1 const& t1, T2 const& t2) const
            ->  decltype(t1 + t2)
            {
                return t1 + t2;
            }

            template <typename T1, typename T2>
            void op_assign(T1& t1, T2 const& t2) const
            {
                t1 += t2;
            }
        };
    }

    //////////////////////////////////////////////////////////////////////////
    match_pattern_type const add_operation::match_data =
    {
        match_pattern_type{"__add",
            std::vector<std::string>{"_1 + __2", "__add(_1, __2)"},
            &create_add_operation, &create_primitive<add_operation>, R"(
            x0, x1
            Args:

                 x0 (number): An addend\n"
                *x1 (number list): A list of one or more addends.\n"

            Returns:

            The sum of all addends.)",
            true
        }
    };

    //////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    void add_operation::append_element(primitive_arguments_type& result,
        primitive_argument_type&& rhs) const
    {
        if (is_list_operand_strict(rhs))
        {
            auto&& rhs_list =
                extract_list_value_strict(std::move(rhs), name_, codename_);

            for (auto&& elem : std::move(rhs_list))
            {
                result.emplace_back(std::move(elem));
            }
        }
        else
        {
            result.emplace_back(std::move(rhs));
        }
    }

    primitive_argument_type add_operation::handle_list_operands(
        primitive_argument_type&& op1, primitive_argument_type&& rhs) const
    {
        ir::range lhs =
            extract_list_value_strict(std::move(op1), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            append_element(result, std::move(rhs));
            return primitive_argument_type{std::move(result)};
        }

        append_element(lhs.args(), std::move(rhs));
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::handle_list_operands(
        primitive_arguments_type&& ops) const
    {
        auto it = ops.begin();
        auto end = ops.end();

        ir::range lhs =
            extract_list_value_strict(std::move(ops[0]), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            for (++it; it != end; ++it)
            {
                append_element(result, std::move(*it));
            }
            return primitive_argument_type{std::move(result)};
        }

        for (++it; it != end; ++it)
        {
            append_element(lhs.args(), std::move(*it));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> add_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type&& args, eval_context ctx) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "add_operation::eval",
                generate_error_message("the add_operation primitive requires "
                    "at least two operands"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "add_operation::eval",
                generate_error_message(
                    "the add_operation primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            // special case for 2 operands
            auto&& op0 =
                value_operand(operands[0], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& lhs,
                    hpx::future<primitive_argument_type>&& rhs)
                -> primitive_argument_type
                {
                    auto && lhs_val = lhs.get();
                    if (is_list_operand_strict(lhs_val))
                    {
                        return this_->handle_list_operands(
                            std::move(lhs_val), rhs.get());
                    }
                    return this_->handle_numeric_operands(
                        std::move(lhs_val), rhs.get());
                },
                std::move(op0),
                value_operand(operands[1], std::move(args), name_, codename_,
                    std::move(ctx)));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& ops)
            ->  primitive_argument_type
            {
                if (is_list_operand_strict(ops[0]))
                {
                    return this_->handle_list_operands(std::move(ops));
                }
                return this_->handle_numeric_operands(std::move(ops));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, std::move(args),
                name_, codename_, std::move(ctx)));
    }
}}}
