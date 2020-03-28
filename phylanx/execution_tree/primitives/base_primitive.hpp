// Copyright (c) 2017-2019 Hartmut Kaiser
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM)
#define PHYLANX_PRIMITIVES_BASE_PRIMITIVE_SEP_05_2017_1102AM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/include/runtime.hpp>
#include <hpx/include/util.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type&& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type&& val,
        primitive_arguments_type&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type const& val,
        primitive_argument_type&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT primitive_argument_type value_operand_sync(
        primitive_argument_type&& val,
        primitive_argument_type&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct value_operand_sync
        {
            template <typename... Ts>
            primitive_argument_type operator()(Ts&&... ts) const
            {
                return execution_tree::value_operand_sync(
                    std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT primitive_argument_type value_operand_ref_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    ///////////////////////////////////////////////////////////////////////////
    inline primitive_argument_type primitive_argument_type::run(
        eval_context ctx) const
    {
        return value_operand_sync(
            *this, primitive_argument_type{}, "", "<unknown>", std::move(ctx));
    }

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT primitive_argument_type to_primitive_value_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<double> to_primitive_numeric_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT std::string to_primitive_string_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<std::int64_t> to_primitive_int_type(
        ast::literal_value_type && val);
    PHYLANX_EXPORT ir::node_data<std::uint8_t> to_primitive_bool_type(
        ast::literal_value_type && val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a value type from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive_argument_type const& extract_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_ref_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type&& extract_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type&& extract_ref_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_copy_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT ir::dictionary extract_dictionary_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::dictionary&& extract_dictionary_value(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT bool is_dictionary_operand(
        primitive_argument_type const& val);

    template <typename T>
    primitive_argument_type extract_ref_value(ir::node_data<T> const& val)
    {
        if (val.is_ref())
        {
            return primitive_argument_type{val};
        }
        return primitive_argument_type{val.ref()};
    }
    template <typename T>
    primitive_argument_type extract_ref_value(ir::node_data<T> && val)
    {
        return primitive_argument_type{std::move(val)};
    }

    template <typename T>
    primitive_argument_type extract_copy_value(ir::node_data<T> const& val)
    {
        if (val.is_ref())
        {
            return primitive_argument_type{val.copy()};
        }
        return primitive_argument_type{val};
    }
    template <typename T>
    primitive_argument_type extract_copy_value(ir::node_data<T> && val)
    {
        if (val.is_ref())
        {
            return primitive_argument_type{val.copy()};
        }
        return primitive_argument_type{std::move(val)};
    }

    // check whether data in argument is a reference
    PHYLANX_EXPORT bool is_ref_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    // Extract a literal type from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive_argument_type extract_literal_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_literal_ref_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive_argument_type extract_literal_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_literal_operand(primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a ir::node_data<double> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT double extract_scalar_numeric_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT double extract_scalar_numeric_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT ir::node_data<double> extract_numeric_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<double>&& extract_numeric_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT double extract_scalar_numeric_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT double extract_scalar_numeric_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_numeric_operand(primitive_argument_type const& val);
    PHYLANX_EXPORT bool is_numeric_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::size_t extract_numeric_value_dimension(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::size_t extract_numeric_value_size(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    extract_numeric_value_dimensions(primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    // Extract a ir::node_data<std::uint8_t> type from a given
    // primitive_argument_type, throw if it doesn't hold one.
    PHYLANX_EXPORT bool is_boolean_data_operand(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a ir::node_data<std::int64_t> type from a given
    // primitive_argument_type, throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract an integer value from a primitive_argument_type
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type const& val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type && val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>> integer_operand(
        primitive_argument_type && val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct integer_operand
        {
            template <typename... Ts>
            hpx::future<ir::node_data<std::int64_t>> operator()(
                Ts&&... ts) const
            {
                return execution_tree::integer_operand(std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT bool is_integer_operand(primitive_argument_type const& val);

    PHYLANX_EXPORT hpx::future<std::int64_t> scalar_integer_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract a ir::node_data<std::int64_t> type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::int64_t> extract_integer_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::int64_t>&& extract_integer_value_strict(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_integer_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract an integer value from a primitive_argument_type
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>>
    integer_operand_strict(primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>>
    integer_operand_strict(primitive_argument_type const& val,
        primitive_arguments_type&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>>
    integer_operand_strict(primitive_argument_type&& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<std::int64_t>>
    integer_operand_strict(primitive_argument_type&& val,
        primitive_arguments_type&& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct integer_operand_strict
        {
            template <typename... Ts>
            hpx::future<ir::node_data<std::int64_t>> operator()(
                Ts&&... ts) const
            {
                return execution_tree::integer_operand_strict(
                    std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT bool is_integer_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::int64_t extract_scalar_positive_integer_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_positive_integer_value(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::int64_t extract_scalar_positive_integer_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_positive_integer_value_strict(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::int64_t extract_scalar_nonneg_integer_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::int64_t extract_scalar_nonneg_integer_value_strict(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT hpx::future<std::int64_t> scalar_integer_operand_strict(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    ///////////////////////////////////////////////////////////////////////////
    // Extract a boolean type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT ir::node_data<std::uint8_t> extract_boolean_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::node_data<std::uint8_t>&& extract_boolean_value_strict(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::uint8_t extract_scalar_boolean_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::uint8_t extract_scalar_boolean_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::uint8_t extract_scalar_boolean_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::uint8_t extract_scalar_boolean_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_boolean_operand(primitive_argument_type const& val);
    PHYLANX_EXPORT bool is_boolean_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> extract_node_data(primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline ir::node_data<double> extract_node_data(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_numeric_value(val, name, codename);
    }
    template <>
    inline ir::node_data<std::int64_t> extract_node_data(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_integer_value(val, name, codename);
    }
    template <>
    inline ir::node_data<std::uint8_t> extract_node_data(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_boolean_value(val, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_node_data(primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline ir::node_data<double> extract_node_data(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_numeric_value(std::move(val), name, codename);
    }
    template <>
    inline ir::node_data<std::int64_t> extract_node_data(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_integer_value(std::move(val), name, codename);
    }
    template <>
    inline ir::node_data<std::uint8_t> extract_node_data(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_boolean_value(std::move(val), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> extract_node_data_strict(
        primitive_argument_type const& val, std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline ir::node_data<double> extract_node_data_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_numeric_value_strict(val, name, codename);
    }
    template <>
    inline ir::node_data<std::int64_t> extract_node_data_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_integer_value_strict(val, name, codename);
    }
    template <>
    inline ir::node_data<std::uint8_t> extract_node_data_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_boolean_value_strict(val, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_node_data_strict(primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline ir::node_data<double> extract_node_data_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_numeric_value_strict(std::move(val), name, codename);
    }
    template <>
    inline ir::node_data<std::int64_t> extract_node_data_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_integer_value_strict(std::move(val), name, codename);
    }
    template <>
    inline ir::node_data<std::uint8_t> extract_node_data_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_boolean_value_strict(std::move(val), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    T extract_scalar_data(primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline double extract_scalar_data(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_numeric_value(val, name, codename);
    }
    template <>
    inline std::int64_t extract_scalar_data(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_integer_value(val, name, codename);
    }
    template <>
    inline std::uint8_t extract_scalar_data(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_boolean_value(val, name, codename);
    }

    template <typename T>
    T extract_scalar_data(primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline double extract_scalar_data(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_numeric_value(std::move(val), name, codename);
    }
    template <>
    inline std::int64_t extract_scalar_data(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_integer_value(std::move(val), name, codename);
    }
    template <>
    inline std::uint8_t extract_scalar_data(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_boolean_value(std::move(val), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    T extract_scalar_data_strict(primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline double extract_scalar_data_strict(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_numeric_value_strict(val, name, codename);
    }
    template <>
    inline std::int64_t extract_scalar_data_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_scalar_integer_value_strict(val, name, codename);
    }
    template <>
    inline std::uint8_t extract_scalar_data_strict(
        primitive_argument_type const& val, std::string const& name,
        std::string const& codename)
    {
        return extract_scalar_boolean_value_strict(val, name, codename);
    }

    template <typename T>
    T extract_scalar_data_strict(primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <>
    inline double extract_scalar_data_strict(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        return extract_scalar_numeric_value_strict(
            std::move(val), name, codename);
    }
    template <>
    inline std::int64_t extract_scalar_data_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_scalar_integer_value_strict(
            std::move(val), name, codename);
    }
    template <>
    inline std::uint8_t extract_scalar_data_strict(
        primitive_argument_type&& val, std::string const& name,
        std::string const& codename)
    {
        return extract_scalar_boolean_value_strict(
            std::move(val), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract a std::string type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT std::string extract_string_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::string extract_string_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_string_operand(primitive_argument_type const& val);

    PHYLANX_EXPORT std::string extract_string_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT std::string extract_string_value_strict(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_string_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a list type from a given primitive_argument_type,
    // Create a list from argument if it does not hold one.
    PHYLANX_EXPORT ir::range extract_list_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::range extract_list_value(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_list_operand(primitive_argument_type const& val);

    // Extract a list type from a given primitive_argument_type,
    // throw if it doesn't hold one.
    PHYLANX_EXPORT ir::range extract_list_value_strict(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT ir::range&& extract_list_value_strict(
        primitive_argument_type&& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::size_t extract_list_value_size(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT bool is_list_operand_strict(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    // Extract a primitive from a given primitive_argument_type, throw
    // if it doesn't hold one.
    PHYLANX_EXPORT primitive const& primitive_operand(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive const& primitive_operand(
        primitive_argument_type const& val,
        compiler::primitive_name_parts const& parts,
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive primitive_operand(
        primitive_argument_type && val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
    PHYLANX_EXPORT primitive primitive_operand(
        primitive_argument_type && val,
        compiler::primitive_name_parts const& parts,
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    // Extract a primitive_argument_type from a primitive_argument_type (that
    // could be a value type).
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val,
        primitive_arguments_type&& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val,
        primitive_arguments_type const& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val,
        primitive_arguments_type&& args,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type const& val, primitive_argument_type const& arg,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> value_operand(
        primitive_argument_type&& val, primitive_argument_type const& arg,
        std::string const& name = "", std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct value_operand
        {
            template <typename... Ts>
            hpx::future<primitive_argument_type> operator()(Ts&&... ts) const
            {
                return execution_tree::value_operand(std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    // was declared above
    //     PHYLANX_EXPORT primitive_argument_type value_operand_sync(
    //         primitive_argument_type const& val,
    //         primitive_arguments_type const& args);

    // Extract a primitive_argument_type from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type const& val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type && val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<primitive_argument_type> literal_operand(
        primitive_argument_type && val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct literal_operand
        {
            template <typename... Ts>
            hpx::future<primitive_argument_type> operator()(Ts&&... ts) const
            {
                return execution_tree::literal_operand(std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT primitive_argument_type literal_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract a node_data<double> from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<double> scalar_numeric_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type const& val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::node_data<double>> numeric_operand(
        primitive_argument_type && val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct numeric_operand
        {
            template <typename... Ts>
            hpx::future<ir::node_data<double>> operator()(Ts&&... ts) const
            {
                return execution_tree::numeric_operand(std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT ir::node_data<double> numeric_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract a boolean from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::node_data<uint8_t>> boolean_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT std::uint8_t scalar_boolean_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<std::uint8_t> scalar_boolean_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract a std::string from a primitive_argument_type (that
    // could be a primitive or a string value).
    PHYLANX_EXPORT hpx::future<std::string> string_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<std::string> string_operand_strict(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT std::string string_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract an AST from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<std::vector<ast::expression>> ast_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT std::vector<ast::expression> ast_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract a list from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type && val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type const& val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::range> list_operand(
        primitive_argument_type && val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct list_operand
        {
            template <typename... Ts>
            hpx::future<ir::range> operator()(Ts&&... ts) const
            {
                return execution_tree::list_operand(std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT ir::range list_operand_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    // Extract a list from a primitive_argument_type (that
    // could be a primitive or a literal value).
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type && val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type const& val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});
    PHYLANX_EXPORT hpx::future<ir::range> list_operand_strict(
        primitive_argument_type && val,
        primitive_arguments_type && args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace functional {

        struct list_operand_strict
        {
            template <typename... Ts>
            hpx::future<ir::range> operator()(Ts&&... ts) const
            {
                return execution_tree::list_operand_strict(
                    std::forward<Ts>(ts)...);
            }
        };
    }    // namespace functional

    PHYLANX_EXPORT ir::range list_operand_strict_sync(
        primitive_argument_type const& val,
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>",
        eval_context ctx = eval_context{});

    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        // Get the string representation for the type
        PHYLANX_EXPORT char const* get_primitive_argument_type_name(
            std::size_t index);
    }
}}

namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace detail
    {
        // Invoke the given function on all items in the input vector, while
        // returning another vector holding the respective results.
        template <typename T, typename Allocator, typename F, typename ... Ts>
        auto map_operands(std::vector<T, Allocator> const& in, F && f,
                Ts && ... ts)
        ->  std::vector<decltype(
                    hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                ),
                typename std::allocator_traits<Allocator>::template
                    rebind_alloc<decltype(
                        hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                    )>
            >
        {
            std::vector<decltype(
                    hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                ),
                typename std::allocator_traits<Allocator>::template
                    rebind_alloc<decltype(
                        hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                    )>
            > out;
            out.reserve(in.size());

            for (auto const& d : in)
            {
                out.emplace_back(hpx::util::invoke(f, d, std::ref(ts)...));
            }
            return out;
        }

        template <typename T, typename Allocator, typename F, typename ... Ts>
        auto map_operands(std::vector<T, Allocator> && in, F && f, Ts && ... ts)
        ->  std::vector<decltype(
                    hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                ),
                typename std::allocator_traits<Allocator>::template
                    rebind_alloc<decltype(
                        hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                    )>
            >
        {
            std::vector<decltype(
                    hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                ),
                typename std::allocator_traits<Allocator>::template
                    rebind_alloc<decltype(
                        hpx::util::invoke(f, std::declval<T>(), std::ref(ts)...)
                    )>
            > out;
            out.reserve(in.size());

            for (auto && d : in)
            {
                out.emplace_back(
                    hpx::util::invoke(f, std::move(d), std::ref(ts)...));
            }
            return out;
        }

        ///////////////////////////////////////////////////////////////////////
        // check if one of the optionals in the list of operands is empty
        inline bool verify_argument_values(
            primitive_arguments_type const& ops)
        {
            for (auto const& op : ops)
            {
                if (!valid(op))
                    return false;
            }
            return true;
        }
    }
}}}

#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#endif


