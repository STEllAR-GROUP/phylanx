// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/controls/fold_right_operation.hpp>
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
#include <iterator>
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
    match_pattern_type const fold_right_operation::match_data =
    {
        hpx::util::make_tuple("fold_right",
            std::vector<std::string>{"fold_right(_1_func, _2_initial, _3_data)"},
            &create_fold_right_operation,
            &create_primitive<fold_right_operation>,
            R"(func, initial, data

            Args:

                func : a function that takes two arbitrary arguments
                       and returns the result of folding the two arguments
                initial : an initial value
                data : a list or an array-like object

            Returns:

                The result of right-folding the elements of the data object
                using the given function.

                This function is equivalent to the Python code:

              def fr(f, i, r):
                  c = i
                  for n in r:
                      c = f(n, c)
                  return c

            Example(s):

              @Phylanx
              def foo():
                  v = fold_right(lambda a, b : 2 * a - b, 3, [1, 2, 3])
                  print(v)
              foo()

            Result:
              1)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    fold_right_operation::fold_right_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type fold_right_operation::fold_right_list(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        primitive_argument_type&& data, eval_context ctx) const
    {
        ir::range&& list = extract_list_value_strict(data, name_, codename_);

        // sequentially evaluate all operations
        std::size_t index = 0;
        for (auto it = list.rbegin(); it != list.rend() ; ++it)
        {
            // the initial value is allowed to be nil in which case fold_right
            // will use the last element of the list as its initial value
            if (0 == index++ && !valid(initial))
            {
                initial = std::move(*it);
                continue;
            }

            primitive_arguments_type args(2);
            args[0] = value_operand_sync(std::move(*it),
                noargs, name_, codename_, ctx);

            args[1] = value_operand_sync(std::move(initial),
                noargs, name_, codename_, ctx);

            initial = value_operand_sync(bound_func, std::move(args),
                name_, codename_, ctx);
        }

        return primitive_argument_type{value_operand_sync(
            std::move(initial), noargs, name_, codename_, ctx)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type fold_right_operation::fold_right_array_helper_1d(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        using vector_type = typename ir::node_data<T>::custom_storage1d_type;
        using iterator = std::reverse_iterator<typename vector_type::Iterator>;

        auto v = data.vector();

        auto begin = iterator(v.end());
        auto end = iterator(v.begin());

        for (auto it = begin; it != end; ++it)
        {
            // the initial value is allowed to be nil in which case fold_right
            // will use the first element of the list as its initial value
            if (it == begin)
            {
                if (!valid(initial))
                {
                    initial = primitive_argument_type{*it};
                    continue;
                }
            }

            primitive_arguments_type args(2);
            args[0] = extract_node_data<T>(std::move(initial), name_, codename_);
            args[1] = primitive_argument_type{*it};

            initial = value_operand_sync(
                bound_func, std::move(args), name_, codename_, ctx);
        }
        return primitive_argument_type{std::move(initial)};
    }

    template <typename T>
    primitive_argument_type fold_right_operation::fold_right_array_helper_2d(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        using vector_type = typename ir::node_data<T>::storage1d_type;
        using matrix_type = typename ir::node_data<T>::custom_storage2d_type;

        using iterator = util::matrix_row_iterator<matrix_type>;
        using reverse_iterator = std::reverse_iterator<iterator>;

        auto m = data.matrix();

        auto begin = reverse_iterator(iterator(m, m.rows()));
        auto end = reverse_iterator(iterator(m));
        for (auto it = begin; it != end; ++it)
        {
            // the initial value is allowed to be nil in which case fold_right
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
    primitive_argument_type fold_right_operation::fold_right_array_helper_3d(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        ir::node_data<T>&& data, eval_context ctx) const
    {
        using matrix_type = typename ir::node_data<T>::storage2d_type;
        using tensor_type = typename ir::node_data<T>::custom_storage3d_type;

        using iterator = util::tensor_pageslice_iterator<tensor_type>;
        using reverse_iterator = std::reverse_iterator<iterator>;

        auto t = data.tensor();

        auto begin = reverse_iterator(iterator(t, t.pages()));
        auto end = reverse_iterator(iterator(t));
        for (auto it = begin; it != end; ++it)
        {
            // the initial value is allowed to be nil in which case fold_right
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
    primitive_argument_type fold_right_operation::fold_right_array_helper(
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
                        "fold_right_operation::fold_right_array_helper",
                    generate_error_message(
                        "the fold_right primitive requires for its data "
                        "argument to be a numeric array data type "
                        "(non-zero dimensional)"));
            }
            break;

        case 1:
            initial = fold_right_array_helper_1d(std::move(bound_func),
                std::move(initial), extract_node_data<T>(std::move(data)),
                std::move(ctx));
            break;

        case 2:
            initial = fold_right_array_helper_2d(std::move(bound_func),
                std::move(initial), extract_node_data<T>(std::move(data)),
                std::move(ctx));
            break;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            initial = fold_right_array_helper_3d(std::move(bound_func),
                std::move(initial), extract_node_data<T>(std::move(data)),
                std::move(ctx));
            break;
#endif
        }

        return primitive_argument_type{std::move(initial)};
    }

    primitive_argument_type fold_right_operation::fold_right_array(
        primitive_argument_type&& bound_func, primitive_argument_type&& initial,
        primitive_argument_type&& data, eval_context ctx) const
    {
        node_data_type dtype = extract_common_type(data, initial);
        switch (dtype)
        {
        case node_data_type_bool:
            return fold_right_array_helper(std::move(bound_func),
                std::move(initial),
                extract_node_data<std::uint8_t>(std::move(data)),
                std::move(ctx));

        case node_data_type_int64:
            return fold_right_array_helper(std::move(bound_func),
                std::move(initial),
                extract_node_data<std::int64_t>(std::move(data)),
                std::move(ctx));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return fold_right_array_helper(std::move(bound_func),
                std::move(initial), extract_node_data<double>(std::move(data)),
                std::move(ctx));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "fold_right_operation::fold_right_array",
            generate_error_message(
                "the fold_right primitive requires for its data argument to "
                "be a numeric data type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> fold_right_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type&& args, eval_context ctx) const
    {
        if (operands.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_right_operation::eval",
                util::generate_error_message(
                    "the fold_right_operation primitive requires exactly "
                        "three operands",
                    name_, codename_));
        }

        // note: the initial value is allowed to be nil
        if (!valid(operands[0]) || !valid(operands_[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_right_operation::eval",
                util::generate_error_message(
                    "the fold_right_operation primitive requires that the "
                        "arguments given by the operands array "
                        "are valid",
                    name_, codename_));
        }

        // the first argument must be an invokable
        if (util::get_if<primitive>(&operands[0]) == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "fold_right_operation::eval",
                util::generate_error_message(
                    "the first argument to map must be an invocable "
                    "object", name_, codename_));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_,
            add_mode(ctx,
                eval_mode(
                    eval_dont_evaluate_lambdas | eval_dont_evaluate_partials)));
        auto&& op1 = value_operand(operands[1], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_), ctx](
                    hpx::future<primitive_argument_type>&& b,
                    hpx::future<primitive_argument_type>&& i,
                    hpx::future<primitive_argument_type>&& d)
            -> primitive_argument_type
            {
                auto&& bound_func = b.get();
                auto&& initial = i.get();
                auto&& data = d.get();

                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "fold_right_operation::eval",
                        util::generate_error_message(
                            "the first argument to filter must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                // handle list separately from arrays
                if (is_list_operand_strict(data))
                {
                    return this_->fold_right_list(std::move(bound_func),
                        std::move(initial), std::move(data), std::move(ctx));
                }

                if (!is_numeric_operand(data))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "fold_right_operation::eval",
                        util::generate_error_message(
                            "the first argument to filter must be an invocable "
                            "object", this_->name_, this_->codename_));
                }

                return this_->fold_right_array(std::move(bound_func),
                    std::move(initial), std::move(data), std::move(ctx));
            },
            std::move(op0), std::move(op1),
            value_operand(operands[2], std::move(args), name_, codename_,
                std::move(ctx)));
    }
}}}
