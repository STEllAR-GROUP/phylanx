// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data.hpp>
#include <phylanx/execution_tree/primitives/slice_range.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given primitive_argument_type instance
    primitive_argument_type slice(primitive_argument_type const& data,
        primitive_argument_type const& indices, std::string const& name,
        std::string const& codename)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{slice(
                extract_integer_value_strict(data), indices, name, codename)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{slice(
                extract_numeric_value_strict(data), indices, name, codename)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{slice(
                extract_boolean_value_strict(data), indices, name, codename)};
        }
        if (is_list_operand_strict(data))
        {
            return slice(
                extract_list_value_strict(data), indices, name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric or range data type and "
                "as such does not support slicing", name, codename));
    }

    primitive_argument_type slice(primitive_argument_type const& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, std::string const& name,
        std::string const& codename)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_integer_value_strict(data), rows, columns,
                    name, codename)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_numeric_value_strict(data), rows, columns,
                    name, codename)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_boolean_value_strict(data), rows, columns,
                    name, codename)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing", name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given primitive_argument_type instance
    primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& indices, primitive_argument_type&& value,
        std::string const& name, std::string const& codename)
    {
        if (is_integer_operand_strict(value))
        {
            return primitive_argument_type{slice(
                extract_integer_value(std::move(data)), indices,
                extract_integer_value_strict(std::move(value)), name, codename)};
        }
        if (is_numeric_operand_strict(value))
        {
            return primitive_argument_type{slice(
                extract_numeric_value(std::move(data)), indices,
                extract_numeric_value_strict(std::move(value)), name, codename)};
        }
        if (is_boolean_operand_strict(value))
        {
            return primitive_argument_type{slice(
                extract_boolean_value(std::move(data)), indices,
                extract_boolean_value_strict(std::move(value)), name, codename)};
        }
        if (is_list_operand_strict(value))
        {
            return slice(extract_list_value(std::move(data)), indices,
                extract_list_value_strict(std::move(value)), name, codename);
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric or range data type and "
                "as such does not support slicing", name, codename));
    }

    primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, primitive_argument_type&& value,
        std::string const& name, std::string const& codename)
    {
        if (is_integer_operand_strict(value))
        {
            return primitive_argument_type{
                slice(extract_integer_value(std::move(data)), rows, columns,
                    extract_integer_value_strict(std::move(value)), name,
                    codename)};
        }
        if (is_numeric_operand_strict(value))
        {
            return primitive_argument_type{
                slice(extract_numeric_value(std::move(data)), rows, columns,
                    extract_numeric_value_strict(std::move(value)), name,
                    codename)};
        }
        if (is_boolean_operand_strict(value))
        {
            return primitive_argument_type{
                slice(extract_boolean_value(std::move(data)), rows, columns,
                    extract_boolean_value_strict(std::move(value)), name,
                    codename)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing", name, codename));
    }
}}


