// Copyright (c) 2021 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/plugins/listops/delete_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const delete_operation::match_data = {
        hpx::make_tuple("delete", std::vector<std::string>{"delete(_1, _2)"},
            &create_delete_operation, &create_primitive<delete_operation>,
            R"(target, index
            Args:

                target (list or dict) : the container to delete a value from
                index (key or list) : the index (indices) of the element(s) to
                    remove from the dictionary or list

            Returns:

            A new list or dictionary with the element(s) removed. Note that
            `target` is not modified by this operation.)")};

    ///////////////////////////////////////////////////////////////////////////
    delete_operation::delete_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type delete_operation::delete_from_dict(
        ir::dictionary&& target, primitive_argument_type&& index,
        eval_context ctx) const
    {
        if (!target.has_key(index))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "delete_operation::delete_from_dict",
                generate_error_message(
                    "attempting to delete a non-existing dictionary entry",
                    ctx));
        }

        target.dict().erase(index);
        return primitive_argument_type{std::move(target)};
    }

    primitive_argument_type delete_operation::delete_from_list(
        ir::range&& target, primitive_argument_type&& index,
        eval_context ctx) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "delete_operation::delete_from_list",
            generate_error_message(
                "deleting an entry from list is not supported yet", ctx));
    }

    primitive_argument_type delete_operation::delete_elements(
        primitive_argument_type&& target, primitive_argument_type&& index,
        eval_context ctx) const
    {
        if (is_dictionary_operand_strict(target))
        {
            return delete_from_dict(extract_dictionary_value_strict(
                                        std::move(target), name_, codename_),
                std::move(index), std::move(ctx));
        }

        if (is_list_operand_strict(target))
        {
            return delete_from_list(
                extract_list_value_strict(std::move(target), name_, codename_),
                std::move(index), std::move(ctx));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "delete_operation::delete_elements",
            generate_error_message("the delete_operation supports removing "
                                   "elements from lists and dictionaries only",
                ctx));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> delete_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "delete_operation::eval",
                generate_error_message(
                    "the delete_operation primitive requires two arguments",
                    ctx));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "delete_operation::eval",
                generate_error_message(
                    "the delete_operation primitive requires that the "
                    "argument given by the operand are valid",
                    ctx));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(
            hpx::launch::sync,
            [this_ = std::move(this_), ctx = std::move(ctx)](
                hpx::future<primitive_argument_type>&& arg0,
                hpx::future<primitive_argument_type>&& arg1) mutable
            -> primitive_argument_type {
                return this_->delete_elements(
                    arg0.get(), arg1.get(), std::move(ctx));
            },
            value_operand(operands[0], args, name_, codename_, ctx),
            value_operand(operands[1], args, name_, codename_, ctx));
    }
}}}    // namespace phylanx::execution_tree::primitives
