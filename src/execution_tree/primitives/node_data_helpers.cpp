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
            result = rhs.scalar();
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

    ///////////////////////////////////////////////////////////////////////////
    // return argument as a vector, scalars are properly broadcast
    template <typename T>
    void extract_value_vector(
        typename ir::node_data<T>::storage1d_type& result,
        ir::node_data<T>&& rhs, std::size_t size, std::string const& name,
        std::string const& codename)
    {
        using storage1d_type = typename ir::node_data<T>::storage1d_type;

        switch (rhs.num_dimensions())
        {
        case 0:
            result = storage1d_type(size, rhs.scalar());
            return;

        case 1:
            {
                // vectors of size one can be broadcast into any other vector
                if (rhs.size() == 1)
                {
                    result = storage1d_type(size, rhs[0]);
                    return;
                }

                if (size != rhs.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_vector",
                        util::generate_error_message(
                            "cannot broadcast a vector into a vector of "
                                "different size",
                            name, codename));
                }

                if (rhs.is_ref())
                {
                    result = rhs.vector();
                }
                else
                {
                    result = std::move(rhs.vector_non_ref());
                }
                return;
            }

        case 2:
            {
                if (result.size() != size)
                {
                    result.resize(size);
                }

                // matrices of size one can be broadcast into any vector
                if (rhs.size() == 1)
                {
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = rhs[0];
                    }
                    return;
                }

                // matrices with one column can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && size == rhs.dimension(1))
                {
                    auto m = rhs.matrix();
                    result = blaze::trans(blaze::row(m, 0));
                    return;
                }

                // matrices with one row can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(1) == 1 && size == rhs.dimension(0))
                {
                    auto m = rhs.matrix();
                    result = blaze::column(m, 0);
                    return;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::extract_value_vector",
                    util::generate_error_message(
                        "cannot broadcast a matrix of arbitrary size into "
                        "a vector",
                        name, codename));
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value_vector",
            util::generate_error_message(
                "primitive_argument_type does not hold a scalar or vector "
                    "numeric value type",
                name, codename));
    }

    template <typename T>
    ir::node_data<T> extract_value_vector(ir::node_data<T>&& arg,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage1d_type result;
        extract_value_vector(result, std::move(arg), size, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    template <typename T>
    ir::node_data<T> extract_value_vector(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        return extract_value_vector(
            extract_node_data<T>(val, name, codename),
            size, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_value_vector(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        return extract_value_vector(
            extract_node_data<T>(std::move(val), name, codename),
            size, name, codename);
    }

    // explicitly instantiate necessary functions
    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_vector<double>(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_vector<std::int64_t>(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_vector<std::uint8_t>(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_vector<double>(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_vector<std::int64_t>(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_vector<std::uint8_t>(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);

    ///////////////////////////////////////////////////////////////////////////
    // return argument as a matrix, scalars and vectors are properly broadcast
    template <typename T>
    void extract_value_matrix(typename ir::node_data<T>::storage2d_type& result,
        ir::node_data<T>&& rhs, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        using storage2d_type = typename ir::node_data<T>::storage2d_type;

        switch (rhs.num_dimensions())
        {
        case 0:
            result = storage2d_type(rows, columns, rhs.scalar());
            return;

        case 1:
            {
                // vectors of size one can be broadcast into any matrix
                if (rhs.size() == 1)
                {
                    result = storage2d_type(rows, columns, rhs[0]);
                    return;
                }

                if (columns != rhs.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_matrix",
                        util::generate_error_message(
                            "cannot broadcast a vector into a matrix with a "
                            "different number of columns",
                            name, codename));
                }

                result.resize(rows, columns);
                for (std::size_t i = 0; i != rows; ++i)
                {
                    blaze::row(result, i) = blaze::trans(rhs.vector());
                }
                return;
            }

        case 2:
            {
                // matrices of size one can be broadcast into any other matrix
                if (rhs.size() == 1)
                {
                    result = storage2d_type(rows, columns, rhs[0]);
                    return;
                }

                // matrices with one column can be broadcast into any other
                // matrix with the same number of columns
                if (rhs.dimension(0) == 1 && columns == rhs.dimension(1))
                {
                    result.resize(rows, columns);
                    auto m = rhs.matrix();
                    auto row = blaze::row(m, 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        blaze::row(result, i) = row;
                    }
                    return;
                }

                // matrices with one row can be broadcast into any other
                // matrix with the same number of rows
                if (rhs.dimension(1) == 1 && rows == rhs.dimension(0))
                {
                    result.resize(rows, columns);
                    auto m = rhs.matrix();
                    auto column = blaze::column(m, 0);
                    for (std::size_t i = 0; i != columns; ++i)
                    {
                        blaze::column(result, i) = column;
                    }
                    return;
                }

                if (rows != rhs.dimension(0) || columns != rhs.dimension(1))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_matrix",
                        util::generate_error_message(
                            "cannot broadcast a matrix into a differently "
                                "sized matrix",
                            name, codename));
                }

                if (rhs.is_ref())
                {
                    result = rhs.matrix();
                }
                else
                {
                    result = std::move(rhs.matrix_non_ref());
                }
                return;
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value_matrix",
            util::generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type",
                name, codename));
    }

    template <typename T>
    ir::node_data<T> extract_value_matrix(ir::node_data<T>&& arg,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        typename ir::node_data<T>::storage2d_type result;
        extract_value_matrix(
            result, std::move(arg), rows, columns, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    template <typename T>
    ir::node_data<T> extract_value_matrix(primitive_argument_type const& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        return extract_value_matrix(
            extract_node_data<T>(val, name, codename),
            rows, columns, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_value_matrix(primitive_argument_type && val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        return extract_value_matrix(
            extract_node_data<T>(std::move(val), name, codename),
            rows, columns, name, codename);
    }

    // explicitly instantiate necessary functions
    template PHYLANX_EXPORT ir::node_data<double> extract_value_matrix<double>(
        primitive_argument_type const& val, std::size_t rows,
        std::size_t columns, std::string const& name,
        std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_matrix<std::int64_t>(primitive_argument_type const& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_matrix<std::uint8_t>(primitive_argument_type const& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double> extract_value_matrix<double>(
        primitive_argument_type&& val, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_matrix<std::int64_t>(primitive_argument_type&& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_matrix<std::uint8_t>(primitive_argument_type&& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);
}}
