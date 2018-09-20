// Copyright (c) 2018 Weile Wei
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/plugins/listops/dict_keys_operation.hpp>
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
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const dict_keys_operation::match_data = {
        hpx::util::make_tuple("dict_keys",
            std::vector<std::string>{"dict_keys(_1)"},
            &create_dict_keys_operation, &create_primitive<dict_keys_operation>,
            "lili\n"
            "Args:\n"
            "\n"
            "    lili (list of lists, optional) : a list of 2-element lists\n"
            "\n"
            "Returns:\n"
            "\n"
            "The dict_keys primitive returns a list of dictionary key objects "
            "constructed "
            "from dictionary.")};

    ///////////////////////////////////////////////////////////////////////////
    dict_keys_operation::dict_keys_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dict_keys_operation::generate_dict_keys(
        primitive_arguments_type&& args) const
    {
        ir::dictionary args_dict =
            extract_dictionary_value(std::move(args[0]), name_, codename_);

        if (args_dict.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dict_keys_operation::eval",
                util::generate_error_message(
                    "the dict_keys primitive accepts exactly one argument",
                    name_, codename_));
        }

        ir::range dict_list(args_dict.begin(), args_dict.end());
        for (ir::range_iterator it = dict_list.begin(); it != dict_list.end();
             ++it)
        {
            std::cout << *it << std::endl;
        }

        return primitive_argument_type{std::move(dict_list)};
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dict_keys_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() > 1 || operands.empty())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dict_keys_operation::eval",
                util::generate_error_message(
                    "the dict_keys primitive accepts exactly one argument",
                    name_, codename_));
        }

        if (!operands.empty() && !valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "dict_keys_operation::eval",
                util::generate_error_message(
                    "the dict_keys primitive requires that the "
                    "argument given by the operand is a valid dictionary "
                    "object",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_](primitive_arguments_type&& args)
                                      -> primitive_argument_type {
                return this_->generate_dict_keys(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }

    hpx::future<primitive_argument_type> dict_keys_operation::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
