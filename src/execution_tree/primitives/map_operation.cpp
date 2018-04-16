// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/map_operation.hpp>

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
    primitive create_map_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("map");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const map_operation::match_data =
    {
        hpx::util::make_tuple("map",
            std::vector<std::string>{"map(_1, __2)"},
            &create_map_operation, &create_primitive<map_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    map_operation::map_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> map_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "map_operation::eval",
                execution_tree::generate_error_message(
                    "the map_operation primitive requires at "
                        "least two operands",
                    name_, codename_));
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
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "map_operation::eval",
                execution_tree::generate_error_message(
                    "the map_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        // the first argument must be an invokable
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "map_operation::eval",
                execution_tree::generate_error_message(
                    "the first argument to map must be an invocable "
                    "object", name_, codename_));
        }

        // all remaining operands have to be lists
        std::vector<primitive_argument_type> lists;
        std::copy(operands.begin() + 1, operands.end(),
            std::back_inserter(lists));

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& bound_func,
                    std::vector<std::vector<primitive_argument_type>>&& lists)
            -> hpx::future<primitive_argument_type>
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "map_operation::eval",
                        execution_tree::generate_error_message(
                            "the first argument to map must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                // make sure all lists have the same size
                std::size_t size = lists[0].size();
                for (auto const& list : lists)
                {
                    if (list.size() != size)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "map_operation::eval",
                            execution_tree::generate_error_message(
                                "all list arguments must have the same length",
                                this_->name_, this_->codename_));
                    }
                }

                // concurrently evaluate all operations
                std::size_t numlists = lists.size();

                std::vector<hpx::future<primitive_argument_type>> result;
                result.reserve(numlists);

                for (std::size_t i = 0; i != size; ++i)
                {
                    std::vector<primitive_argument_type> args;
                    args.resize(numlists);

                    // each invocation has its own argument set
                    for (std::size_t j = 0; j != numlists; ++j)
                    {
                        args[j] = lists[j][i];
                    }

                    // evaluate function for each of the argument sets
                    result.push_back(p->eval(std::move(args)));
                }

                return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                    [](std::vector<primitive_argument_type>&& result)
                    -> primitive_argument_type
                    {
                        return primitive_argument_type{std::move(result)};
                    }),
                    std::move(result));
            }),
            p->bind(args),
            detail::map_operands(
                lists, functional::list_operand{}, args,
                name_, codename_));
    }

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> map_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
