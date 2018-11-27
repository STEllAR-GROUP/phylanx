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

    /// Extract node_data_type from a primitive name
    PHYLANX_EXPORT node_data_type extract_dtype(std::string name);

    /// Return the common data type to be used for the result of an operation
    /// involving the given argument.
    PHYLANX_EXPORT node_data_type extract_common_type(
        primitive_argument_type const& args);

    /// Return the common data type to be used for the result of an operation
    /// involving the given arguments.
    PHYLANX_EXPORT node_data_type extract_common_type(
        primitive_arguments_type const& args);

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
    // Extract the smallest dimension from all of the given arguments
    PHYLANX_EXPORT std::size_t extract_smallest_dimension(
        primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract the largest dimension from all of the given arguments
    template <typename... Ts>
    std::size_t extract_smallest_dimension(std::string const& name,
        std::string const& codename, Ts const&... args)
    {
        std::size_t const __dummy[] = {
            extract_numeric_value_dimension(args, name, codename)...,
            0
        };
        return *std::min_element(&__dummy[0], &__dummy[sizeof...(args)]);
    }

    ///////////////////////////////////////////////////////////////////////////
    // Extract the largest dimension from all of the given arguments
    PHYLANX_EXPORT std::size_t extract_largest_dimension(
        primitive_arguments_type const& args,
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
    PHYLANX_EXPORT std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>
    extract_largest_dimensions(primitive_arguments_type const& args,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    // Extract the largest dimension from all of the given arguments
    template <typename... Ts>
    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> extract_largest_dimensions(
        std::string const& name, std::string const& codename, Ts const&... args)
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const __dummy[] =
        {
            extract_numeric_value_dimensions(args, name, codename)...,
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>{0, 0}
        };

        auto max_array =
            [](std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& lhs,
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> const& rhs)
            {
                return std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>{
                    (std::max)(lhs[0], rhs[0]), (std::max)(lhs[1], rhs[1])};
            };

        return std::accumulate(&__dummy[0], &__dummy[sizeof...(args)],
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS>{0, 0}, max_array);
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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // Extract a matrix value from the given argument
    template <typename T>
    ir::node_data<T> extract_value_tensor(primitive_argument_type const& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);

    template <typename T>
    ir::node_data<T> extract_value_tensor(primitive_argument_type&& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
#endif

    ///////////////////////////////////////////////////////////////////////////
    // in-place versions of above functions
    template <typename T>
    void extract_value_scalar(
        typename ir::node_data<T>::storage0d_type& result,
        ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    void extract_value_vector(
        typename ir::node_data<T>::storage1d_type& result,
        ir::node_data<T>&& val, std::size_t size, std::string const& name,
        std::string const& codename);

    template <typename T>
    void extract_value_matrix(
        typename ir::node_data<T>::storage2d_type& result,
        ir::node_data<T>&& rhs, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    void extract_value_tensor(
        typename ir::node_data<T>::storage3d_type& result,
        ir::node_data<T>&& rhs,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
#endif

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a scalar in place of existing storage,
    // apply given function to value before returning
    template <typename T, typename F>
    void extract_value_scalar(typename ir::node_data<T>::storage0d_type& result,
        ir::node_data<T>&& rhs, F&& f, std::string const& name,
        std::string const& codename)
    {
        switch (rhs.num_dimensions())
        {
        case 0:
            result = f(rhs.scalar());
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
                result = f(v[0]);
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
                result = f(m(0, 0));
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
                result = f(t(0, 0, 0));
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
    // Return argument as a vector in place of existing storage,
    // scalars are properly broadcast, apply given
    // function to all elements of the vector before returning
    template <typename T, typename F>
    void extract_value_vector(typename ir::node_data<T>::storage1d_type& result,
        ir::node_data<T>&& rhs, F&& f, std::size_t size,
        std::string const& name, std::string const& codename)
    {
        if (result.size() != size)
        {
            result.resize(size);
        }

        switch (rhs.num_dimensions())
        {
        case 0:
            {

                for (std::size_t i = 0; i != size; ++i)
                {
                    result[i] = f(rhs.scalar(), i);
                }
                return;
            }

        case 1:
            {
                // vectors of size one can be broadcast into any other vector
                if (rhs.size() == 1)
                {
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(rhs[0], i);
                    }
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

                for (std::size_t i = 0; i != size; ++i)
                {
                    result[i] = f(rhs[i], i);
                }
                return;
            }

        case 2:
            {
                // matrices of size one can be broadcast into any vector
                if (rhs.size() == 1)
                {
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(rhs[0], i);
                    }
                    return;
                }

                // matrices with one column can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && size == rhs.dimension(1))
                {
                    auto m = rhs.matrix();
                    auto row = blaze::row(m, 0);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(row[i], i);
                    }
                    return;
                }

                // matrices with one row can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(1) == 1 && size == rhs.dimension(0))
                {
                    auto m = rhs.matrix();
                    auto column = blaze::column(m, 0);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(column[i], i);
                    }
                    return;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::extract_value_vector",
                    util::generate_error_message(
                        "cannot broadcast a matrix of arbitrary size into "
                        "a vector",
                        name, codename));
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            {
                // tensors of size one can be broadcast into any vector
                if (rhs.size() == 1)
                {
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(rhs.at(0, 0, 0), i);
                    }
                    return;
                }

                // tensors with one row can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && size == rhs.dimension(1) &&
                    rhs.dimension(2) == 1)
                {
                    auto t = rhs.tensor();
                    auto row = blaze::row(blaze::pageslice(t, 0), 0);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(row[i], i);
                    }
                    return;
                }

                // tensors with one column can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == size && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == 1)
                {
                    auto t = rhs.tensor();
                    auto column = blaze::column(blaze::pageslice(t, 0), 0);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(column[i], i);
                    }
                    return;
                }

                // tensors with one page can be broadcast into any vector
                // with the same number of elements
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == size)
                {
                    auto t = rhs.tensor();
                    auto page = blaze::column(blaze::rowslice(t, 0), 0);
                    for (std::size_t i = 0; i != size; ++i)
                    {
                        result[i] = f(page[i], i);
                    }
                    return;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::extract_value_vector",
                    util::generate_error_message(
                        "cannot broadcast a matrix of arbitrary size into "
                        "a vector",
                        name, codename));
            }
#endif

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
    // Return argument as a matrix in place of existing storage,
    // scalars and vectors are properly broadcast,
    // apply given function to all elements of the matrix before returning
    template <typename T, typename F>
    void extract_value_matrix(typename ir::node_data<T>::storage2d_type& result,
        ir::node_data<T>&& rhs, F&& f, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        if (result.rows() != rows || result.columns() != columns)
        {
            result.resize(rows, columns);
        }

        switch (rhs.num_dimensions())
        {
        case 0:
            {
                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = f(rhs.scalar(), i, j);
                    }
                }
                return;
            }

        case 1:
            {
                // vectors of size one can be broadcast into any matrix
                if (rhs.size() == 1)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(rhs[0], i, j);
                        }
                    }
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

                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = f(rhs[j], i, j);
                    }
                }
                return;
            }

        case 2:
            {
                // matrices of size one can be broadcast into any other matrix
                if (rhs.size() == 1)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(rhs[0], i, j);
                        }
                    }
                    return;
                }

                // matrices with one column can be broadcast into any other
                // matrix with the same number of columns
                if (rhs.dimension(0) == 1 && columns == rhs.dimension(1))
                {
                    auto m = rhs.matrix();
                    auto row = blaze::row(m, 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(row[j], i, j);
                        }
                    }
                    return;
                }

                // matrices with one row can be broadcast into any other
                // matrix with the same number of rows
                if (rhs.dimension(1) == 1 && rows == rhs.dimension(0))
                {
                    auto m = rhs.matrix();
                    auto column = blaze::column(m, 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(column[i], i, j);
                        }
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

                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = f(rhs.at(i, j), i, j);
                    }
                }
                return;
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
//         case 3:
//             {
//                 // tensors of size one can be broadcast into any other matrix
//                 if (rhs.size() == 1)
//                 {
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(rhs.at(0, 0, 0), i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 // tensors with one row can be broadcast into any other
//                 // matrix with the same number of columns
//                 if (rhs.dimension(0) == 1 && rhs.dimension(1) == columns &&
//                     rhs.dimension(2) == 1)
//                 {
//                     auto t = rhs.tensor();
//                     auto row = blaze::row(blaze::pageslice(t, 0), 0);
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(row[j], i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 // tensors with one column can be broadcast into any other
//                 // matrix with the same number of rows
//                 if (rhs.dimension(0) == rows && rhs.dimension(1) == 1 &&
//                     rhs.dimension(2) == 1)
//                 {
//                     auto t = rhs.tensor();
//                     auto column = blaze::column(blaze::pageslice(t, 0), 0);
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(column[i], i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 // tensors with one page-column can be broadcast into any other
//                 // matrix with the same number of rows
//                 if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
//                     rhs.dimension(2) == rows)
//                 {
//                     auto t = rhs.tensor();
//                     auto column = blaze::column(blaze::rowslice(t, 0), 0);
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(column[i], i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 // pageslice matches
//                 if (rhs.dimension(0) == rows && rhs.dimension(1) == columns &&
//                     rhs.dimension(2) == 1)
//                 {
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(rhs.at(i, j, 0), i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 // columnslice matches
//                 if (rhs.dimension(0) == rows && rhs.dimension(1) == 1 &&
//                     rhs.dimension(2) == columns)
//                 {
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(rhs.at(i, 0, j), i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 // rowslice matches
//                 if (rhs.dimension(0) == 1 && rhs.dimension(1) == rows &&
//                     rhs.dimension(2) == columns)
//                 {
//                     for (std::size_t i = 0; i != rows; ++i)
//                     {
//                         for (std::size_t j = 0; j != columns; ++j)
//                         {
//                             result(i, j) = f(rhs.at(0, i, j), i, j);
//                         }
//                     }
//                     return;
//                 }
//
//                 HPX_THROW_EXCEPTION(hpx::bad_parameter,
//                     "phylanx::execution_tree::extract_value_matrix",
//                     util::generate_error_message(
//                         "cannot broadcast a tensor into a differently "
//                             "sized matrix",
//                         name, codename));
//             }
#endif

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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a matrix in place of existing storage,
    // scalars and vectors are properly broadcast,
    // apply given function to all elements of the matrix before returning
    template <typename T, typename F>
    void extract_value_tensor(typename ir::node_data<T>::storage3d_type& result,
        ir::node_data<T>&& rhs, F&& f, std::size_t pages, std::size_t rows,
        std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        if (result.rows() != rows || result.columns() != columns ||
            result.pages() != pages)
        {
            result.resize(rows, columns, pages);
        }

        switch (rhs.num_dimensions())
        {
        case 0:
            {
                for (std::size_t k = 0; k != pages; ++k)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j, k) = f(rhs.scalar(), i, j, k);
                        }
                    }
                }
                return;
            }

        case 1:
            {
                // vectors of size one can be broadcast into any matrix
                if (rhs.size() == 1)
                {
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                result(i, j, k) = f(rhs[0], i, j, k);
                            }
                        }
                    }
                    return;
                }

                if (columns != rhs.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_tensor",
                        util::generate_error_message(
                            "cannot broadcast a vector into a tensor with a "
                                "different number of columns",
                            name, codename));
                }

                for (std::size_t k = 0; k != pages; ++k)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j, k) = f(rhs[j], i, j, k);
                        }
                    }
                }
                return;
            }

        case 2:
            {
                // matrices of size one can be broadcast into any tensor
                if (rhs.size() == 1)
                {
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                result(i, j, k) = f(rhs[0], i, j, k);
                            }
                        }
                    }
                    return;
                }

                // matrices with one column can be broadcast into any
                // tensors with the same number of columns
                if (rhs.dimension(0) == 1 && columns == rhs.dimension(1))
                {
                    auto m = rhs.matrix();
                    auto row = blaze::row(m, 0);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                result(i, j, k) = f(row[j], i, j, k);
                            }
                        }
                    }
                    return;
                }

                // matrices with one row can be broadcast into any other
                // matrix with the same number of rows
                if (rhs.dimension(1) == 1 && rows == rhs.dimension(0))
                {
                    auto m = rhs.matrix();
                    auto column = blaze::column(m, 0);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(i, j) = f(column[i], i, j);
                        }
                    }
                    return;
                }

                if (rows != rhs.dimension(0) || columns != rhs.dimension(1))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_tensor",
                        util::generate_error_message(
                            "cannot broadcast a matrix into a differently "
                                "sized matrix",
                            name, codename));
                }

                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = f(rhs.at(i, j), i, j);
                    }
                }
                return;
            }

        case 3:
            break;

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value_tensor",
            util::generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type",
                name, codename));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a scalar, apply given function to value before
    // returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    ir::node_data<T> extract_value_scalar(Arg&& arg, F&& f,
        std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage0d_type result;
        extract_value_scalar(result,
            extract_node_data<T>(std::forward<Arg>(arg), name, codename),
            std::forward<F>(f), name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a vector, scalars are properly broadcast, apply given
    // function to all elements of the vector before returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    ir::node_data<T> extract_value_vector(Arg&& arg, F&& f, std::size_t size,
            std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage1d_type result;
        extract_value_vector(result,
            extract_node_data<T>(std::forward<Arg>(arg), name, codename),
            std::forward<F>(f), size, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a matrix, scalars and vectors are properly broadcast,
    // apply given function to all elements of the matrix before returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    ir::node_data<T> extract_value_matrix(Arg&& arg, F&& f,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        typename ir::node_data<T>::storage2d_type result;
        extract_value_matrix(result,
            extract_node_data<T>(std::forward<Arg>(arg), name, codename),
            std::forward<F>(f), rows, columns, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    // Return argument as a tensor in place of existing storage,
    // scalars, vectors, and matrices are properly broadcast,
    // apply given function to all elements of the tensor before returning
    template <typename T, typename Arg, typename F,
        typename Enable = typename std::enable_if<std::is_same<
            typename std::decay<Arg>::type, primitive_argument_type
        >::value>::type>
    void extract_value_tensor(Arg&& arg, F&& f, std::size_t pages,
        std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage3d_type result;
        extract_value_tensor(result,
            extract_node_data<T>(std::forward<Arg>(arg), name, codename),
            std::forward<F>(f), pages, rows, columns, name, codename);
        return ir::node_data<T>{std::move(result)};
    }
#endif
}}

#endif
