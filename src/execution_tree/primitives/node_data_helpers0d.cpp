// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

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
    node_data_type map_dtype(std::string const& spec)
    {
        node_data_type result = node_data_type_unknown;
        if (spec == "bool")
        {
            result = node_data_type_bool;
        }
        else if (spec.find("int") == 0)
        {
            result = node_data_type_int64;
        }
        else if (spec.find("float") == 0)
        {
            result = node_data_type_double;
        }
        return result;
    }

    node_data_type extract_dtype(std::string name)
    {
        compiler::primitive_name_parts name_parts;
        if (compiler::parse_primitive_name(name, name_parts))
        {
            name = std::move(name_parts.primitive);
        }

        auto p = name.find("__");
        if (p != std::string::npos)
        {
            return map_dtype(std::string(&name[p + 2], name.size() - p - 2));
        }
        return node_data_type_unknown;
    }

    ///////////////////////////////////////////////////////////////////////////
    node_data_type extract_common_type(primitive_argument_type const& arg)
    {
        node_data_type result = node_data_type_unknown;
        if (is_numeric_operand_strict(arg))
        {
            result = node_data_type_double;
        }
        else if (is_integer_operand_strict(arg))
        {
            result = node_data_type_int64;
        }
        else if (is_boolean_operand_strict(arg))
        {
            result = node_data_type_bool;
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    node_data_type extract_common_type(
        primitive_arguments_type const& args)
    {
        node_data_type result = node_data_type_unknown;
        for (auto const& arg : args)
        {
            if (is_numeric_operand_strict(arg))
            {
                result = node_data_type_double;
                break;
            }
            else if (is_integer_operand_strict(arg) &&
                (result == node_data_type_unknown ||
                    result == node_data_type_bool))
            {
                result = node_data_type_int64;
            }
            else if (is_boolean_operand_strict(arg) &&
                result == node_data_type_unknown)
            {
                result = node_data_type_bool;
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
            if (is_numeric_operand(arg))
            {
                result = (std::min)(result,
                    extract_numeric_value_dimension(arg, name, codename));

                if (result == 0)
                {
                    break;      // can't get larger than that
                }
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t num_dimensions(
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
    {
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        if (dims[2] != 0)
        {
            return 3;
        }
#endif
        if (dims[1] != 0)
        {
            return 2;
        }
        if (dims[0] != 0)
        {
            return 1;
        }
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    align_dimensions_to_vector(std::size_t real_numdims,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
    {
        HPX_ASSERT(real_numdims == 0);
        return {};
    }

    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    align_dimensions_to_matrix(std::size_t real_numdims,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
    {
        HPX_ASSERT(real_numdims <= 1);
        if (real_numdims == 0)
        {
            return {};
        }

        return {0, dims[0]};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    align_dimensions_to_tensor(std::size_t real_numdims,
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims)
    {
        HPX_ASSERT(real_numdims <= 2);
        if (real_numdims == 0)
        {
            return {};
        }
        if (real_numdims == 1)
        {
            return {0, 0, dims[0]};
        }

        return {0, dims[0], dims[1]};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    extract_aligned_dimensions(
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& dims,
        std::size_t numdims, std::string const& name,
        std::string const& codename)
    {
        std::size_t real_numdims = num_dimensions(dims);
        if (real_numdims == numdims)
        {
            return dims;
        }

        if (real_numdims > numdims)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::extract_aligned_dimensions",
                util::generate_error_message(
                    "invalid dimensionality",
                    name, codename));
        }

        switch (numdims)
        {
        case 1:
            return align_dimensions_to_vector(real_numdims, dims);

        case 2:
            return align_dimensions_to_matrix(real_numdims, dims);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return align_dimensions_to_tensor(real_numdims, dims);
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_aligned_dimensions",
            util::generate_error_message(
                "unexpected dimensionality",
                name, codename));
    }

    std::size_t extract_largest_dimension(
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename)
    {
        std::size_t result = 0;
        for (auto const& arg : args)
        {
            if (is_numeric_operand(arg))
            {
                result = (std::max)(result,
                    extract_numeric_value_dimension(arg, name, codename));

                if (result == PHYLANX_MAX_DIMENSIONS)
                {
                    break;      // can't get larger than that
                }
            }
        }
        return result;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    extract_largest_dimensions(
        primitive_arguments_type const& args,
        std::string const& name, std::string const& codename)
    {
        std::size_t numdims = extract_largest_dimension(args, name, codename);

        std::vector<std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>> sizes;
        sizes.reserve(args.size());

        for (auto const& arg : args)
        {
            if (is_numeric_operand(arg))
            {
                sizes.emplace_back(extract_aligned_dimensions(
                    extract_numeric_value_dimensions(arg, name, codename),
                    numdims, name, codename));
            }
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result{};

        auto max_array =
            [](std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& lhs,
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& rhs)
            {
                return std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>{
                        (std::max)(lhs[0], rhs[0])
                      , (std::max)(lhs[1], rhs[1])
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                      , (std::max)(lhs[2], rhs[2])
#endif
                    };
            };

        return std::accumulate(sizes.begin(), sizes.end(), result, max_array);
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
