// Copyright (c) 2017 Alireza Kheirkhahan
// Copyright (c) 2018 Bibek Wagle
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
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("store");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const store_operation::match_data =
    {
        hpx::util::make_tuple("store",
            std::vector<std::string>{"store(_1, __2)"},
            &create_store_operation, &create_primitive<store_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    store_operation::store_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename, true)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> store_operation::eval(
        std::vector<primitive_argument_type> const& args, eval_mode) const
    {
        std::size_t operands_size = operands_.size();
        if (operands_size < 2 || operands_size > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::eval",
                generate_error_message(
                    "the store_operation primitive requires either "
                        "two, three, or four operands"));
        }

        for (std::size_t i = 0; i < operands_size; ++i)
        {
            if (!valid(operands_[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "store_operation::store_operation",
                    generate_error_message(
                        "the store_operation primitive requires that "
                        "the arguments given by the operands array is valid"));
            }
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
        if (operands_size == 2)
        {
            return value_operand(operands_[1], args, name_, codename_)
                .then(hpx::launch::sync,
                    [this_, lhs = extract_ref_value(operands_[0])](
                            hpx::future<primitive_argument_type>&& val)
                    ->  primitive_argument_type
                    {
                        primitive_operand(lhs, this_->name_, this_->codename_)
                            .store(hpx::launch::sync, val.get());
                        return primitive_argument_type{};
                    });
        }

        std::vector<primitive_argument_type> params;
        params.reserve(operands_size - 1);
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            params.emplace_back(std::move(*it));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_, lhs = extract_ref_value(operands_[0])](
                    std::vector<primitive_argument_type>&& args)
            ->  primitive_argument_type
            {
                primitive_operand(lhs, this_->name_, this_->codename_)
                    .store(hpx::launch::sync, std::move(args));
                return primitive_argument_type{};
            }),
            detail::map_operands(std::move(params),
                functional::value_operand{}, args, name_, codename_));
    }

    hpx::future<primitive_argument_type> store_operation::eval(
        primitive_argument_type&& arg, eval_mode) const
    {
        std::size_t operands_size = operands_.size();
        if (operands_size < 2 || operands_size > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "store_operation::eval",
                generate_error_message(
                    "the store_operation primitive requires either "
                        "two, three, or four operands"));
        }

        for (std::size_t i = 0; i < operands_size; ++i)
        {
            if (!valid(operands_[i]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "store_operation::store_operation",
                    generate_error_message(
                        "the store_operation primitive requires that "
                        "the arguments given by the operands array is valid"));
            }
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
        if (operands_size == 2)
        {
            return value_operand(operands_[1], std::move(arg), name_, codename_)
                .then(hpx::launch::sync,
                    [this_, lhs = extract_ref_value(operands_[0])](
                            hpx::future<primitive_argument_type>&& val)
                    ->  primitive_argument_type
                    {
                        primitive_operand(lhs, this_->name_, this_->codename_)
                            .store(hpx::launch::sync, val.get());
                        return primitive_argument_type{};
                    });
        }

        std::vector<primitive_argument_type> params;
        params.reserve(operands_size - 1);
        for (auto it = operands_.begin() + 1; it != operands_.end(); ++it)
        {
            params.emplace_back(std::move(*it));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_, lhs = extract_ref_value(operands_[0])](
                    std::vector<primitive_argument_type>&& args)
            ->  primitive_argument_type
            {
                primitive_operand(lhs, this_->name_, this_->codename_)
                    .store(hpx::launch::sync, std::move(args));
                return primitive_argument_type{};
            }),
            detail::map_operands(std::move(params),
                functional::value_operand{}, std::move(arg), name_, codename_));
    }
}}}

