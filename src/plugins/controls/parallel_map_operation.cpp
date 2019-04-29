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
            R"(func, listv
            Args:

                func (function) : A function that takes a single argument
                listv (iterator) : A sequence of values to apply the function to

            Returns:

                A list of values obtained by apply `func` to every value it
                `listv` in parallel.

            Examples:

               print(parallel_map(lambda a : a * a, [1, 2, 3]))

            Evaluates to [1, 4, 9])"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    parallel_map_operation::parallel_map_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> parallel_map_operation::map_1(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto&& op0 = value_operand(operands[0], args, name_,
            codename_, add_mode(ctx, eval_dont_evaluate_lambdas));

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_), ctx](
                    hpx::future<primitive_argument_type>&& b,
                    hpx::future<ir::range>&& l)
            ->  hpx::future<primitive_argument_type>
            {
                auto&& bound_func = b.get();
                auto&& list = l.get();

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
                std::vector<hpx::future<primitive_argument_type>,
                    arguments_allocator<hpx::future<primitive_argument_type>>>
                    result;
                result.reserve(list.size());

                for (auto && elem : list)
                {
                    // Evaluate function for each of the argument sets
                    primitive_arguments_type args;
                    args.emplace_back(std::move(elem));
                    result.push_back(p->eval(std::move(args), ctx));
                }

                return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                    [](primitive_arguments_type&& result)
                    -> primitive_argument_type
                    {
                        return primitive_argument_type{std::move(result)};
                    }),
                    std::move(result));
            },
            std::move(op0),
            list_operand_strict(operands[1], args, name_, codename_,
                std::move(ctx)));
    }

    hpx::future<primitive_argument_type> parallel_map_operation::map_n(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // all remaining operands have to be lists
        primitive_arguments_type lists;
        lists.reserve(operands.size() - 1);

        std::copy(operands.begin() + 1, operands.end(),
            std::back_inserter(lists));

        auto&& op0 = value_operand(operands[0], args, name_, codename_,
            add_mode(ctx,
                eval_mode(eval_dont_wrap_functions |
                    eval_dont_evaluate_partials | eval_dont_evaluate_lambdas)));

        using range_list =
            std::vector<ir::range, arguments_allocator<ir::range>>;

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_), ctx](
                primitive_argument_type&& bound_func, range_list&& lists)
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

                std::vector<ir::range_iterator,
                    arguments_allocator<ir::range_iterator>> iters;
                iters.reserve(numlists);

                for (auto const& j : lists)
                {
                    iters.push_back(j.begin());
                }

                std::vector<hpx::future<primitive_argument_type>,
                    arguments_allocator<hpx::future<primitive_argument_type>>>
                    result;
                result.reserve(size);

                for (std::size_t i = 0; i != size; ++i)
                {
                    primitive_arguments_type args;
                    args.reserve(numlists);

                    // Each invocation has its own argument set
                    for (ir::range_iterator& j : iters)
                    {
                        args.push_back(*j++);
                    }

                    // Evaluate function for each of the argument sets
                    result.push_back(p->eval(std::move(args), ctx));
                }

                return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                    [](primitive_arguments_type&& result)
                    -> primitive_argument_type
                    {
                        return primitive_argument_type{std::move(result)};
                    }),
                    std::move(result));
            }),
            std::move(op0),
            detail::map_operands(lists,
                functional::list_operand_strict{}, args,
                name_, codename_, std::move(ctx)));
    }

    hpx::future<primitive_argument_type> parallel_map_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
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
            return map_1(operands, args, std::move(ctx));
        }

        return map_n(operands, args, std::move(ctx));
    }
}}}
