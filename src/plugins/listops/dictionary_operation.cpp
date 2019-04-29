// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/plugins/listops/dictionary_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const dict_operation::match_data =
    {
        hpx::util::make_tuple("dict",
            std::vector<std::string>{"dict(__1)"},
            &create_dict_operation, &create_primitive<dict_operation>,
            R"(lili
            Args:

                lili (list of lists, optional) : a list of 2-element lists

            Returns:

            The dict primitive returns a dictionary object constructed
            from a list of 2-element lists. The 2-element lists provide
            a key in the first element and a value in the second.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    dict_operation::dict_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dict_operation::generate_dict(
        std::vector<ir::range, arguments_allocator<ir::range>>&& args) const
    {
        ir::range args_list =
            extract_list_value_strict(std::move(args[0]), name_, codename_);

        if (args_list.empty())
        {
            return primitive_argument_type{ir::dictionary{}};
        }

        phylanx::ir::dictionary dict;
        dict.reserve(args_list.size());
        for (auto it = args_list.begin(); it != args_list.end(); ++it)
        {
            if (!is_list_operand_strict(*it))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dictionary_operation::generate_dict",
                    generate_error_message(
                        "dict_operation expects a list of lists with exactly "
                        "two elements each"));
            }

            auto element = extract_list_value_strict(std::move(*it));
            auto && p = element.args();

            if (p.size() != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dictionary_operation::generate_dict",
                    generate_error_message(
                        "dict_operation needs exactly two values for each of "
                        "the key/value pairs"));
            }

            dict[std::move(p[0])] = std::move(p[1]);
        }
        return primitive_argument_type(std::move(dict));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dict_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dict_operation::eval",
                generate_error_message(
                    "the dictionary_operation primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        if (operands.empty() ||
            (operands.size() == 1 && is_implicit_nil(operands[0])))
        {
            return hpx::make_ready_future(
                primitive_argument_type{ir::dictionary{}});
        }

        if (!valid(operands[0]) && is_explicit_nil(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dict_operation::eval",
                generate_error_message(
                    "the dictionary_operation primitive requires that the "
                    "argument given by the operand is a valid list"));
        }

        using range_list =
            std::vector<ir::range, arguments_allocator<ir::range>>;

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                std::vector<ir::range, arguments_allocator<ir::range>>&& args)
            -> primitive_argument_type
            {
                return this_->generate_dict(std::move(args));
            }),
            detail::map_operands(operands, functional::list_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
