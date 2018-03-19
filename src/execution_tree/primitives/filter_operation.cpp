// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/filter_operation.hpp>

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
    primitive create_filter_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("filter");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const filter_operation::match_data =
    {
        hpx::util::make_tuple("filter",
            std::vector<std::string>{"filter(_1, _2)"},
            &create_filter_operation, &create_primitive<filter_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    filter_operation::filter_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> filter_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "filter_operation::eval",
                execution_tree::generate_error_message(
                    "the filter_operation primitive requires exactly "
                        "two operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "filter_operation::eval",
                execution_tree::generate_error_message(
                    "the filter_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        // the first argument must be an invokable
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "filter_operation::eval",
                execution_tree::generate_error_message(
                    "the first argument to map must be an invocable "
                    "object", name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](primitive_argument_type&& bound_func,
                    std::vector<primitive_argument_type>&& list)
            -> primitive_argument_type
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "filter_operation::eval",
                        execution_tree::generate_error_message(
                            "the first argument to filter must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                // sequentially evaluate all operations
                std::size_t size = list.size();

                std::vector<primitive_argument_type> result;
                result.reserve(size);

                for (std::size_t i = 0; i != size; ++i)
                {
                    std::vector<primitive_argument_type> arg(1, list[i]);
                    if (boolean_operand_sync(bound_func, std::move(arg),
                            this_->name_, this_->codename_))
                    {
                        result.push_back(std::move(list[i]));
                    }
                }

                return primitive_argument_type{std::move(result)};
            }),
            p->bind(args),
            list_operand(operands_[1], args, name_, codename_));
    }

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> filter_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
