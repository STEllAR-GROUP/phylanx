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
            &create_len_operation, &create_primitive<len_operation>,
            "li\n"
            "Args:\n"
            "\n"
            "    li (object) : a list, vector, or matrix\n"
            "\n"
            "Returns:\n"
            "\n"
            "The size of the given object."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    len_operation::len_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> len_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::len_operation::eval",
                generate_error_message(
                    "len_operation accepts exactly one argument"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& arg)
            ->  primitive_argument_type
        {
            if (is_list_operand_strict(arg))
            {
                auto&& val = extract_list_value_strict(std::move(arg));
                return primitive_argument_type{ir::node_data<std::int64_t>{
                    static_cast<std::int64_t>(val.size())}};
            }
            else if (is_string_operand(arg))
            {
                auto val = extract_string_value(std::move(arg));
                return primitive_argument_type{ir::node_data<std::int64_t>{
                    static_cast<std::int64_t>(val.size())}};
            }
            else if (is_boolean_operand_strict(arg) ||
                is_integer_operand_strict(arg) ||
                is_numeric_operand_strict(arg))
            {
                std::size_t dim = extract_numeric_value_dimension(arg);
                auto val = extract_numeric_value_dimensions(std::move(arg));
                switch (dim)
                {
                case 0:
                    return primitive_argument_type{ir::node_data<std::int64_t>{
                        static_cast<std::int64_t>(1)}};

                case 1:     // for vectors, return number of elements
                    return primitive_argument_type{ir::node_data<std::int64_t>{
                        static_cast<std::int64_t>(val[0])}};

                case 2:     // for matrices, return number of rows
                    return primitive_argument_type{ir::node_data<std::int64_t>{
                        static_cast<std::int64_t>(val[0])}};

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:     // for tensors, return number of pages
                    return primitive_argument_type{ir::node_data<std::int64_t>{
                        static_cast<std::int64_t>(val[0])}};
#endif
                default:
                    break;
                }
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::len_operation::eval",
                this_->generate_error_message(
                    "len_operation accepts a list, a string, or a numeric "
                    "value as its operand only"));
        }),
            value_operand(operands[0], args,
                name_, codename_, std::move(ctx)));
    }
}}}
