//  Copyright (c) 2020 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/indices.hpp>

#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type indices0d(ir::range const& shape,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices0d",
                util::generate_error_message(
                    "the shape argument should be zero dimensional", name,
                    codename, ctx.back_trace()));
        }

        return primitive_argument_type(primitive_arguments_type());
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type indices1d_helper(std::size_t size)
    {
        blaze::DynamicVector<T> indices(size);
        std::iota(indices.begin(), indices.end(), std::size_t(0));

        execution_tree::primitive_arguments_type result;
        result.reserve(1);
        result.emplace_back(std::move(indices));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type indices1d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices1d",
                util::generate_error_message(
                    "the shape argument should be one dimensional", name,
                    codename, ctx.back_trace()));
        }

        std::size_t size = extract_scalar_positive_integer_value_strict(
            *shape.begin(), name, codename);
        switch (dtype)
        {
        case node_data_type_int64:
            return indices1d_helper<std::int64_t>(size);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return indices1d_helper<double>(size);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "phylanx::common::indices1d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type indices2d_helper(
        std::size_t num_rows, std::size_t num_columns)
    {
        blaze::DynamicMatrix<T> rows(num_rows, num_columns);
        blaze::DynamicMatrix<T> columns(num_rows, num_columns);

        for (std::size_t i = 0; i != num_rows; ++i)
        {
            for (std::size_t j = 0; j != num_columns; ++j)
            {
                rows(i, j) = i;
                columns(i, j) = j;
            }
        }

        execution_tree::primitive_arguments_type result;
        result.reserve(2);
        result.emplace_back(std::move(rows));
        result.emplace_back(std::move(columns));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type indices2d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices2d",
                util::generate_error_message(
                    "the shape argument should be two dimensional", name,
                    codename, ctx.back_trace()));
        }

        auto it = shape.begin();
        std::size_t rows =
            extract_scalar_positive_integer_value_strict(*it, name, codename);
        std::size_t columns =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);

        switch (dtype)
        {
        case node_data_type_int64:
            return indices2d_helper<std::int64_t>(rows, columns);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return indices2d_helper<double>(rows, columns);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "phylanx::common::indices2d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type indices3d_helper(
        std::size_t num_pages, std::size_t num_rows, std::size_t num_columns)
    {
        blaze::DynamicTensor<T> pages(num_pages, num_rows, num_columns);
        blaze::DynamicTensor<T> rows(num_pages, num_rows, num_columns);
        blaze::DynamicTensor<T> columns(num_pages, num_rows, num_columns);

        for (std::size_t i = 0; i != num_pages; ++i)
        {
            for (std::size_t j = 0; j != num_rows; ++j)
            {
                for (std::size_t k = 0; k != num_columns; ++k)
                {
                    pages(i, j, k) = i;
                    rows(i, j, k) = j;
                    columns(i, j, k) = k;
                }
            }
        }

        execution_tree::primitive_arguments_type result;
        result.reserve(3);
        result.emplace_back(std::move(pages));
        result.emplace_back(std::move(rows));
        result.emplace_back(std::move(columns));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type indices3d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices3d",
                util::generate_error_message(
                    "the shape argument should be three dimensional", name,
                    codename, ctx.back_trace()));
        }

        auto it = shape.begin();
        std::size_t pages =
            extract_scalar_positive_integer_value_strict(*it, name, codename);
        std::size_t rows =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);
        std::size_t columns =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);

        switch (dtype)
        {
        case node_data_type_int64:
            return indices3d_helper<std::int64_t>(pages, rows, columns);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return indices3d_helper<double>(pages, rows, columns);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "phylanx::common::indices3d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type indices4d_helper(
        std::size_t num_quats, std::size_t num_pages, std::size_t num_rows,
        std::size_t num_columns)
    {
        blaze::DynamicArray<4, T> quats(
            num_quats, num_pages, num_rows, num_columns);
        blaze::DynamicArray<4, T> pages(
            num_quats, num_pages, num_rows, num_columns);
        blaze::DynamicArray<4, T> rows(
            num_quats, num_pages, num_rows, num_columns);
        blaze::DynamicArray<4, T> columns(
            num_quats, num_pages, num_rows, num_columns);

        for (std::size_t i = 0; i != num_quats; ++i)
        {
            for (std::size_t j = 0; j != num_pages; ++j)
            {
                for (std::size_t k = 0; k != num_rows; ++k)
                {
                    for (std::size_t l = 0; l != num_columns; ++l)
                    {
                        quats(i, j, k, l) = i;
                        pages(i, j, k, l) = j;
                        rows(i, j, k, l) = k;
                        columns(i, j, k, l) = l;
                    }
                }
            }
        }

        execution_tree::primitive_arguments_type result;
        result.reserve(4);
        result.emplace_back(std::move(quats));
        result.emplace_back(std::move(pages));
        result.emplace_back(std::move(rows));
        result.emplace_back(std::move(columns));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type indices4d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices4d",
                util::generate_error_message(
                    "the shape argument should be four dimensional", name,
                    codename, ctx.back_trace()));
        }

        auto it = shape.begin();
        std::size_t quats =
            extract_scalar_positive_integer_value_strict(*it, name, codename);
        std::size_t pages =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);
        std::size_t rows =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);
        std::size_t columns =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);

        switch (dtype)
        {
        case node_data_type_int64:
            return indices4d_helper<std::int64_t>(quats, pages, rows, columns);

        case node_data_type_unknown:
            [[fallthrough]]
        case node_data_type_double:
            return indices4d_helper<double>(quats, pages, rows, columns);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "phylanx::common::indices4d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type generate_indices(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        switch (shape.size())
        {
        case 0:
            return indices0d(shape, name, codename, std::move(ctx));

        case 1:
            return indices1d(shape, dtype, name, codename, std::move(ctx));

        case 2:
            return indices2d(shape, dtype, name, codename, std::move(ctx));

        case 3:
            return indices3d(shape, dtype, name, codename, std::move(ctx));

        case 4:
            return indices4d(shape, dtype, name, codename, std::move(ctx));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::generate_indices",
            util::generate_error_message(
                "unsupported dimensionality of shape argument", name, codename,
                ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type sparse_indices0d(
        ir::range const& shape, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices0d",
                util::generate_error_message(
                    "the shape argument should be zero dimensional", name,
                    codename, ctx.back_trace()));
        }

        return primitive_argument_type(primitive_arguments_type());
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type sparse_indices1d_helper(
        std::size_t size)
    {
        blaze::DynamicVector<T> indices(size);
        std::iota(indices.begin(), indices.end(), std::size_t(0));

        execution_tree::primitive_arguments_type result;
        result.reserve(1);
        result.emplace_back(std::move(indices));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type sparse_indices1d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::indices1d",
                util::generate_error_message(
                    "the shape argument should be one dimensional", name,
                    codename, ctx.back_trace()));
        }

        std::size_t size = extract_scalar_positive_integer_value_strict(
            *shape.begin(), name, codename);
        switch (dtype)
        {
        case node_data_type_int64:
            return sparse_indices1d_helper<std::int64_t>(size);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return sparse_indices1d_helper<double>(size);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::common::sparse_indices1d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type sparse_indices2d_helper(
        std::size_t num_rows, std::size_t num_columns)
    {
        blaze::DynamicMatrix<T> rows(num_rows, 1);
        auto column = blaze::column(rows, 0);
        std::iota(column.begin(), column.end(), std::size_t(0));

        blaze::DynamicMatrix<T> columns(1, num_columns);
        auto row = blaze::row(columns, 0);
        std::iota(row.begin(), row.end(), std::size_t(0));

        execution_tree::primitive_arguments_type result;
        result.reserve(2);
        result.emplace_back(std::move(rows));
        result.emplace_back(std::move(columns));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type sparse_indices2d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::sparse_indices2d",
                util::generate_error_message(
                    "the shape argument should be two dimensional", name,
                    codename, ctx.back_trace()));
        }

        auto it = shape.begin();
        std::size_t rows =
            extract_scalar_positive_integer_value_strict(*it, name, codename);
        std::size_t columns =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);

        switch (dtype)
        {
        case node_data_type_int64:
            return sparse_indices2d_helper<std::int64_t>(rows, columns);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return sparse_indices2d_helper<double>(rows, columns);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::common::sparse_indices2d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type sparse_indices3d_helper(
        std::size_t num_pages, std::size_t num_rows, std::size_t num_columns)
    {
        blaze::DynamicTensor<T> pages(num_pages, 1, 1);
        auto page = blaze::row(blaze::rowslice(pages, 0), 0);
        std::iota(page.begin(), page.end(), std::size_t(0));

        blaze::DynamicTensor<T> rows(1, num_rows, 1);
        auto column = blaze::row(blaze::columnslice(rows, 0), 0);
        std::iota(column.begin(), column.end(), std::size_t(0));

        blaze::DynamicTensor<T> columns(1, 1, num_columns);
        auto row = blaze::row(blaze::pageslice(columns, 0), 0);
        std::iota(row.begin(), row.end(), std::size_t(0));

        execution_tree::primitive_arguments_type result;
        result.reserve(3);
        result.emplace_back(std::move(pages));
        result.emplace_back(std::move(rows));
        result.emplace_back(std::move(columns));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type sparse_indices3d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::sparse_indices3d",
                util::generate_error_message(
                    "the shape argument should be three dimensional", name,
                    codename, ctx.back_trace()));
        }

        auto it = shape.begin();
        std::size_t pages =
            extract_scalar_positive_integer_value_strict(*it, name, codename);
        std::size_t rows =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);
        std::size_t columns =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);

        switch (dtype)
        {
        case node_data_type_int64:
            return sparse_indices3d_helper<std::int64_t>(pages, rows, columns);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return sparse_indices3d_helper<double>(pages, rows, columns);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::common::sparse_indices3d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type sparse_indices4d_helper(
        std::size_t num_quats, std::size_t num_pages, std::size_t num_rows,
        std::size_t num_columns)
    {
        blaze::DynamicArray<4, T> quats(num_quats, 1, 1, 1);
        for (std::size_t i = 0; i != num_quats; ++i)
        {
            quats(i, 0, 0, 0) = i;
        }

        blaze::DynamicArray<4, T> pages(1, num_pages, 1, 1);
        auto page =
            blaze::row(blaze::rowslice(blaze::quatslice(pages, 0), 0), 0);
        std::iota(page.begin(), page.end(), std::size_t(0));

        blaze::DynamicArray<4, T> rows(1, 1, num_rows, 1);
        auto column =
            blaze::row(blaze::columnslice(blaze::quatslice(rows, 0), 0), 0);
        std::iota(column.begin(), column.end(), std::size_t(0));

        blaze::DynamicArray<4, T> columns(1, 1, 1, num_columns);
        auto row =
            blaze::row(blaze::pageslice(blaze::quatslice(columns, 0), 0), 0);
        std::iota(row.begin(), row.end(), std::size_t(0));

        execution_tree::primitive_arguments_type result;
        result.reserve(4);
        result.emplace_back(std::move(quats));
        result.emplace_back(std::move(pages));
        result.emplace_back(std::move(rows));
        result.emplace_back(std::move(columns));

        return execution_tree::primitive_argument_type(std::move(result));
    }

    execution_tree::primitive_argument_type sparse_indices4d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        using namespace phylanx::execution_tree;

        if (shape.size() != 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::common::sparse_indices4d",
                util::generate_error_message(
                    "the shape argument should be four dimensional", name,
                    codename, ctx.back_trace()));
        }

        auto it = shape.begin();
        std::size_t quats =
            extract_scalar_positive_integer_value_strict(*it, name, codename);
        std::size_t pages =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);
        std::size_t rows =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);
        std::size_t columns =
            extract_scalar_positive_integer_value_strict(*++it, name, codename);

        switch (dtype)
        {
        case node_data_type_int64:
            return sparse_indices4d_helper<std::int64_t>(
                quats, pages, rows, columns);

        case node_data_type_unknown:
            [[fallthrough]];
        case node_data_type_double:
            return sparse_indices4d_helper<double>(quats, pages, rows, columns);

        case node_data_type_bool:
            [[fallthrough]];
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::common::sparse_indices4d",
            util::generate_error_message(
                "the indices primitive requires for the dtype argument to "
                "be valid",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type generate_sparse_indices(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx)
    {
        switch (shape.size())
        {
        case 0:
            return sparse_indices0d(shape, name, codename, std::move(ctx));

        case 1:
            return sparse_indices1d(
                shape, dtype, name, codename, std::move(ctx));

        case 2:
            return sparse_indices2d(
                shape, dtype, name, codename, std::move(ctx));

        case 3:
            return sparse_indices3d(
                shape, dtype, name, codename, std::move(ctx));

        case 4:
            return sparse_indices4d(
                shape, dtype, name, codename, std::move(ctx));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "common::generate_sparse_indices",
            util::generate_error_message(
                "unsupported dimensionality of shape argument", name, codename,
                ctx.back_trace()));
    }

}}    // namespace phylanx::common
