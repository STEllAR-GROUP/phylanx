// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    node_data_type extract_dtype(std::string name)
    {
        compiler::primitive_name_parts name_parts;
        if (compiler::parse_primitive_name(name, name_parts))
        {
            name = std::move(name_parts.primitive);
        }

        node_data_type result = node_data_type_unknown;
        auto p = name.find("__");
        if (p != std::string::npos)
        {
            boost::string_ref spec(&name[p + 2], name.size() - p - 2);
            if (spec == "bool")
            {
                result = node_data_type_bool;
            }
            else if (spec == "int")
            {
                result = node_data_type_int64;
            }
            else if (spec == "float")
            {
                result = node_data_type_double;
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    node_data_type extract_common_type(primitive_argument_type const& arg)
    {
        node_data_type result = node_data_type_bool;
        if (is_numeric_operand_strict(arg))
        {
            result = node_data_type_double;
        }
        else if (is_integer_operand_strict(arg))
        {
            result = node_data_type_int64;
        }
        else if (!is_boolean_operand_strict(arg))
        {
            result = node_data_type_unknown;
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    node_data_type extract_common_type(
        primitive_arguments_type const& args)
    {
        node_data_type result = node_data_type_bool;
        for (auto const& arg : args)
        {
            if (is_numeric_operand_strict(arg))
            {
                result = node_data_type_double;
                break;
            }
            else if (is_integer_operand_strict(arg))
            {
                result = node_data_type_int64;
            }
            else if (!is_boolean_operand_strict(arg))
            {
                result = node_data_type_unknown;
                break;
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t extract_smallest_dimension(
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename)
    {
        std::size_t result = 2;
        for (auto const& arg : args)
        {
            result = (std::min)(
                result, extract_numeric_value_dimension(arg, name, codename));

            if (result == 0)
            {
                break;      // can't get larger than that
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t extract_largest_dimension(
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename)
    {
        std::size_t result = 0;
        for (auto const& arg : args)
        {
            result = (std::max)(
                result, extract_numeric_value_dimension(arg, name, codename));

            if (result == 2)
            {
                break;      // can't get larger than that
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> extract_largest_dimensions(
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename)
    {
        std::vector<std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>> sizes;
        sizes.reserve(args.size());

        for (auto const& arg : args)
        {
            sizes.emplace_back(
                extract_numeric_value_dimensions(arg, name, codename));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result{0, 0};

        auto max_array =
            [](std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& lhs,
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& rhs)
            {
                return std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>{
                    (std::max)(lhs[0], rhs[0]), (std::max)(lhs[1], rhs[1])};
            };

        return std::accumulate(sizes.begin(), sizes.end(), result, max_array);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void extract_value_scalar(
        typename ir::node_data<T>::storage0d_type& result,
        ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename)
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            if (rhs.is_ref())
            {
                result = rhs.scalar();
            }
            else
            {
                result = std::move(rhs.scalar_non_ref());
            }
            return;

        case 1:
            {
                auto v = rhs.vector();
                if (v.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_scalar",
                        util::generate_error_message(
                            "cannot broadcast a vector of size larger than one "
                            "into a scalar",
                            name, codename));
                }
                result = v[0];
                return;
            }

        case 2:
            {
                auto m = rhs.matrix();
                if (m.rows() != 1 || m.columns() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_scalar",
                        util::generate_error_message(
                            "cannot broadcast a matrix of size larger than one "
                            "by one into a scalar",
                            name, codename));
                }
                result = m(0, 0);
                return;
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            {
                auto t = rhs.tensor();
                if (t.rows() != 1 || t.columns() != 1 || t.pages() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_scalar",
                        util::generate_error_message(
                            "cannot broadcast a tensor of size larger than one "
                            "by one by one into a scalar",
                            name, codename));
                }
                result = t(0, 0, 0);
                return;
            }
#endif

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value_scalar",
            util::generate_error_message(
                "primitive_argument_type does not hold a scalar numeric "
                    "value type",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    // return argument as a scalar
    template <typename T>
    ir::node_data<T> extract_value_scalar(ir::node_data<T>&& arg,
        std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage0d_type result;
        extract_value_scalar(result, std::move(arg), name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    template <typename T>
    ir::node_data<T> extract_value_scalar(primitive_argument_type const& val,
        std::string const& name, std::string const& codename)
    {
        return extract_value_scalar(
            extract_node_data<T>(val, name, codename),
            name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_value_scalar(primitive_argument_type&& val,
        std::string const& name, std::string const& codename)
    {
        return extract_value_scalar(
            extract_node_data<T>(std::move(val), name, codename),
            name, codename);
    }

    // explicitly instantiate necessary functions
    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_scalar<double>(primitive_argument_type const& val,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_scalar<std::int64_t>(primitive_argument_type const& val,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_scalar<std::uint8_t>(primitive_argument_type const& val,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_scalar<double>(primitive_argument_type&& val,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_scalar<std::int64_t>(primitive_argument_type&& val,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_scalar<std::uint8_t>(primitive_argument_type&& val,
        std::string const& name, std::string const& codename);
}}
