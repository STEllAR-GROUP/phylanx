// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/filter_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

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
    match_pattern_type const filter_operation::match_data =
    {
        hpx::util::make_tuple("filter",
            std::vector<std::string>{"filter(_1, _2)"},
            &create_filter_operation, &create_primitive<filter_operation>, R"(
            func, iter

            Args:

                func (function): a function that takes one arg and returns a
                                 boolean
                iter (iterator): an iterator

            Returns:\n"

            Filter applies `func` to each item in iter and creates a list
            consisting of the values for which `func` evaluated to true.

            Example:

                from phylanx.ast import Phylanx

                @Phylanx
                def foo():
                    print(filter(lambda a : a > 1, [1, 2, 3, 4]))

                foo()

            Prints [2, 3, 4])")
    };

    ///////////////////////////////////////////////////////////////////////////
    filter_operation::filter_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> filter_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "filter_operation::eval",
                generate_error_message(
                    "the filter_operation primitive requires exactly "
                        "two operands"));
        }

        if (!valid(operands[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "filter_operation::eval",
                generate_error_message(
                    "the filter_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        // the first argument must be an invokable
        if (util::get_if<primitive>(&operands_[0]) == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "filter_operation::eval",
                generate_error_message(
                    "the first argument to map must be an invocable object"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_), ctx](
                    primitive_argument_type&& bound_func, ir::range&& list)
            mutable ->  primitive_argument_type
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "filter_operation::eval",
                        this_->generate_error_message(
                            "the first argument to filter must be an "
                            "invocable object"));
                }

                // sequentially evaluate all operations
                primitive_arguments_type result;
                result.reserve(list.size());

                for (auto && curr : list)
                {
                    primitive_arguments_type arg2(1, extract_ref_value(curr));
                    if (scalar_boolean_operand_sync(bound_func, std::move(arg2),
                            this_->name_, this_->codename_, ctx))
                    {
                        result.push_back(std::move(curr));
                    }
                }

                return primitive_argument_type{std::move(result)};
            }),
            value_operand(operands[0], args, name_, codename_,
                add_mode(ctx, eval_dont_evaluate_lambdas)),
            list_operand(operands[1], args, name_, codename_, ctx));
    }
}}}
