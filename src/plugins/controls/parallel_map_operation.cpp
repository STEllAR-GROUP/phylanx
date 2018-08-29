// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/parallel_map_operation.hpp>

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
    match_pattern_type const parallel_map_operation::match_data =
    {
        hpx::util::make_tuple("parallel_map",
            std::vector<std::string>{"parallel_map(_1, __2)"},
            &create_parallel_map_operation,
            &create_primitive<parallel_map_operation>,
            "func,listv\n"
            "Args:\n"
            "\n"
            "    func (function) : A function that takes a single argument\n"
            "    listv (iterator) : A sequence of values to apply the function to\n"
            "\n"
            "Returns:\n"
            "\n"
            "    A list of values obtained by apply `func` to every value it `listv`\n"
            "    in parallel.\n"
            "\n"
            "Examples:\n"
            "\n"
            "   print(parallel_map(lambda a : a*a, [1,2,3]))\n"
            "\n"
            "Evaluates to [1,4,9]\n"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    parallel_map_operation::parallel_map_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> parallel_map_operation::map_1(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& bound_func, ir::range&& list)
            ->  hpx::future<primitive_argument_type>
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "parallel_map_operation::map_1",
                        this_->generate_error_message(
                            "the first argument to map must be an invocable "
                                "object"));
                }

                // Concurrently evaluate all operations
                std::vector<hpx::future<primitive_argument_type>> result;
                result.reserve(list.size());

                for (auto && elem : list)
                {
                    // Evaluate function for each of the argument sets
                    std::vector<primitive_argument_type> args;
                    args.emplace_back(std::move(elem));
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
            value_operand(operands_[0], args, name_, codename_,
                eval_dont_evaluate_lambdas),
            list_operand_strict(operands[1], args, name_, codename_));
    }

    hpx::future<primitive_argument_type> parallel_map_operation::map_n(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        // all remaining operands have to be lists
        std::vector<primitive_argument_type> lists;
        std::copy(operands.begin() + 1, operands.end(),
            std::back_inserter(lists));

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& bound_func,
                std::vector<ir::range>&& lists)
            ->  hpx::future<primitive_argument_type>
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "parallel_map_operation::map_n",
                        this_->generate_error_message(
                            "the first argument to map must be an invocable "
                                "object"));
                }

                // Make sure all lists have the same size
                std::size_t size = lists[0].size();
                for (auto const& list : lists)
                {
                    if (list.size() != size)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "parallel_map_operation::map_n",
                            this_->generate_error_message(
                                "all list arguments must have the same length"));
                    }
                }

                // Concurrently evaluate all operations
                std::size_t numlists = lists.size();

                std::vector<ir::range_iterator> iters;
                iters.reserve(numlists);

                for (auto const& j : lists)
                {
                    iters.push_back(j.begin());
                }

                std::vector<hpx::future<primitive_argument_type>> result;
                result.reserve(size);

                for (std::size_t i = 0; i != size; ++i)
                {
                    std::vector<primitive_argument_type> args;
                    args.reserve(numlists);

                    // Each invocation has its own argument set
                    for (ir::range_iterator& j : iters)
                    {
                        args.push_back(*j++);
                    }

                    // Evaluate function for each of the argument sets
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
            value_operand(operands_[0], args, name_, codename_,
                eval_mode(eval_dont_wrap_functions |
                    eval_dont_evaluate_partials |
                    eval_dont_evaluate_lambdas)),
            detail::map_operands(lists, functional::list_operand_strict{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> parallel_map_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "parallel_map_operation::eval",
                generate_error_message(
                    "the parallel_map_operation primitive requires at "
                        "least two operands"));
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
                "parallel_map_operation::eval",
                generate_error_message(
                    "the parallel_map_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        // the first argument must be an invokable
        if (util::get_if<primitive>(&operands_[0]) == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "parallel_map_operation::eval",
                generate_error_message(
                    "the first argument to map must be an invocable object"));
        }

        // handle common case separately
        if (operands.size() == 2)
        {
            return map_1(operands, args);
        }

        return map_n(operands, args);
    }

    // Start iteration over given for statement
    hpx::future<primitive_argument_type> parallel_map_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
