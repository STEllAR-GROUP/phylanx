// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/controls/fold_left_operation.hpp>

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
    match_pattern_type const fold_left_operation::match_data =
    {
        hpx::util::make_tuple("fold_left",
            std::vector<std::string>{"fold_left(_1, _2, _3)"},
            &create_fold_left_operation,
            &create_primitive<fold_left_operation>,
            R"(fun, ini, range

            Args:

                fun (function) : a function that takes a two float arguments
                                 and returns a float argument
                ini (float) : an initial value
                range (iterator) : a list or iterator

            Returns:

                This function is equivalent to the Python code:

              def fl(f, i, r):
                  c = i
                  for n in r:
                      c = f(c, n)
                  return c

            Example(s):

              @Phylanx
              def foo():
                  v = fold_left(lambda a, b : 2 * a - b, 3, [1, 2, 3])
                  print(v)
              foo()

            Result:
              13)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    fold_left_operation::fold_left_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> fold_left_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_left_operation::eval",
                util::generate_error_message(
                    "the fold_left_operation primitive requires exactly "
                    "three operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) || !valid(operands_[1]) || !valid(operands_[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_left_operation::eval",
                util::generate_error_message(
                    "the fold_left_operation primitive requires that the "
                    "arguments given by the operands array "
                    "are valid",
                    name_, codename_));
        }

        // the first argument must be an invokable
        if (util::get_if<primitive>(&operands_[0]) == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_left_operation::eval",
                util::generate_error_message(
                    "the first argument to map must be an invocable "
                    "object", name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_), ctx](
                    primitive_argument_type&& bound_func,
                    primitive_argument_type&& initial, ir::range&& list)
            ->  primitive_argument_type
            {
                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "fold_left_operation::eval",
                        util::generate_error_message(
                            "the first argument to filter must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                // sequentially evaluate all operations
                for (auto&& elem : list)
                {
                    primitive_arguments_type args(2);
                    args[0] = std::move(initial);
                    args[1] = std::move(elem);

                    initial = value_operand_sync(bound_func, std::move(args),
                        this_->name_, this_->codename_, ctx);
                }

                return primitive_argument_type{std::move(initial)};
            }),
            value_operand(operands_[0], args, name_, codename_,
                add_mode(ctx, eval_mode(eval_dont_evaluate_lambdas |
                    eval_dont_evaluate_partials))),
            value_operand(operands_[1], args, name_, codename_, ctx),
            list_operand(operands_[2], args, name_, codename_, ctx));
    }
}}}
