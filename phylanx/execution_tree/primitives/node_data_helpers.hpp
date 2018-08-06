// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_NODE_DATA_HELPERS_AUG_05_2018_0446PM)
#define PHYLANX_EXECUTION_TREE_NODE_DATA_HELPERS_AUG_05_2018_0446PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <type_traits>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    enum node_data_type
    {
        node_data_type_unknown = -1,
        node_data_type_double = 0,
        node_data_type_int64 = 1,
        node_data_type_bool = 2,
    };

    /// Return the common data type to be used for the result of an operation
    /// involving the given argument.
    PHYLANX_EXPORT node_data_type extract_common_type(
        primitive_argument_type const& args);

    /// Return the common data type to be used for the result of an operation
    /// involving the given arguments.
    PHYLANX_EXPORT node_data_type extract_common_type(
        std::vector<primitive_argument_type> const& args);

    /// Return the common data type to be used for the result of an operation
    /// involving the given arguments.
    template <typename... Ts>
    node_data_type extract_common_type(Ts const&... args)
    {
        int const __dummy[] = {
            extract_common_type(args)...,
            node_data_type_bool
        };
        return node_data_type(
            *std::min_element(&__dummy[0], &__dummy[sizeof...(args)]));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract the largest dimension from all of the given arguments
    PHYLANX_EXPORT std::size_t extract_largest_dimension(
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract the largest dimension from all of the given arguments
    template <typename... Ts>
    std::size_t extract_largest_dimension(std::string const& name,
        std::string const& codename, Ts const&... args)
    {
        std::size_t const __dummy[] = {
            extract_numeric_value_dimension(args, name, codename)...,
            0
        };
        return *std::max_element(&__dummy[0], &__dummy[sizeof...(args)]);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract the largest dimension from all of the given arguments
    PHYLANX_EXPORT std::array<std::size_t, 2> extract_largest_dimensions(
        std::vector<primitive_argument_type> const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract the largest dimension from all of the given arguments
    template <typename... Ts>
    std::array<std::size_t, 2> extract_largest_dimensions(
        std::string const& name, std::string const& codename, Ts const&... args)
    {
        std::array<std::size_t, 2> const __dummy[] =
        {
            extract_numeric_value_dimensions(args, name, codename)...,
            std::array<std::size_t, 2>{0, 0}
        };

        auto max_array =
            [](std::array<std::size_t, 2> const& lhs,
                std::array<std::size_t, 2> const& rhs)
            {
                return std::array<std::size_t, 2>{
                    (std::max)(lhs[0], rhs[0]), (std::max)(lhs[1], rhs[1])};
            };

        return std::accumulate(&__dummy[0], &__dummy[sizeof...(args)],
            std::array<std::size_t, 2>{0, 0}, max_array);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract a scalar value from the given argument
    template <typename T>
    ir::node_data<T> extract_value_scalar(primitive_argument_type const& val,
        std::string const& name, std::string const& codename);

    template <typename T>
    ir::node_data<T> extract_value_scalar(primitive_argument_type&& val,
        std::string const& name, std::string const& codename);

    // Extract a vector value from the given argument
    template <typename T>
    ir::node_data<T> extract_value_vector(primitive_argument_type const& val,
        std::size_t size, std::string const& name, std::string const& codename);

    template <typename T>
    ir::node_data<T> extract_value_vector(primitive_argument_type&& val,
        std::size_t size, std::string const& name, std::string const& codename);

    // Extract a matrix value from the given argument
    template <typename T>
    ir::node_data<T> extract_value_matrix(primitive_argument_type const& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);

    template <typename T>
    ir::node_data<T> extract_value_matrix(primitive_argument_type&& val,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a scalar, apply given function to value before
    // returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    ir::node_data<T>
    extract_value_scalar(Arg&& val, F&& f, std::string const& name,
        std::string const& codename)
    {
        ir::node_data<T> result =
            extract_node_data<T>(std::forward<Arg>(val), name, codename);

        switch (result.num_dimensions())
        {
        case 0:
            return ir::node_data<T>{f(result.scalar())};

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

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a vector, scalars are properly broadcast, apply given
    // function to all elements of the vector before returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    ir::node_data<T>
    extract_value_vector(Arg&& val, F&& f, std::size_t size,
        std::string const& name, std::string const& codename)
    {
        using storage1d_type = typename ir::node_data<T>::storage1d_type;

        ir::node_data<T> && arg =
            extract_node_data<T>(std::forward<Arg>(val), name, codename);

        switch (arg.num_dimensions())
        {
        case 0:
            {
                storage1d_type result(size);
                for (std::size_t i = 0; i != size; ++i)
                {
                    result[i] = f(arg.scalar(), i);
                }
                return ir::node_data<T>{std::move(result)};
            }

        case 1:
            {
                // vectors of size one can be broadcast into any other vector
                if (arg.size() == 1)
                {
                    storage1d_type result(size);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(arg[0], i);
                    }
                    return ir::node_data<T>{std::move(result)};
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

                if (arg.is_ref())
                {
                    storage1d_type result(size);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(arg[i], i);
                    }
                    return ir::node_data<T>{std::move(result)};
                }

                for (std::size_t i = 0; i != size; ++i)
                {
                    arg[i] = f(arg[i], i);
                }
                return ir::node_data<T>{std::move(arg)};
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

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a matrix, scalars and vectors are properly broadcast,
    // apply given function to all elements of the matrix before returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    ir::node_data<T>
    extract_value_matrix(Arg&& val, F&& f, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        using storage1d_type = typename ir::node_data<T>::storage1d_type;
        using storage2d_type = typename ir::node_data<T>::storage2d_type;

        ir::node_data<T> arg = extract_node_data<T>(val, name, codename);

        switch (arg.num_dimensions())
        {
        case 0:
            {
                storage2d_type result(rows, columns);
                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = f(arg.scalar(), i, j);
                    }
                }
                return ir::node_data<T>{std::move(result)};
            }

        case 1:
            {
                // vectors of size one can be broadcast into any matrix
                if (arg.size() == 1)
                {
                    storage2d_type result(rows, columns);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(arg[0], i, j);
                        }
                    }
                    return ir::node_data<T>{std::move(result)};
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
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = f(arg[j], i, j);
                    }
                }
                return ir::node_data<T>{std::move(result)};
            }

        case 2:
            {
                // matrices of size one can be broadcast into any other matrix
                if (arg.size() == 1)
                {
                    storage2d_type result(rows, columns);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(arg[0], i, j);
                        }
                    }
                    return ir::node_data<T>{std::move(result)};
                }

                // matrices with one column can be broadcast into any other
                // matrix with the same number of columns
                if (arg.dimension(0) == 1 && columns == arg.dimension(1))
                {
                    storage2d_type result(rows, columns);
                    auto row = blaze::row(arg.matrix(), 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(row[j], i, j);
                        }
                    }
                    return ir::node_data<T>{std::move(result)};
                }

                // matrices with one row can be broadcast into any other
                // matrix with the same number of rows
                if (arg.dimension(1) == 1 && rows == arg.dimension(0))
                {
                    storage2d_type result(rows, columns);
                    auto column = blaze::column(arg.matrix(), 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(column[i], i, j);
                        }
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

                if (arg.is_ref())
                {
                    storage2d_type result(rows, columns);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(arg.at(i, j), i, j);
                        }
                    }
                    return ir::node_data<T>{std::move(result)};
                }

                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        arg.at(i, j) = f(arg.at(i, j), i, j);
                    }
                }
                return ir::node_data<T>{std::move(arg)};
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
}}

#endif
