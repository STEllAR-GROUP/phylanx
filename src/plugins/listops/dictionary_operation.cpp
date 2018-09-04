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
    match_pattern_type const dict_operation::match_data = {
        hpx::util::make_tuple("dict", std::vector<std::string>{"dict(__1)"},
            &create_dict_operation, &create_primitive<dict_operation>)};

    ///////////////////////////////////////////////////////////////////////////
    dict_operation::dict_operation(
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type dict_operation::generate_dict(
        std::vector<phylanx::ir::range>&& args) const
    {
        if (args.size() == 0)
        {
            phylanx::ir::dictionary dict;
            return primitive_argument_type(std::move(dict));
        }

        ir::range args_list =
            extract_list_value_strict(std::move(args[0]), name_, codename_);

        if (args_list.size() == 0)
        {
            phylanx::ir::dictionary dict;
            return primitive_argument_type(std::move(dict));
        }

        phylanx::ir::dictionary dict;
        for (auto it = args_list.begin(); it != args_list.end(); ++it)
        {
            auto const& key_value = phylanx::execution_tree::extract_list_value(*it);

            if (key_value.args().size() != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dictionary_operation::generate_dict",
                    util::generate_error_message("dict_operation needs exactly "
                                                 "two parameters for key-value",
                        name_, codename_));
            }
            dict[key_value.args()[0]] = key_value.args()[1];
        }
        return primitive_argument_type(std::move(dict));
    }

    //////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> dict_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "dict_operation::eval",
                    util::generate_error_message(
                        "the dictionary_operation primitive requires that the "
                        "arguments given by the operands array are valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_](
                    std::vector<ir::range>&& args) -> primitive_argument_type {
                    return this_->generate_dict(std::move(args));
                }),
            detail::map_operands(
                operands, functional::list_operand{}, args, name_, codename_));
    }

    hpx::future<primitive_argument_type> dict_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
