//   Copyright (c) 2017 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/insert.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/datastructures/optional.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const insert::match_data = {
        match_pattern_type{
            "insert",
            std::vector<std::string>{
                "insert(_1, _2, _3, __arg(_4_axis, nil), __arg(_5_dtype, nil))"},
            &create_insert, &create_primitive<insert>, R"(
            arr, obj, values, axis, dtype
            Args:

                arr (array_like) : input array
                obj (int, slice or sequence of ints) : Object that defines the
                    index or indices before which values is inserted.
                values (array_like) : Values to insert into arr.
                axis (int, optional) : Axis along which to insert values. If
                    axis is None then arr is flattened first.
                dtype (optional, string) : the data-type of the returned array,
                  defaults to 'float'.

            Returns:

            A copy of arr with values inserted.)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    insert::insert(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::size_t insert::get_vector_size(ir::node_data<T> const& arg)
    {
        switch (arg.num_dimensions())
        {
        case 0:
            return 1;

        case 1:
            return arg.size();

        case 2:
            return arg.dimension(1);

        case 3:
            return arg.dimension(2);

        default:
            break;
        }
        return 0;
    }

    template <typename T>
    primitive_argument_type insert::flatten_nd_helper(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        if (indices.num_dimensions() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_nd_helper",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        }

        if (arg.num_dimensions() == 0)
        {
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        }

        auto flattened_result = arg.vector();

        std::size_t size =
            (std::max)(get_vector_size(indices), get_vector_size(values));
        auto val =
            extract_value_vector<T>(std::move(values), size, name_, codename_);

        auto values_vec = val.vector();
        auto ind =
            extract_value_vector<std::int64_t>(indices, size, name_, codename_);
        auto indices_vec = ind.vector();
        std::transform(indices_vec.data(), indices_vec.data() + size,
            indices_vec.data(), [&](std::int64_t a) -> std::int64_t {
                if (a < 0)
                    a += flattened_result.size();
                return a;
            });

        blaze::DynamicVector<T> sorted_indices(indices_vec.size());

        std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
        std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
            [&](std::int64_t c, std::int64_t b) {
                return indices_vec[c] < indices_vec[b];
            });
        auto d = flattened_result.data();
        blaze::DynamicVector<T> result(flattened_result.size() + size);
        blaze::DynamicVector<std::int64_t> mask(result.size(), 1);

        for (std::size_t i = 0; i < size; ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.size();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >=
                    static_cast<std::int64_t>(result.size()))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_flatten_nd_helper",
                    generate_error_message("index out of bound"));
            }
            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            result[indices_vec[sorted_indices[i]]] = values_vec[sorted_indices[i]];
        }

        for (std::size_t i = 0; i < result.size(); ++i)
        {
            if (mask[i] == 1)
            {
                if (flattened_result.size() == 1)
                    result[i] = flattened_result[0];
                else
                    result[i] = *d++;
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type insert::insert_flatten_2d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        auto arr = arg.matrix();

        using phylanx::util::matrix_row_iterator;
        const matrix_row_iterator<decltype(arr)> arr_begin(arr);
        const matrix_row_iterator<decltype(arr)> arr_end(arr, arr.rows());
        blaze::DynamicVector<T> result_flattened(arr.rows() * arr.columns(), 0);

        auto d = result_flattened.data();

        for (auto it = arr_begin; it != arr_end; ++it)
            d = std::copy(it->begin(), it->end(), d);

        return flatten_nd_helper(ir::node_data<T>{std::move(result_flattened)},
            std::move(indices), std::move(values));
    }

    template <typename T>
    primitive_argument_type insert::insert_flatten_3d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        auto arr = arg.tensor();

        using phylanx::util::matrix_row_iterator;
        blaze::DynamicVector<T> result_flattened(
            arr.rows() * arr.columns() * arr.pages(), 0);
        auto d = result_flattened.data();

        for (std::size_t i = 0; i != arr.pages(); ++i)
        {
            auto slice = blaze::pageslice(arr, i);
            matrix_row_iterator<decltype(slice)> arr_begin(slice);
            matrix_row_iterator<decltype(slice)> arr_end(slice, slice.rows());

            for (auto it = arr_begin; it != arr_end; ++it)
                d = std::copy(it->begin(), it->end(), d);
        }

        return flatten_nd_helper(ir::node_data<T>{std::move(result_flattened)},
            std::move(indices), std::move(values));
    }

    template <typename T>
    primitive_argument_type insert::insert_flatten_nd(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        auto ndim = arg.num_dimensions();
        switch (ndim)
        {
        case 0: HPX_FALLTHROUGH;
        case 1:
            return flatten_nd_helper(
                std::move(arg), std::move(indices), std::move(values));

        case 2:
            return insert_flatten_2d(
                std::move(arg), std::move(indices), std::move(values));

        case 3:
            return insert_flatten_3d(
                std::move(arg), std::move(indices), std::move(values));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_flatten_nd",
            generate_error_message("index is out of bounds"));
    }

    template <typename T>
    primitive_argument_type insert::insert_1d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        std::int64_t axis) const
    {
        if (axis == 0 || axis == -1)
        {
            return flatten_nd_helper(
                std::move(arg), std::move(indices), std::move(values));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_1d",
            generate_error_message(
                "axis is out of bound for array of dimension 1"));
    }

    template <typename T>
    primitive_argument_type insert::insert_2d_axis0(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        if (indices.num_dimensions() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_2d_axis0",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        }
        if (arg.num_dimensions() == 0)
        {
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        }
        if (indices.num_dimensions() == 0)
        {
            indices = extract_value_vector<T>(indices, 1, name_, codename_);
        }

        auto m = arg.matrix();
        auto val = extract_value_vector<T>(
            std::move(values), m.columns(), name_, codename_);

        auto values_vec = val.vector();
        auto indices_vec = indices.vector();

        std::transform(indices_vec.data(),
            indices_vec.data() + indices_vec.size(), indices_vec.data(),
            [&](std::int64_t a) -> std::int64_t {
                if (a < 0)
                    a += m.rows();
                return a;
            });

        blaze::DynamicVector<T> sorted_indices(indices_vec.size());

        std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
        std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
            [&](std::int64_t c, std::int64_t b) {
                return indices_vec[c] < indices_vec[b];
            });

        blaze::DynamicMatrix<T> result(
            m.rows() + indices_vec.size(), m.columns());
        blaze::DynamicVector<std::int64_t> mask(
            m.rows() + indices_vec.size(), 1);

        for (std::size_t i = 0; i < indices_vec.size(); ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.rows();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >=
                    static_cast<std::int64_t>(result.rows()))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_2d_axis_0",
                    generate_error_message("index out of bound"));
            }
            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            blaze::row(result, indices_vec[sorted_indices[i]]) =
                blaze::trans(values_vec);
        }

        std::size_t j = 0;
        for (std::size_t i = 0; i < result.rows(); ++i)
        {
            if (mask[i] == 1)
                blaze::row(result, i) = blaze::row(m, j++);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type insert::insert_2d_axis1(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        if (indices.num_dimensions() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_2d_axis1",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        }
        if (arg.num_dimensions() == 0)
        {
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        }
        if (indices.num_dimensions() == 0)
        {
            indices = extract_value_vector<T>(indices, 1, name_, codename_);
        }

        auto m = arg.matrix();
        auto val = extract_value_vector<T>(
            std::move(values), indices.size(), name_, codename_);

        auto values_vec = val.vector();
        auto indices_vec = indices.vector();

        std::transform(indices_vec.data(),
            indices_vec.data() + indices_vec.size(), indices_vec.data(),
            [&](std::int64_t a) -> std::int64_t {
                if (a < 0)
                    a += m.columns();
                return a;
            });

        blaze::DynamicVector<T> sorted_indices(indices_vec.size());

        std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
        std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
            [&](std::int64_t c, std::int64_t b) {
                return indices_vec[c] < indices_vec[b];
            });

        blaze::DynamicMatrix<T> result(
            m.rows(), m.columns() + indices_vec.size());
        blaze::DynamicVector<std::int64_t> mask(
            m.columns() + indices_vec.size(), 1);

        for (std::size_t i = 0; i < indices_vec.size(); ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.columns();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >=
                    static_cast<std::int64_t>(result.columns()))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_2d_axis_1",
                    generate_error_message("index out of bound"));
            }

            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            blaze::column(result, indices_vec[sorted_indices[i]]) =
                values_vec[sorted_indices[i]];
        }

        std::size_t j = 0;
        for (std::size_t i = 0; i < result.columns(); ++i)
        {
            if (mask[i] == 1)
                blaze::column(result, i) = blaze::column(m, j++);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type insert::insert_2d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        std::int64_t axis) const
    {
        if (axis == 0 || axis == 2)
            return insert_2d_axis0(
                std::move(arg), std::move(indices), std::move(values));
        if (axis == 1 || axis == -1)
            return insert_2d_axis1(
                std::move(arg), std::move(indices), std::move(values));

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_2d",
            generate_error_message(
                "axis is out of bounds for array of dimension 2"));
    }

    template <typename T>
    primitive_argument_type insert::insert_3d_axis0(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        if (indices.num_dimensions() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_3d_axis0",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        }
        if (arg.num_dimensions() == 0)
        {
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        }
        if (indices.num_dimensions() == 0)
        {
            indices = extract_value_vector<T>(indices, 1, name_, codename_);
        }

        auto m = arg.tensor();
        auto val = extract_value_matrix<T>(
            std::move(values), m.rows(), m.columns(), name_, codename_);

        auto values_mat = val.matrix();
        auto indices_vec = indices.vector();

        std::transform(indices_vec.data(),
            indices_vec.data() + indices_vec.size(), indices_vec.data(),
            [&](std::int64_t a) -> std::int64_t {
                if (a < 0)
                    a += m.pages();
                return a;
            });

        blaze::DynamicVector<T> sorted_indices(indices_vec.size());

        std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
        std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
            [&](std::int64_t c, std::int64_t b) {
                return indices_vec[c] < indices_vec[b];
            });

        blaze::DynamicTensor<T> result(
            arg.dimension(0) + indices_vec.size(), m.rows(), m.columns());
        blaze::DynamicVector<std::int64_t> mask(
            arg.dimension(0) + indices_vec.size(), 1);

        for (std::size_t i = 0; i < indices_vec.size(); ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.pages();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >=
                    static_cast<std::int64_t>(result.pages()))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_3d_axis0",
                    generate_error_message("index out of bound"));
            }

            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            blaze::pageslice(result, indices_vec[sorted_indices[i]]) =
                values_mat;
        }
        std::size_t j = 0;
        for (std::size_t i = 0; i < result.pages(); ++i)
        {
            if (mask[i] == 1)
                blaze::pageslice(result, i) = blaze::pageslice(m, j++);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type insert::insert_3d_axis1(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        if (indices.num_dimensions() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_3d_axis1",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        }
        if (arg.num_dimensions() == 0)
        {
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        }
        if (indices.num_dimensions() == 0)
        {
            indices = extract_value_vector<T>(indices, 1, name_, codename_);
        }

        auto m = arg.tensor();
        auto val = extract_value_matrix<T>(
            std::move(values), m.pages(), indices.size(), name_, codename_);

        auto values_vec = val.matrix();
        auto indices_vec = indices.vector();

        std::transform(indices_vec.data(),
            indices_vec.data() + indices_vec.size(), indices_vec.data(),
            [&](std::int64_t a) -> std::int64_t {
                if (a < 0)
                    a += m.rows();
                return a;
            });

        blaze::DynamicVector<T> sorted_indices(indices_vec.size());

        std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
        std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
            [&](std::int64_t c, std::int64_t b) {
                return indices_vec[c] < indices_vec[b];
            });

        blaze::DynamicTensor<T> result(
            m.pages(), m.rows() + indices_vec.size(), m.columns());
        blaze::DynamicVector<std::int64_t> mask(
            m.rows() + indices_vec.size(), 1);

        for (std::size_t i = 0; i < indices_vec.size(); ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.rows();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >=
                    static_cast<std::int64_t>(result.rows()))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_3d_axis_1",
                    generate_error_message("index out of bound"));
            }

            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            blaze::rowslice(result, indices_vec[sorted_indices[i]]) =
                blaze::trans(values_vec);
        }

        std::size_t j = 0;
        for (std::size_t i = 0; i < result.rows(); ++i)
        {
            if (mask[i] == 1)
                blaze::rowslice(result, i) = blaze::rowslice(m, j++);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type insert::insert_3d_axis2(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        if (indices.num_dimensions() > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_3d_axis2",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        }

        if (arg.num_dimensions() == 0)
        {
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        }
        if (indices.num_dimensions() == 0)
        {
            indices = extract_value_vector<T>(indices, 1, name_, codename_);
        }

        auto m = arg.tensor();
        auto val = extract_value_vector<T>(
            std::move(values), indices.size(), name_, codename_);

        auto values_vec = val.vector();
        auto indices_vec = indices.vector();

        std::transform(indices_vec.data(),
            indices_vec.data() + indices_vec.size(), indices_vec.data(),
            [&](std::int64_t a) -> std::int64_t {
                if (a < 0)
                    a += m.columns();
                return a;
            });

        blaze::DynamicVector<T> sorted_indices(indices_vec.size());

        std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
        std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
            [&](std::int64_t c, std::int64_t b) {
                return indices_vec[c] < indices_vec[b];
            });

        blaze::DynamicTensor<T> result(
            m.pages(), m.rows(), m.columns() + indices_vec.size());
        blaze::DynamicVector<std::int64_t> mask(
            m.columns() + indices_vec.size(), 1);

        for (std::size_t i = 0; i < indices_vec.size(); ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.columns();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >=
                    static_cast<std::int64_t>(result.columns()))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_3d_axis_2",
                    generate_error_message("index out of bound"));
            }

            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            blaze::columnslice(result, indices_vec[sorted_indices[i]]) =
                values_vec[sorted_indices[i]];
        }

        std::size_t j = 0;
        for (std::size_t i = 0; i < result.columns(); ++i)
        {
            if (mask[i] == 1)
                blaze::columnslice(result, i) = blaze::columnslice(m, j++);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type insert::insert_3d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        std::int64_t axis) const
    {
        if (axis == 0 || axis == -3)
        {
            return insert_3d_axis0(
                std::move(arg), std::move(indices), std::move(values));
        }
        if (axis == 1 || axis == -2)
        {
            return insert_3d_axis1(
                std::move(arg), std::move(indices), std::move(values));
        }
        if (axis == 2 || axis == -1)
        {
            return insert_3d_axis2(
                std::move(arg), std::move(indices), std::move(values));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_flatten_3d",
            generate_error_message(
                "axis is out of bounds for array of dimension 3"));
    }

    template <typename T>
    primitive_argument_type insert::insert_nd(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (!axis)
        {
            return insert_flatten_nd(
                std::move(arg), std::move(indices), std::move(values));
        }

        switch (arg.num_dimensions())
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_nd",
                generate_error_message("assignment to 0-d array"));

        case 1:
            return insert_1d(std::move(arg), std::move(indices),
                std::move(values), axis.value());

        case 2:
            return insert_2d(std::move(arg), std::move(indices),
                std::move(values), axis.value());

        case 3:
            return insert_3d(std::move(arg), std::move(indices),
                std::move(values), axis.value());

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_flatten_0d",
            generate_error_message("index is out of bounds"));
    }

    hpx::future<primitive_argument_type> insert::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || (operands.size() < 3 || operands.size() > 5))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::eval",
                generate_error_message(
                    "the insert primitive requires three to five operands"));
        }

        if ((!valid(operands[0]) || !valid(operands[1])) || !valid(operands[2]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::eval",
                generate_error_message(
                    "the insert primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                hpx::util::optional<std::int64_t> axis;
                node_data_type dtype = extract_common_type(args[0]);

                if (args.size() >= 4 && valid(args[3]))
                {
                    axis = extract_scalar_integer_value_strict(
                        args[3], this_->name_, this_->codename_);
                }

                if (args.size() == 5 && valid(args[4]))
                {
                    dtype = map_dtype(extract_string_value_strict(
                        args[3], this_->name_, this_->codename_));
                }

                switch (dtype)
                {
                case node_data_type_bool:
                    return this_->insert_nd(
                        extract_boolean_value(std::move(args[0]),
                            this_->name_, this_->codename_),
                        extract_integer_value_strict(std::move(args[1]),
                            this_->name_, this_->codename_),
                        extract_boolean_value(std::move(args[2]),
                            this_->name_, this_->codename_),
                        axis);

                case node_data_type_int64:
                    return this_->insert_nd(
                        extract_integer_value(std::move(args[0]),
                            this_->name_, this_->codename_),
                        extract_integer_value_strict(std::move(args[1]),
                            this_->name_, this_->codename_),
                        extract_integer_value(std::move(args[2]),
                            this_->name_, this_->codename_),
                        axis);

                case node_data_type_unknown: HPX_FALLTHROUGH;
                case node_data_type_double:
                    return this_->insert_nd(
                        extract_numeric_value(std::move(args[0]),
                            this_->name_, this_->codename_),
                        extract_integer_value_strict(std::move(args[1]),
                            this_->name_, this_->codename_),
                        extract_numeric_value(std::move(args[2]),
                            this_->name_, this_->codename_),
                        axis);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "insert::eval",
                        this_->generate_error_message(
                            "the insert primitive requires for all "
                            "arguments "
                            "to be numeric data types"));
                }
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
