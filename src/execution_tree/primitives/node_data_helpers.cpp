// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
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
        std::vector<primitive_argument_type> const& args)
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
    std::size_t extract_largest_dimension(
        std::vector<primitive_argument_type> const& args,
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
    // return argument as a scalar
    template <typename T>
    ir::node_data<T> extract_value_scalar(ir::node_data<T>&& arg,
        std::string const& name, std::string const& codename)
    {
        switch (arg.num_dimensions())
        {
        case 0:
            return ir::node_data<T>{arg.scalar()};

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
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
    ir::node_data<T> extract_value_vector(ir::node_data<T>&& arg,
        std::size_t size, std::string const& name, std::string const& codename)
    {
        using storage1d_type = typename ir::node_data<T>::storage1d_type;

        switch (arg.num_dimensions())
        {
        case 0:
            return ir::node_data<T>{storage1d_type(size, arg.scalar())};

        case 1:
            {
                // vectors of size one can be broadcast into any other vector
                if (arg.size() == 1)
                {
                    return ir::node_data<T>{storage1d_type(size, arg[0])};
                }

                if (size != arg.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_vector",
                        util::generate_error_message(
                            "cannot broadcast a vector into a vector of "
                                "different size",
                            name, codename));
                }

                return std::move(arg);
            }

        case 2: HPX_FALLTHROUGH;
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
    ir::node_data<T> extract_value_matrix(ir::node_data<T>&& arg,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        using storage2d_type = typename ir::node_data<T>::storage2d_type;

        switch (arg.num_dimensions())
        {
        case 0:
            return ir::node_data<T>{storage2d_type(rows, columns, arg.scalar())};

        case 1:
            {
                // vectors of size one can be broadcast into any matrix
                if (arg.size() == 1)
                {
                    return ir::node_data<T>{
                        storage2d_type(rows, columns, arg[0])};
                }

                if (columns != arg.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_matrix",
                        util::generate_error_message(
                            "cannot broadcast a vector into a matrix with a "
                            "different number of columns",
                            name, codename));
                }

                storage2d_type result(rows, columns);
                for (std::size_t i = 0; i != rows; ++i)
                {
                    blaze::row(result, i) = blaze::trans(arg.vector());
                }
                return ir::node_data<T>{std::move(result)};
            }

        case 2:
            {
                // matrices of size one can be broadcast into any other matrix
                if (arg.size() == 1)
                {
                    return ir::node_data<T>{
                        storage2d_type(rows, columns, arg[0])};
                }

                // matrices with one column can be broadcast into any other
                // matrix with the same number of columns
                if (arg.dimension(0) == 1 && columns == arg.dimension(1))
                {
                    storage2d_type result(rows, columns);
                    auto row = blaze::row(arg.matrix(), 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        blaze::row(result, i) = row;
                    }
                    return ir::node_data<T>{std::move(result)};
                }

                // matrices with one row can be broadcast into any other
                // matrix with the same number of rows
                if (arg.dimension(1) == 1 && rows == arg.dimension(0))
                {
                    storage2d_type result(rows, columns);
                    auto column = blaze::column(arg.matrix(), 0);
                    for (std::size_t i = 0; i != columns; ++i)
                    {
                        blaze::column(result, i) = column;
                    }
                    return ir::node_data<T>{std::move(result)};
                }

                if (rows != arg.dimension(0) || columns != arg.dimension(1))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_matrix",
                        util::generate_error_message(
                            "cannot broadcast a matrix into a differently "
                                "sized matrix",
                            name, codename));
                }

                return std::move(arg);
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
