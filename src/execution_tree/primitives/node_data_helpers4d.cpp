// Copyright (c) 2017-2019 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/errors/throw_exception.hpp>

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
    // return argument as a quaterns, tensors, matrices, vectors and scalars
    // are properly broadcast
    template <typename T>
    PHYLANX_EXPORT void extract_value_quatern(
        typename ir::node_data<T>::storage4d_type& result,
        ir::node_data<T>&& rhs, std::size_t quats, std::size_t pages,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        using storage4d_type = typename ir::node_data<T>::storage4d_type;

        switch (rhs.num_dimensions())
        {
        case 0:
            {
                result.resize(
                    std::array<std::size_t, 4>{columns, rows, pages, quats});
                result = rhs.scalar();
            }
            return;

        case 1:
            {
                // vectors of size one can be broadcast into any quatern
                if (rhs.size() == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    result = rhs[0];
                    return;
                }

                if (columns != rhs.size())
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "cannot broadcast a vector into a quatern with a "
                            "different number of columns",
                            name, codename));
                }

                result.resize(
                    std::array<std::size_t, 4>{columns, rows, pages, quats});

                auto v = rhs.vector();
                auto row = blaze::trans(v);
                for (std::size_t l = 0; l != quats; ++l)
                {
                    auto quat = blaze::quatslice(result, l);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        auto page = blaze::pageslice(quat, k);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            blaze::row(page, i) = row;
                        }
                    }
                }
                return;
            }

        case 2:
            {
                // matrices of size one can be broadcast into any quatern
                if (rhs.size() == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    result = rhs[0];
                    return;
                }

                // matrices with one row can be broadcast into any
                // quatern with the same number of columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});

                    auto m = rhs.matrix();
                    auto row = blaze::row(m, 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            auto page = blaze::pageslice(quat, k);
                            for (std::size_t i = 0; i != rows; ++i)
                            {
                                blaze::row(page, i) = row;
                            }
                        }
                    }
                    return;
                }

                // matrices with one column can be broadcast into any
                // quatern with the same number of rows
                if (rhs.dimension(0) == rows && rhs.dimension(1) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});

                    auto m = rhs.matrix();
                    auto column = blaze::column(m, 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            auto page = blaze::pageslice(quat, k);
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                blaze::column(page, j) = column;
                            }
                        }
                    }
                    return;
                }

                if (rhs.dimension(0) != rows || rhs.dimension(1) != columns)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "cannot broadcast a matrix into a quatern with "
                                "a different number of rows or columns",
                            name, codename));
                }

                result.resize(
                    std::array<std::size_t, 4>{columns, rows, pages, quats});
                for (std::size_t l = 0; l != quats; ++l)
                {
                    auto quat = blaze::quatslice(result, l);
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        blaze::pageslice(quat, k) = rhs.matrix();
                    }
                }
                return;
            }

        case 3:
            {
                // tensors of size one can be broadcast into any quatern
                if (rhs.size() == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});

                    result = rhs.at(0, 0, 0);
                    return;
                }

                // tensors with just one page and row can be broadcast into any
                // quatern with the same number of columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto t = rhs.tensor();

                    auto row = blaze::row(blaze::pageslice(t, 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            auto page = blaze::pageslice(quat, k);
                            for (std::size_t i = 0; i != rows; ++i)
                            {
                                blaze::row(page, i) = row;
                            }
                        }
                    }
                    return;
                }

                // tensors with just one page and column can be broadcast into
                // quatern with the same number of rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == rows &&
                    rhs.dimension(2) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto t = rhs.tensor();

                    auto col = blaze::column(blaze::pageslice(t, 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            auto page = blaze::pageslice(quat, k);
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                blaze::column(page, j) = col;
                            }
                        }
                    }
                    return;
                }

                // tensors with just one row and column can be broadcast into
                // any quatern with the same number of pages
                if (rhs.dimension(0) == pages && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto t = rhs.tensor();

                    auto row = blaze::row(blaze::rowslice(t, 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            auto slice = blaze::rowslice(quat, i);
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                blaze::row(slice, j) = row;
                            }
                        }
                    }
                    return;
                }

                // tensors with just one page can be broadcast into any
                // quatern with the same number of columns/rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == rows &&
                    rhs.dimension(2) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto t = rhs.tensor();

                    auto rhs_page = blaze::pageslice(t, 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            blaze::pageslice(quat, k) = rhs_page;
                        }
                    }
                    return;
                }

                // tensors with just one row can be broadcast into any
                // quatern with the same number of pages/columns
                if (rhs.dimension(0) == pages && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto t = rhs.tensor();

                    auto rhs_rowslice = blaze::rowslice(t, 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            blaze::rowslice(quat, i) = rhs_rowslice;
                        }
                    }
                    return;
                }

                // tensors with just one column can be broadcast into any
                // quatern with the same number of pages/rows
                if (rhs.dimension(0) == pages && rhs.dimension(1) == rows &&
                    rhs.dimension(2) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto t = rhs.tensor();

                    auto rhs_columnslice = blaze::columnslice(t, 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            blaze::columnslice(quat, j) = rhs_columnslice;
                        }
                    }
                    return;
                }

                if (rhs.dimension(0) != pages || rhs.dimension(1) != rows ||
                    rhs.dimension(2) != columns)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "cannot broadcast a tensor into a quatern with "
                            "different pages, rows or columns",
                            name, codename));
                }

                result.resize(
                    std::array<std::size_t, 4>{columns, rows, pages, quats});
                for (std::size_t l = 0; l != quats; ++l)
                {
                    blaze::quatslice(result, l) = rhs.tensor();
                }
                return;
            }

        case 4:
            {
                // quaterns of size one can be broadcast into any quatern
                if (rhs.size() == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});

                    result = rhs.at(0, 0, 0, 0);
                    return;
                }

                // quaterns with just one quat, page and row can be broadcast
                // into any quatern with the same number of columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto row = blaze::row(
                        blaze::pageslice(blaze::quatslice(q, 0), 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            auto page = blaze::pageslice(quat, k);
                            for (std::size_t i = 0; i != rows; ++i)
                            {
                                blaze::row(page, i) = row;
                            }
                        }
                    }
                    return;
                }

                // quaterns with just one quat, page and column can be broadcast
                // into any quatern with the same number of rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto col = blaze::column(
                        blaze::pageslice(blaze::quatslice(q, 0), 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            auto page = blaze::pageslice(quat, k);
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                blaze::column(page, j) = col;
                            }
                        }
                    }
                    return;
                }

                // quaterns with just one quat, row and column can be broadcast
                // into any quatern with the same number of pages
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto row = blaze::row(
                        blaze::rowslice(blaze::quatslice(q, 0), 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            auto slice = blaze::rowslice(quat, i);
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                blaze::row(slice, j) = row;
                            }
                        }
                    }
                    return;
                }

                // quaterns with just one page, row and column can be broadcast
                // into any quatern with the same number of quats
                if (rhs.dimension(0) == quats && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == 1)
                {
//                     result.resize(std::array<std::size_t, 4>{
//                         columns, rows, pages, quats});
//                     auto q = rhs.quatern();

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "not implemented error. broadcasting (quats,1,1,1)",
                            name, codename));
                }

                // quaterns with just one page and row can be broadcast
                // into any quatern with the same number of quats/columns
                if (rhs.dimension(0) == quats && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == columns)
                {
//                     result.resize(std::array<std::size_t, 4>{
//                         columns, rows, pages, quats});
//                     auto q = rhs.quatern();

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "not implemented error. broadcasting "
                            "(quats,1,1,columns)",
                            name, codename));
                }

                // quaterns with just one page and column can be broadcast
                // into any quatern with the same number of quats/row
                if (rhs.dimension(0) == quats && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == 1)
                {
//                     result.resize(std::array<std::size_t, 4>{
//                         columns, rows, pages, quats});
//                     auto q = rhs.quatern();

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "not implemented error. broadcasting "
                            "(quats,1,rows,1)",
                            name, codename));
                }

                // quaterns with just one row and column can be broadcast
                // into any quatern with the same number of quats/pages
                if (rhs.dimension(0) == quats && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == 1)
                {
//                     result.resize(std::array<std::size_t, 4>{
//                         columns, rows, pages, quats});
//                     auto q = rhs.quatern();

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "not implemented error. broadcasting "
                            "(quats,pages,1,1)",
                            name, codename));
                }

                // quaterns with just one quat and page can be broadcast into
                // any quatern with the same number of columns/rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto rhs_page = blaze::pageslice(blaze::quatslice(q, 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            blaze::pageslice(quat, k) = rhs_page;
                        }
                    }
                    return;
                }

                // quaterns with just one quat and row can be broadcast into any
                // quatern with the same number of pages/columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto rhs_rowslice =
                        blaze::rowslice(blaze::quatslice(q, 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            blaze::rowslice(quat, i) = rhs_rowslice;
                        }
                    }
                    return;
                }

                // quaterns with just one quat and column can be broadcast into
                // any quatern with the same number of pages/rows
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto rhs_columnslice =
                        blaze::columnslice(blaze::quatslice(q, 0), 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            blaze::columnslice(quat, j) = rhs_columnslice;
                        }
                    }
                    return;
                }

                // quaterns with just one quat can be broadcast into
                // any quatern with the same number of pages/rows/columns
                if (rhs.dimension(0) == 1 && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    auto rhs_tensor = blaze::quatslice(q, 0);
                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        blaze::quatslice(result, l) = rhs_tensor;
                    }
                    return;
                }

                // quaterns with just one page can be broadcast into
                // any quatern with the same number of quats/rows/columns
                if (rhs.dimension(0) == quats && rhs.dimension(1) == 1 &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        auto rhs_slice =
                            blaze::pageslice(blaze::quatslice(q, l), 0);
                        for (std::size_t k = 0; k != pages; ++k)
                        {
                            blaze::pageslice(quat, k) = rhs_slice;
                        }
                    }
                    return;
                }

                // quaterns with just one row can be broadcast into
                // any quatern with the same number of quats/pages/columns
                if (rhs.dimension(0) == quats && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == 1 && rhs.dimension(3) == columns)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        auto rhs_slice =
                            blaze::rowslice(blaze::quatslice(q, l), 0);
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            blaze::rowslice(quat, i) = rhs_slice;
                        }
                    }
                    return;
                }

                // quaterns with just one column can be broadcast into
                // any quatern with the same number of quats/pages/rows
                if (rhs.dimension(0) == quats && rhs.dimension(1) == pages &&
                    rhs.dimension(2) == rows && rhs.dimension(3) == 1)
                {
                    result.resize(std::array<std::size_t, 4>{
                        columns, rows, pages, quats});
                    auto q = rhs.quatern();

                    for (std::size_t l = 0; l != quats; ++l)
                    {
                        auto quat = blaze::quatslice(result, l);
                        auto rhs_slice =
                            blaze::columnslice(blaze::quatslice(q, l), 0);
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            blaze::columnslice(quat, j) = rhs_slice;
                        }
                    }
                    return;
                }

                if (rhs.dimension(0) != quats || rhs.dimension(1) != pages ||
                    rhs.dimension(2) != rows || rhs.dimension(3) != columns)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::extract_value_quatern",
                        util::generate_error_message(
                            "cannot broadcast a quatern into a quatern with "
                            "different quats, pages, rows or columns",
                            name, codename));
                }

                if (rhs.is_ref())
                {
                    result = rhs.quatern();
                }
                else
                {
                    result = std::move(rhs.quatern_non_ref());
                }
                return;
            }


        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::extract_value_quatern",
            util::generate_error_message(
                "primitive_argument_type does not hold a numeric "
                    "value type",
                name, codename));
    }

    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> extract_value_quatern(
        ir::node_data<T>&& arg, std::size_t quats, std::size_t pages,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        typename ir::node_data<T>::storage4d_type result;
        extract_value_quatern(result, std::move(arg), quats, pages, rows,
            columns, name, codename);
        return ir::node_data<T>{std::move(result)};
    }

    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> extract_value_quatern(
        primitive_argument_type const& val, std::size_t quats,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename)
    {
        return extract_value_quatern(
            extract_node_data<T>(val, name, codename),
            quats, pages, rows, columns, name, codename);
    }

    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> extract_value_quatern(
        primitive_argument_type&& val, std::size_t quats, std::size_t pages,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename)
    {
        return extract_value_quatern(
            extract_node_data<T>(std::move(val), name, codename), quats, pages,
            rows, columns, name, codename);
    }

    template PHYLANX_EXPORT ir::node_data<double> extract_value_quatern<double>(
        primitive_argument_type const& val, std::size_t quats,
        std::size_t pages, std::size_t rows, std::size_t columns,
        std::string const& name, std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_quatern<std::int64_t>(primitive_argument_type const& val,
        std::size_t quats, std::size_t pages, std::size_t rows,
        std::size_t columns, std::string const& name,
        std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_quatern<std::uint8_t>(primitive_argument_type const& val,
        std::size_t quats, std::size_t pages, std::size_t rows,
        std::size_t columns, std::string const& name,
        std::string const& codename);

    template PHYLANX_EXPORT ir::node_data<double> extract_value_quatern<double>(
        primitive_argument_type&& val, std::size_t quats, std::size_t pages,
        std::size_t rows, std::size_t columns, std::string const& name,
        std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::int64_t>
    extract_value_quatern<std::int64_t>(primitive_argument_type&& val,
        std::size_t quats, std::size_t pages, std::size_t rows,
        std::size_t columns, std::string const& name,
        std::string const& codename);
    template PHYLANX_EXPORT ir::node_data<std::uint8_t>
    extract_value_quatern<std::uint8_t>(primitive_argument_type&& val,
        std::size_t quats, std::size_t pages, std::size_t rows,
        std::size_t columns, std::string const& name,
        std::string const& codename);
}}

