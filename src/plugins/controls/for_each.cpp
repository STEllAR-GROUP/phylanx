// Copyright (c) 2018-2021 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/controls/for_each.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const for_each::match_data = {hpx::make_tuple("for_each",
        std::vector<std::string>{"for_each(_1, _2)"}, &create_for_each,
        &create_primitive<for_each>,
        R"(func, range
            The for_each primitive calls a function `func` for
            each item in the iterator.
            Args:

                func (function): a function that takes one argument
                range (iter): an iterator

            Returns:

              The value returned from the last iteration, `nil` otherwise.

            Examples:

                @Phylanx
                def foo():
                    for_each(lambda a : print(a), [1, 2])
                foo()

            Prints 1 and 2 on individual lines.)")};

    ///////////////////////////////////////////////////////////////////////////
    for_each::for_each(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    void for_each::iterate_over_array_scalar(primitive const* p,
        primitive_argument_type&& value, eval_context ctx) const
    {
        p->eval(hpx::launch::sync, std::move(value), ctx);
    }

    ////////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <typename T>
        void iterate_over_array_vector_helper(primitive const* p,
            ir::node_data<T>&& value, eval_context ctx, std::string const& name,
            std::string const& codename)
        {
            for (auto&& e : value.vector())
            {
                auto result =
                    p->eval(hpx::launch::sync, primitive_argument_type{e}, ctx);

                if (is_boolean_operand_strict(result))
                {
                    if (extract_boolean_value(
                            std::move(result), name, codename))
                    {
                        break;    // stop, if requested
                    }
                }
            }
        }
    }    // namespace detail

    void for_each::iterate_over_array_vector(primitive const* p,
        primitive_argument_type&& value, eval_context ctx) const
    {
        switch (extract_common_type(value))
        {
        case node_data_type_bool:
            return detail::iterate_over_array_vector_helper(p,
                extract_boolean_value_strict(
                    std::move(value), name_, codename_),
                std::move(ctx), name_, codename_);

        case node_data_type_int64:
            return detail::iterate_over_array_vector_helper(p,
                extract_integer_value_strict(
                    std::move(value), name_, codename_),
                std::move(ctx), name_, codename_);

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return detail::iterate_over_array_vector_helper(p,
                extract_numeric_value_strict(
                    std::move(value), name_, codename_),
                std::move(ctx), name_, codename_);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "for_each::iterate_over_array_vector",
            generate_error_message(
                "the given array type has an unsupported type", ctx));
    }

    ////////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <typename T>
        void iterate_over_array_matrix_helper(primitive const* p,
            ir::node_data<T>&& value, eval_context ctx, std::string const& name,
            std::string const& codename)
        {
            auto m = value.matrix();

            using phylanx::util::matrix_iterator;
            matrix_iterator<decltype(m)> begin(m, 0);
            matrix_iterator<decltype(m)> end(m, m.rows());

            for (auto it = begin; it != end; ++it)
            {
                blaze::DynamicVector<T> row(std::move(*it));
                auto result = p->eval(hpx::launch::sync,
                    primitive_argument_type{std::move(row)}, ctx);

                if (is_boolean_operand_strict(result))
                {
                    if (extract_boolean_value(
                            std::move(result), name, codename))
                    {
                        break;    // stop, if requested
                    }
                }
            }
        }
    }    // namespace detail

    void for_each::iterate_over_array_matrix(primitive const* p,
        primitive_argument_type&& value, eval_context ctx) const
    {
        switch (extract_common_type(value))
        {
        case node_data_type_bool:
            return detail::iterate_over_array_matrix_helper(p,
                extract_boolean_value_strict(
                    std::move(value), name_, codename_),
                std::move(ctx), name_, codename_);

        case node_data_type_int64:
            return detail::iterate_over_array_matrix_helper(p,
                extract_integer_value_strict(
                    std::move(value), name_, codename_),
                std::move(ctx), name_, codename_);

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return detail::iterate_over_array_matrix_helper(p,
                extract_numeric_value_strict(
                    std::move(value), name_, codename_),
                std::move(ctx), name_, codename_);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "for_each::iterate_over_array_matrix",
            generate_error_message(
                "the given array type has an unsupported type", ctx));
    }

    ///////////////////////////////////////////////////////////////////////////
    void for_each::iterate_over_array(primitive const* p,
        primitive_argument_type&& value, eval_context ctx) const
    {
        std::size_t dim =
            extract_numeric_value_dimension(value, name_, codename_);

        switch (dim)
        {
        case 0:
            iterate_over_array_scalar(p, std::move(value), std::move(ctx));
            return;

        case 1:
            iterate_over_array_vector(p, std::move(value), std::move(ctx));
            return;

        case 2:
            iterate_over_array_matrix(p, std::move(value), std::move(ctx));
            return;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "for_each::iterate_over_array",
            generate_error_message("the iteration space has an unsupported "
                                   "numeric type (dimension)",
                ctx));
    }

    hpx::future<primitive_argument_type> for_each::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "for_each::eval",
                generate_error_message("the for_each primitive requires "
                                       "exactly two operands",
                    ctx));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "for_each::eval",
                generate_error_message(
                    "the for_each primitive requires that the arguments given "
                    "by the operands array are valid",
                    ctx));
        }

        // the first argument must be an invokable
        primitive const* p = util::get_if<primitive>(&operands_[0]);
        if (p == nullptr)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "for_each::eval",
                generate_error_message("the first argument to for_each must be "
                                       "an invocable object",
                    ctx));
        }

        ctx.remove_mode(eval_dont_wrap_functions);

        auto op0 = value_operand(operands_[0], args, name_, codename_,
            add_mode(ctx, eval_dont_evaluate_lambdas));
        auto op1 = value_operand(operands_[1], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                hpx::future<primitive_argument_type>&& f,
                hpx::future<primitive_argument_type>&& fval) mutable
            -> primitive_argument_type {
                auto&& bound_func = f.get();

                primitive const* p = util::get_if<primitive>(&bound_func);
                if (p == nullptr)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "for_each::eval",
                        this_->generate_error_message(
                            "the first argument to for_each must "
                            "resolve to an invocable object",
                            ctx));
                }

                // evaluate function for each of the elements from the given
                // range
                auto&& value = fval.get();

                if (is_list_operand_strict(value))
                {
                    auto&& list = extract_list_value_strict(
                        std::move(value), this_->name_, this_->codename_);
                    for (auto&& e : std::move(list))
                    {
                        auto r = p->eval(hpx::launch::sync, std::move(e), ctx);
                        if (is_boolean_operand_strict(r))
                        {
                            if (extract_boolean_value(std::move(r),
                                    this_->name_, this_->codename_))
                            {
                                break;    // stop, if requested
                            }
                        }
                    }
                }
                else if (is_dictionary_operand_strict(value))
                {
                    auto&& dict = extract_dictionary_value_strict(
                        std::move(value), this_->name_, this_->codename_);
                    for (auto&& e : std::move(dict).dict())
                    {
                        auto result = p->eval(hpx::launch::sync,
                            primitive_argument_type{std::move(e.first.get())},
                            ctx);

                        if (is_boolean_operand_strict(result))
                        {
                            if (extract_boolean_value(std::move(result),
                                    this_->name_, this_->codename_))
                            {
                                break;    // stop, if requested
                            }
                        }
                    }
                }
                else if (is_numeric_operand_strict(value) ||
                    is_boolean_operand_strict(value) ||
                    is_integer_operand_strict(value))
                {
                    this_->iterate_over_array(
                        p, std::move(value), std::move(ctx));
                }
                else
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "for_each::eval",
                        this_->generate_error_message(
                            "unsupported iteration space for for_each", ctx));
                }

                return primitive_argument_type{};
            },
            std::move(op0), std::move(op1));
    }
}}}    // namespace phylanx::execution_tree::primitives
