// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
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
    // return argument as a matrix, scalars and vectors are properly broadcast
    template <typename T>
    void extract_value_tensor(typename ir::node_data<T>::storage3d_type& result,
        ir::node_data<T>&& rhs,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        using storage3d_type = typename ir::node_data<T>::storage3d_type;

        switch (rhs.num_dimensions())
        {
        case 0:
            {
                result.resize(pages, rows, columns);
                result = rhs.scalar();
            }
            return;

        case 1:
            {
                // vectors of size one can be broadcast into any tensor
                if (rhs.size() == 1)
                {
                    result.resize(pages, rows, columns);
                    result = rhs[0];
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

                result.resize(pages, rows, columns);

                auto row = blaze::trans(rhs.vector());
                for (std::size_t k = 0; k != pages; ++k)
                {
                    auto page = blaze::pageslice(result, k);
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        blaze::row(page, i) = row;
                    }
                }
                return;
            }

        case 2:
            {
                // matrices of size one can be broadcast into any other tensor
                if (rhs.size() == 1)
                {
                    result.resize(pages, rows, columns);
                    result = rhs[0];
                    return;
                }

                // matrices with one row can be broadcast into any other
                // matrix with the same number of columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == columns)
                {
                    result.resize(pages, rows, columns);

                    auto m = rhs.matrix();
                    auto row = blaze::row(m, 0);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        auto page = blaze::pageslice(result, k);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            blaze::row(page, i) = row;
                        }
                    }
                    return;
                }

                // matrices with one column can be broadcast into any other
                // matrix with the same number of rows
                if (rhs.dimension(0) == rows && rhs.dimension(1) == 1)
                {
                    result.resize(pages, rows, columns);

                    auto m = rhs.matrix();
                    auto column = blaze::column(m, 0);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        auto page = blaze::pageslice(result, k);
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            blaze::column(page, j) = column;
                        }
                    }
                    return;
                }

                if (rhs.dimension(0) != rows || rhs.dimension(1) != columns)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_tensor",
                        util::generate_error_message(
                            "cannot broadcast a matrix into a differently "
                                "sized tensor",
                            name, codename));
                }

                result.resize(pages, rows, columns);
                for (std::size_t k = 0; k != pages; ++k)
                {
                    blaze::pageslice(result, k) = rhs.matrix();
                }
                return;
            }

        case 3:
            {
                // tensors of size one can be broadcast into any other tensor
                if (rhs.size() == 1)
                {
                    result.resize(pages, rows, columns);

                    result = rhs.at(0, 0, 0);
                    return;
                }

                // tensors with just one row can be broadcast into any other
                // tensor with the same number of columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == columns)
                {
                    result.resize(pages, rows, columns);
                    auto t = rhs.tensor();

                    auto row = blaze::row(blaze::pageslice(t, 0), 0);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        auto page = blaze::pageslice(result, k);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            blaze::row(page, i) = row;
                        }
                    }
                    return;
                }

                // tensors with just one column can be broadcast into any other
                // tensor with the same number of rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == rows &&
                    rhs.dimension(2) == 1)
                {
                    result.resize(pages, rows, columns);
                    auto t = rhs.tensor();

                    auto column = blaze::column(blaze::pageslice(t, 0), 0);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        auto page = blaze::pageslice(result, k);
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            blaze::column(page, j) = column;
                        }
                    }
                    return;
                }

                // tensors with just one page can be broadcast into any other
                // tensor with the same number of columns/rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == rows &&
                    rhs.dimension(2) == columns)
                {
                    result.resize(pages, rows, columns);
                    auto t = rhs.tensor();

                    auto rhs_page = blaze::pageslice(t, 0);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        blaze::pageslice(result, k) = rhs_page;
                    }
                    return;
                }

                if (rhs.dimension(0) != pages || rhs.dimension(1) != rows ||
                    rhs.dimension(2) != columns)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_tensor",
                        util::generate_error_message(
                            "cannot broadcast a tensor into a differently "
                                "sized tensor",
                            name, codename));
                }

                if (rhs.is_ref())
                {
                    result = rhs.tensor();
                }
                else
                {
                    result = std::move(rhs.tensor_non_ref());
                }
                return;
            }

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

    template <typename T>
    ir::node_data<T> extract_value_tensor(ir::node_data<T>&& arg,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        typename ir::node_data<T>::storage3d_type result;
        extract_value_tensor(
            result, std::move(arg), pages, rows, columns, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    template <typename T>
    ir::node_data<T> extract_value_tensor(primitive_argument_type const& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        return extract_value_tensor(
            extract_node_data<T>(val, name, codename),
            pages, rows, columns, name, codename);
    }

    template <typename T>
    ir::node_data<T> extract_value_tensor(primitive_argument_type && val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        return extract_value_tensor(
            extract_node_data<T>(std::move(val), name, codename),
            pages, rows, columns, name, codename);
    }

    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_tensor<double>( primitive_argument_type const& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_tensor<std::int64_t>(primitive_argument_type const& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_tensor<std::uint8_t>(primitive_argument_type const& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double>
    extract_value_tensor<double>(primitive_argument_type&& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_tensor<std::int64_t>(primitive_argument_type&& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_tensor<std::uint8_t>(primitive_argument_type&& val,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
}}

#endif
