// Copyright (c) 2017 Alireza Kheirkhahan
// Copyright (c) 2018 Bibek Wagle
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/store_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_store_operation(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("store");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const store_operation::match_data =
    {
        hpx::util::make_tuple("store",
            std::vector<std::string>{"store(_1, _2)"},
            &create_store_operation, &create_primitive<store_operation>,
            R"(var, value
            Update the definition of variable `var` with value `value`.
            This primitive is used to implement the assignment operator.
            Note that the variable should first be created with define.
            Args:

                var (symbol) : a variable to be defined
                value (expression) : a value to assign

            Returns:)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    store_operation::store_operation(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> store_operation::eval(
        primitive_arguments_type&& args, eval_context ctx) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::eval",
                generate_error_message(
                    "the store_operation primitive requires exactly "
                        "two operands"));
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                generate_error_message(
                    "the store_operation primitive requires that "
                    "the arguments given by the operands array is valid"));
        }

        if (!is_primitive_operand(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                generate_error_message(
                    "the first argument of the store primitive must "
                        "refer to a another primitive and can't be a "
                        "literal value"));
        }

        auto f = value_operand(operands_[1], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return f.then(hpx::launch::sync,
            [
                this_ = std::move(this_),
                args = std::move(args),
                ctx = std::move(ctx)
            ]
            (hpx::future<primitive_argument_type>&& val) mutable
            ->  primitive_argument_type
            {
                auto p = primitive_operand(
                    this_->operands_[0], this_->name_, this_->codename_);

                p.store(hpx::launch::sync, val.get(), std::move(args),
                    std::move(ctx));
                return primitive_argument_type{};
            });
    }

    hpx::future<primitive_argument_type> store_operation::eval(
        primitive_argument_type&& arg, eval_context ctx) const
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::eval",
                generate_error_message(
                    "the store_operation primitive requires exactly "
                        "two  operands"));
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                generate_error_message(
                    "the store_operation primitive requires that "
                    "the arguments given by the operands array is valid"));
        }

        if (!is_primitive_operand(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::store_operation",
                generate_error_message(
                    "the first argument of the store primitive must "
                        "refer to a another primitive and can't be a "
                        "literal value"));
        }

        auto this_ = this->shared_from_this();

        primitive_arguments_type params;
        params.emplace_back(extract_ref_value(arg, name_, codename_));

        auto f =
            value_operand(operands_[1], std::move(arg), name_, codename_, ctx);

        return f.then(hpx::launch::sync,
                [
                    this_ = std::move(this_),
                    args = std::move(params),
                    ctx = std::move(ctx)
                ]
                (hpx::future<primitive_argument_type>&& val) mutable
                -> primitive_argument_type
                {
                    auto p = primitive_operand(
                        this_->operands_[0], this_->name_, this_->codename_);

                    p.store(hpx::launch::sync, val.get(), std::move(args),
                        std::move(ctx));
                    return primitive_argument_type{};
                });
    }
}}}

