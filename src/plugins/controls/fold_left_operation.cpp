// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/controls/fold_left_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <phylanx/util/tensor_iterators.hpp>
#endif

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const fold_left_operation::match_data =
    {
        hpx::util::make_tuple("fold_left",
            std::vector<std::string>{"fold_left(_1_func, _2_initial, _3_data)"},
            &create_fold_left_operation,
            &create_primitive<fold_left_operation>,
            R"(func, initial, data

            Args:

                func : a function that takes two arbitrary arguments
                       and returns the result of folding the two arguments
                initial : an initial value
                data : a list or an array-like object

            Returns:

                The result of left-folding the elements of the data object
                using the given function.

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

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type fold_left_operation::fold_left_list(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        primitive_argument_type&& data, eval_context ctx) const
    {
        ir::range&& list = extract_list_value_strict(data, name_, codename_);

        // sequentially evaluate bound_func for all elements of the list
        std::size_t index = 0;
        for (auto&& elem : list)
        {
            // the initial value is allowed to be nil in which case fold_left
            // will use the first element of the list as its initial value
            if (0 == index++ && !valid(initial))
            {
                initial = std::move(elem);
                continue;
            }

            primitive_arguments_type args(2);
            args[0] = value_operand_sync(std::move(initial),
                noargs, name_, codename_, ctx);

            args[1] = value_operand_sync(std::move(elem),
                noargs, name_, codename_, ctx);

            initial = value_operand_sync(
                bound_func, std::move(args), name_, codename_, ctx);
        }

        return primitive_argument_type{value_operand_sync(
            std::move(initial), noargs, name_, codename_, ctx)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type fold_left_operation::fold_left_array_helper_1d(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        std::size_t index = 0;
        for (auto && elem : data.vector())
        {
            // the initial value is allowed to be nil in which case fold_left
            // will use the first element of the list as its initial value
            if (0 == index++)
            {
                if (!valid(initial))
                {
                    initial = primitive_argument_type{elem};
                    continue;
                }
            }

            primitive_arguments_type args(2);
            args[0] = extract_node_data<T>(std::move(initial), name_, codename_);
            args[1] = primitive_argument_type{elem};

            initial = value_operand_sync(
                bound_func, std::move(args), name_, codename_, ctx);
        }
        return primitive_argument_type{std::move(initial)};
    }

    template <typename T>
    primitive_argument_type fold_left_operation::fold_left_array_helper_2d(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        using vector_type = typename ir::node_data<T>::storage1d_type;
        using matrix_type = typename ir::node_data<T>::custom_storage2d_type;

        using iterator = util::matrix_row_iterator<matrix_type>;

        auto m = data.matrix();

        auto begin = iterator(m);
        auto end = iterator(m, m.rows());
        for (auto it = begin; it != end; ++it)
        {
            // the initial value is allowed to be nil in which case fold_left
            // will use the first element of the list as its initial value
            if (it == begin)
            {
                if (!valid(initial))
                {
                    initial =
                        primitive_argument_type{vector_type(blaze::trans(*it))};
                    continue;
                }
            }

            primitive_arguments_type args(2);
            args[0] = extract_node_data<T>(std::move(initial), name_, codename_);
            args[1] = primitive_argument_type{vector_type(blaze::trans(*it))};

            initial = value_operand_sync(
                bound_func, std::move(args), name_, codename_, ctx);
        }
        return primitive_argument_type{std::move(initial)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type fold_left_operation::fold_left_array_helper_3d(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        using matrix_type = typename ir::node_data<T>::storage2d_type;
        using tensor_type = typename ir::node_data<T>::custom_storage3d_type;

        using iterator = util::tensor_pageslice_iterator<tensor_type>;

        auto t = data.tensor();

        auto begin = iterator(t);
        auto end = iterator(t, t.pages());
        for (auto it = begin; it != end; ++it)
        {
            // the initial value is allowed to be nil in which case fold_left
            // will use the first element of the list as its initial value
            if (it == begin)
            {
                if (!valid(initial))
                {
                    initial = primitive_argument_type{matrix_type(*it)};
                    continue;
                }
            }

            primitive_arguments_type args(2);
            args[0] = extract_node_data<T>(std::move(initial), name_, codename_);
            args[1] = primitive_argument_type{matrix_type(*it)};

            initial = value_operand_sync(
                bound_func, std::move(args), name_, codename_, ctx);
        }
        return primitive_argument_type{std::move(initial)};
    }
#endif

    template <typename T>
    primitive_argument_type fold_left_operation::fold_left_array_helper(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        if (valid(initial))
        {
            initial = value_operand_sync(std::move(initial), name_, codename_);
        }

        switch (extract_numeric_value_dimension(data, name_, codename_))
        {
        case 0:     // scalars are not allowed
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "fold_left_operation::fold_left_array_helper",
                    generate_error_message(
                        "the fold_left primitive requires for its data "
                        "argument to be a numeric array data type "
                        "(non-zero dimensional)"));
            }
            break;

        case 1:
            initial = fold_left_array_helper_1d(std::move(bound_func),
                std::move(initial), extract_node_data<T>(std::move(data)),
                std::move(ctx));
            break;

        case 2:
            initial = fold_left_array_helper_2d(std::move(bound_func),
                std::move(initial), extract_node_data<T>(std::move(data)),
                std::move(ctx));
            break;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            initial = fold_left_array_helper_3d(std::move(bound_func),
                std::move(initial), extract_node_data<T>(std::move(data)),
                std::move(ctx));
            break;
#endif
        }

        return primitive_argument_type{std::move(initial)};
    }

    primitive_argument_type fold_left_operation::fold_left_array(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        primitive_argument_type&& data, eval_context ctx) const
    {
        node_data_type dtype = extract_common_type(data, initial);
        switch (dtype)
        {
        case node_data_type_bool:
            return fold_left_array_helper(std::move(bound_func),
                std::move(initial),
                extract_node_data<std::uint8_t>(std::move(data)),
                std::move(ctx));

        case node_data_type_int64:
            return fold_left_array_helper(std::move(bound_func),
                std::move(initial),
                extract_node_data<std::int64_t>(std::move(data)),
                std::move(ctx));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return fold_left_array_helper(std::move(bound_func),
                std::move(initial), extract_node_data<double>(std::move(data)),
                std::move(ctx));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "fold_left_operation::fold_left_array",
            generate_error_message(
                "the fold_left primitive requires for its data argument to "
                "be a numeric data type"));
    }

    ///////////////////////////////////////////////////////////////////////////
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

        // note: the initial value is allowed to be nil
        if (!valid(operands[0]) || !valid(operands_[2]))
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
                    primitive_argument_type&& initial,
                    primitive_argument_type&& data)
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

                // handle list separately from arrays
                if (is_list_operand_strict(data))
                {
                    return this_->fold_left_list(std::move(bound_func),
                        std::move(initial), std::move(data), std::move(ctx));
                }

                if (!is_numeric_operand(data))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "fold_left_operation::eval",
                        util::generate_error_message(
                            "the first argument to filter must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                return this_->fold_left_array(std::move(bound_func),
                    std::move(initial), std::move(data), std::move(ctx));
            }),
            value_operand(operands_[0], args, name_, codename_,
                add_mode(ctx, eval_mode(eval_dont_evaluate_lambdas |
                    eval_dont_evaluate_partials))),
            value_operand(operands_[1], args, name_, codename_, ctx),
            value_operand(operands_[2], args, name_, codename_, ctx));
    }
}}}
