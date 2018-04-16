// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/fold_left_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_fold_left_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("fold_left");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const fold_left_operation::match_data =
    {
        hpx::util::make_tuple("fold_left",
            std::vector<std::string>{"fold_left(_1, _2, _3)"},
            &create_fold_left_operation, &create_primitive<fold_left_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    fold_left_operation::fold_left_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> fold_left_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_left_operation::eval",
                execution_tree::generate_error_message(
                    "the fold_left_operation primitive requires exactly "
                        "three operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands_[1]) || !valid(operands_[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_left_operation::eval",
                execution_tree::generate_error_message(
                    "the fold_left_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        // the first argument must be an invokable
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_left_operation::eval",
                execution_tree::generate_error_message(
                    "the first argument to map must be an invocable "
                    "object", name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& bound_func,
                    primitive_argument_type&& initial,
                    std::vector<primitive_argument_type>&& list)
            -> primitive_argument_type
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "fold_left_operation::eval",
                        execution_tree::generate_error_message(
                            "the first argument to filter must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                // sequentially evaluate all operations
                for (auto&& elem : list)
                {
                    std::vector<primitive_argument_type> args(2);
                    args[0] = std::move(initial);
                    args[1] = std::move(elem);

                    initial = value_operand_sync(bound_func, std::move(args),
                        this_->name_, this_->codename_);
                }

                return primitive_argument_type{std::move(initial)};
            }),
            p->bind(args),
            value_operand(operands_[1], args, name_, codename_),
            list_operand(operands_[2], args, name_, codename_));
    }

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> fold_left_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
