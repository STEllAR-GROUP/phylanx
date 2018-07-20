// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/ir/slice_node_data.hpp>

#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given primitive_argument_type instance
    primitive_argument_type slice(primitive_argument_type const& data,
        ir::range const& indices)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{
                ir::slice(extract_integer_value_strict(data), indices)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{
                ir::slice(extract_numeric_value_strict(data), indices)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{
                ir::slice(extract_boolean_value_strict(data), indices)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            "target object does not hold a numeric data type and as such does "
            "not support slicing");
    }

    primitive_argument_type slice(
        primitive_argument_type const& data,
        ir::range const& rows, ir::range const& columns)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_integer_value_strict(data), rows, columns)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_numeric_value_strict(data), rows, columns)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_boolean_value_strict(data), rows, columns)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            "target object does not hold a numeric data type and as such does "
            "not support slicing");
    }

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given primitive_argument_type instance
    primitive_argument_type slice(primitive_argument_type && data,
        ir::range const& indices, primitive_argument_type && value)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{
                ir::slice(extract_integer_value_strict(std::move(data)),
                    indices, extract_integer_value(std::move(value)))};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{
                ir::slice(extract_numeric_value_strict(std::move(data)),
                    indices, extract_numeric_value(std::move(value)))};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{ir::slice(
                extract_boolean_value_strict(std::move(data)), indices,
                    extract_boolean_value(std::move(value)))};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            "target object does not hold a numeric data type and as such does "
            "not support slicing");
    }

    primitive_argument_type slice(primitive_argument_type&& data,
        ir::range const& rows, ir::range const& columns,
        primitive_argument_type&& value)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_integer_value_strict(std::move(data)), rows,
                    columns, extract_integer_value(std::move(value)))};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_numeric_value_strict(std::move(data)), rows,
                    columns, extract_numeric_value(std::move(value)))};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{
                slice(extract_boolean_value_strict(std::move(data)), rows,
                    columns, extract_boolean_value(std::move(value)))};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            "target object does not hold a numeric data type and as such does "
            "not support slicing");
    }
}}


