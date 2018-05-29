// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/listops/len_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const len_operation::match_data =
    {
        hpx::util::make_tuple("len",
            std::vector<std::string>{"len(_1)"},
            &create_len_operation, &create_primitive<len_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    len_operation::len_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> len_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::len_operation::eval",
                execution_tree::generate_error_message(
                    "len_operation accepts exactly one argument", name_,
                    codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](primitive_argument_type&& arg)
            ->  primitive_argument_type
        {
            if (is_list_operand_strict(arg))
            {
                auto val = extract_list_value_strict(std::move(arg));
                return primitive_argument_type{ir::node_data<std::int64_t>{
                    static_cast<std::int64_t>(val.size())}};
            }
            else if (is_string_operand(arg))
            {
                auto val = extract_string_value(std::move(arg));
                return primitive_argument_type{ir::node_data<std::int64_t>{
                    static_cast<std::int64_t>(val.size())}};
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::len_operation::eval",
                execution_tree::generate_error_message(
                    "len_operation accepts a list or a string as operand",
                    this_->name_, this_->codename_));
        }),
            value_operand(operands[0], args,
            name_, codename_));
    }

    hpx::future<primitive_argument_type> len_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
