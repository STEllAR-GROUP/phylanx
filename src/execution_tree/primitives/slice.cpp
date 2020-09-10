// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/slice.hpp>
#include <phylanx/execution_tree/primitives/slice_node_data.hpp>
#include <phylanx/execution_tree/primitives/slice_range.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/assert.hpp>

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
        std::string const& codename, eval_context const& ctx)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_integer_value_strict(data, name, codename), indices,
                name, codename, ctx)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_numeric_value_strict(data, name, codename), indices,
                name, codename, ctx)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_boolean_value_strict(data, name, codename), indices,
                name, codename, ctx)};
        }
        if (is_list_operand_strict(data))
        {
            return slice_list(extract_list_value_strict(data, name, codename),
                indices, name, codename, ctx);
        }
        if (is_dictionary_operand_strict(data))
        {
            auto&& dict =
                phylanx::execution_tree::extract_dictionary_value_strict(
                    data, name, codename);
            if (!is_hashable_operand(indices))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::slice",
                    util::generate_error_message(
                        "the given index cannot be used for indexing a "
                        "dictionary",
                        name, codename, ctx.back_trace()));
            }
            return dict[indices];
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric, range, or dictionary "
                "type and as such does not support slicing",
                name, codename, ctx.back_trace()));
    }

    primitive_argument_type dist_slice(primitive_argument_type const& data,
        primitive_argument_type const& indices, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        localities_information arr_localities =
            extract_localities_information(data, name, codename);
        if (is_integer_operand_strict(data))
        {
            return dist_slice_extract(
                extract_integer_value_strict(data, name, codename), indices,
                std::move(arr_localities), name, codename, ctx);
        }
        if (is_numeric_operand_strict(data))
        {
            return dist_slice_extract(
                extract_numeric_value_strict(data, name, codename), indices,
                std::move(arr_localities), name, codename, ctx);
        }
        if (is_boolean_operand_strict(data))
        {
            return dist_slice_extract(
                extract_boolean_value_strict(data, name, codename), indices,
                std::move(arr_localities), name, codename, ctx);
        }
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "distributed target object does not hold a numeric type and as "
                "such does not support slicing",
                name, codename, ctx.back_trace()));
    }

    primitive_argument_type slice(primitive_argument_type const& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_integer_value_strict(data, name, codename),
                rows, columns, name, codename, ctx)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_numeric_value_strict(data, name, codename),
                rows, columns, name, codename, ctx)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_boolean_value_strict(data, name, codename),
                rows, columns, name, codename, ctx)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing",
                name, codename, ctx.back_trace()));
    }

    primitive_argument_type dist_slice(primitive_argument_type const& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        localities_information arr_localities =
            extract_localities_information(data, name, codename);
        if (is_integer_operand_strict(data))
        {
            return dist_slice_extract(
                extract_integer_value_strict(data, name, codename), rows,
                columns, std::move(arr_localities), name, codename, ctx);
        }
        if (is_numeric_operand_strict(data))
        {
            return dist_slice_extract(
                extract_numeric_value_strict(data, name, codename), rows,
                columns, std::move(arr_localities), name, codename, ctx);
        }
        if (is_boolean_operand_strict(data))
        {
            return dist_slice_extract(
                extract_boolean_value_strict(data, name, codename), rows,
                columns, std::move(arr_localities), name, codename, ctx);
        }
        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing",
                name, codename, ctx.back_trace()));

    }

    primitive_argument_type slice(primitive_argument_type const& data,
        primitive_argument_type const& pages,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, std::string const& name,
        std::string const& codename, eval_context const& ctx)
    {
        if (is_integer_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_integer_value_strict(data, name, codename),
                pages, rows, columns, name, codename, ctx)};
        }
        if (is_numeric_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_numeric_value_strict(data, name, codename),
                pages, rows, columns, name, codename, ctx)};
        }
        if (is_boolean_operand_strict(data))
        {
            return primitive_argument_type{slice_extract(
                extract_boolean_value_strict(data, name, codename),
                pages, rows, columns, name, codename, ctx)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given primitive_argument_type instance
    primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& indices, primitive_argument_type&& value,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {

        if (data.has_annotation() && value.has_annotation())
        {
            localities_information arr_localities =
                extract_localities_information(data, name, codename);

            localities_information val_localities =
                extract_localities_information(value, name, codename);

            if (is_integer_operand_strict(data))
            {
                if (is_integer_operand_strict(value))
                {
                    return dist_slice_assign(extract_integer_value_strict(
                        std::move(data), name, codename),
                        indices,
                        extract_integer_value_strict(
                            std::move(value), name, codename),
                        std::move(arr_localities), std::move(val_localities),
                        name, codename, ctx);
                }

                return dist_slice_assign(extract_integer_value_strict(
                    std::move(data), name, codename),
                    indices,
                    extract_integer_value(std::move(value), name, codename),
                    std::move(arr_localities), std::move(val_localities), name,
                    codename, ctx);
            }
            else if (is_numeric_operand_strict(data))
            {
                if (is_numeric_operand_strict(value))
                {
                    return dist_slice_assign(extract_numeric_value_strict(
                        std::move(data), name, codename),
                        indices,
                        extract_numeric_value_strict(
                            std::move(value), name, codename),
                        std::move(arr_localities), std::move(val_localities),
                        name, codename, ctx);
                }

                return dist_slice_assign(extract_numeric_value_strict(
                    std::move(data), name, codename),
                    indices,
                    extract_numeric_value(std::move(value), name, codename),
                    std::move(arr_localities), std::move(val_localities), name,
                    codename, ctx);
            }
            else if (is_boolean_operand_strict(data))
            {
                if (is_boolean_operand_strict(value))
                {
                    return dist_slice_assign(extract_boolean_value_strict(
                        std::move(data), name, codename),
                        indices,
                        extract_boolean_value_strict(
                            std::move(value), name, codename),
                        std::move(arr_localities), std::move(val_localities),
                        name, codename, ctx);
                }

                return dist_slice_assign(extract_boolean_value_strict(
                    std::move(data), name, codename),
                    indices,
                    extract_boolean_value(std::move(value), name, codename),
                    std::move(arr_localities), std::move(val_localities), name,
                    codename, ctx);
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::execution_tree::slice",
                util::generate_error_message(
                    "distributed target object does not hold a numeric type "
                    "and as such does not support slicing",
                    name, codename, ctx.back_trace()));
        }

        if (data.has_annotation())
        {
            localities_information arr_localities =
                extract_localities_information(data, name, codename);

            if (is_integer_operand_strict(data))
            {
                if (is_integer_operand_strict(value))
                {
                    return dist_slice_assign(extract_integer_value_strict(
                        std::move(data), name, codename),
                        indices,
                        extract_integer_value_strict(
                            std::move(value), name, codename),
                        std::move(arr_localities), name, codename, ctx);
                }

                return dist_slice_assign(extract_integer_value_strict(
                    std::move(data), name, codename),
                    indices,
                    extract_integer_value(std::move(value), name, codename),
                    std::move(arr_localities), name, codename, ctx);
            }
            else if (is_numeric_operand_strict(data))
            {
                if (is_numeric_operand_strict(value))
                {
                    return dist_slice_assign(extract_numeric_value_strict(
                        std::move(data), name, codename),
                        indices,
                        extract_numeric_value_strict(
                            std::move(value), name, codename),
                        std::move(arr_localities), name, codename, ctx);
                }

                return dist_slice_assign(extract_numeric_value_strict(
                    std::move(data), name, codename),
                    indices,
                    extract_numeric_value(std::move(value), name, codename),
                    std::move(arr_localities), name, codename, ctx);
            }
            else if (is_boolean_operand_strict(data))
            {
                if (is_boolean_operand_strict(value))
                {
                    return dist_slice_assign(extract_boolean_value_strict(
                        std::move(data), name, codename),
                        indices,
                        extract_boolean_value_strict(
                            std::move(value), name, codename),
                        std::move(arr_localities), name, codename, ctx);
                }

                return dist_slice_assign(extract_boolean_value_strict(
                    std::move(data), name, codename),
                    indices,
                    extract_boolean_value(std::move(value), name, codename),
                    std::move(arr_localities), name, codename, ctx);
            }

            HPX_THROW_EXCEPTION(hpx::invalid_status,
                "phylanx::execution_tree::slice",
                util::generate_error_message(
                    "distributed target object does not hold a numeric type "
                    "and as such does not support slicing",
                    name, codename, ctx.back_trace()));
        }


        if (is_list_operand_strict(data))
        {
            return primitive_argument_type{slice_list(
                extract_list_value_strict(std::move(data), name, codename),
                indices, extract_value(std::move(value), name, codename), name,
                codename, ctx)};
        }
        else if (is_integer_operand_strict(data))
        {
            if (is_integer_operand_strict(value))
            {
                return primitive_argument_type{slice_assign(
                    extract_integer_value_strict(
                        std::move(data), name, codename),
                    indices,
                    extract_integer_value_strict(
                        std::move(value), name, codename),
                    name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_integer_value_strict(std::move(data), name, codename),
                indices,
                extract_integer_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        else if (is_numeric_operand_strict(data))
        {
            if (is_numeric_operand_strict(value))
            {
                return primitive_argument_type{slice_assign(
                    extract_numeric_value_strict(
                        std::move(data), name, codename),
                    indices,
                    extract_numeric_value_strict(
                        std::move(value), name, codename),
                    name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_numeric_value_strict(std::move(data), name, codename),
                indices,
                extract_numeric_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        else if (is_boolean_operand_strict(data))
        {
            if (is_boolean_operand_strict(value))
            {
                return primitive_argument_type{slice_assign(
                    extract_boolean_value_strict(
                        std::move(data), name, codename),
                    indices,
                    extract_boolean_value_strict(
                        std::move(value), name, codename),
                    name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_boolean_value_strict(std::move(data), name, codename),
                indices,
                extract_boolean_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        else if (is_dictionary_operand_strict(data))
        {
            auto&& dict =
                phylanx::execution_tree::extract_dictionary_value_strict(
                    std::move(data));
            if (!is_hashable_operand(indices))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::slice",
                    util::generate_error_message(
                        "the given index cannot be used for indexing a "
                        "dictionary",
                        name, codename, ctx.back_trace()));
            }
            dict[indices] = std::move(value);
            return primitive_argument_type{std::move(dict)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric, range, or dictionary "
                "type and as such does not support slicing",
                name, codename, ctx.back_trace()));
    }

    primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, primitive_argument_type&& value,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        if (is_integer_operand_strict(data))
        {
            if (is_integer_operand_strict(value))
            {
                return primitive_argument_type{
                    slice_assign(
                        extract_integer_value_strict(
                            std::move(data), name, codename),
                        rows, columns,
                        extract_integer_value_strict(
                            std::move(value), name, codename),
                        name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_integer_value_strict(std::move(data), name, codename),
                rows, columns,
                extract_integer_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        if (is_numeric_operand_strict(data))
        {
            if (is_numeric_operand_strict(value))
            {
                return primitive_argument_type{
                    slice_assign(
                        extract_numeric_value_strict(
                            std::move(data), name, codename),
                        rows, columns,
                        extract_numeric_value_strict(
                            std::move(value), name, codename),
                        name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_numeric_value_strict(std::move(data), name, codename),
                rows, columns,
                extract_numeric_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        if (is_boolean_operand_strict(data))
        {
            if (is_boolean_operand_strict(value))
            {
                return primitive_argument_type{
                    slice_assign(
                        extract_boolean_value_strict(
                            std::move(data), name, codename),
                        rows, columns,
                        extract_boolean_value_strict(
                            std::move(value), name, codename),
                        name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_boolean_value_strict(std::move(data), name, codename),
                rows, columns,
                extract_boolean_value(std::move(value), name, codename),
                name, codename, ctx)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing",
                name, codename, ctx.back_trace()));
    }

    primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& pages,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, primitive_argument_type&& value,
        std::string const& name, std::string const& codename,
        eval_context const& ctx)
    {
        if (is_integer_operand_strict(data))
        {
            if (is_integer_operand_strict(value))
            {
                return primitive_argument_type{
                    slice_assign(
                        extract_integer_value_strict(
                            std::move(data), name, codename),
                        pages, rows, columns,
                        extract_integer_value_strict(
                            std::move(value), name, codename),
                        name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_integer_value_strict(std::move(data), name, codename),
                pages, rows, columns,
                extract_integer_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        if (is_numeric_operand_strict(data))
        {
            if (is_numeric_operand_strict(value))
            {
                return primitive_argument_type{
                    slice_assign(
                        extract_numeric_value_strict(
                            std::move(data), name, codename),
                        pages, rows, columns,
                        extract_numeric_value_strict(
                            std::move(value), name, codename),
                        name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_numeric_value_strict(std::move(data), name, codename),
                pages, rows, columns,
                extract_numeric_value(std::move(value), name, codename),
                name, codename, ctx)};
        }
        if (is_boolean_operand_strict(data))
        {
            if (is_boolean_operand_strict(value))
            {
                return primitive_argument_type{
                    slice_assign(
                        extract_boolean_value_strict(
                            std::move(data), name, codename),
                        pages, rows, columns,
                        extract_boolean_value_strict(
                            std::move(value), name, codename),
                        name, codename, ctx)};
            }

            return primitive_argument_type{slice_assign(
                extract_boolean_value_strict(std::move(data), name, codename),
                pages, rows, columns,
                extract_boolean_value(std::move(value), name, codename),
                name, codename, ctx)};
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice",
            util::generate_error_message(
                "target object does not hold a numeric data type and "
                "as such does not support slicing",
                name, codename, ctx.back_trace()));
    }
}}


