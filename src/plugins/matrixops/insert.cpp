//   Copyright (c) 2017 Shahrzad Shirzad
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/insert.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif
///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const insert::match_data = {match_pattern_type{"insert",
        std::vector<std::string>{
            "insert(_1, _2, _3)", "insert(_1, _2, _3, _4)"},
        &create_insert, &create_primitive<insert>, R"(
            arr, obj, values, axis
            Args:

                arr (array_like) : input array
                obj (int, slice or sequence of ints) : Object that defines the index or
                                                       indices before which values is
                                                       inserted.
                values (array_like) : Values to insert into arr.
                axis (int, optional) : Axis along which to insert values. If axis is
                                       None then arr is flattened first.

            Returns:

            A copy of arr with values inserted.
            )",
        true}};

    ///////////////////////////////////////////////////////////////////////////
    insert::insert(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return arg.dimension(2);
#endif
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
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::insert_flatten_nd_helper",
                generate_error_message("index array argument to insert must be "
                                       "one dimensional or scalar"));
        if (arg.num_dimensions() == 0)
            arg = extract_value_vector<T>(arg, 1, name_, codename_);
        auto flattened_result = arg.vector();

        std::size_t size = (std::max)(get_vector_size(indices), get_vector_size(values));
        auto val = extract_value_vector<T>(values, size, name_, codename_);
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

        for (int i = 0; i < size; ++i)
        {
            if (indices_vec[sorted_indices[i]] < 0)
                indices_vec[sorted_indices[i]] += result.size();

            if (indices_vec[sorted_indices[i]] < 0 ||
                indices_vec[sorted_indices[i]] >= result.size())
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "insert::insert_flatten_nd_helper",
                    generate_error_message("index out of bound"));
            indices_vec[sorted_indices[i]] += i;
            mask[indices_vec[sorted_indices[i]]] = 0;
            result[indices_vec[sorted_indices[i]]] = values[sorted_indices[i]];
        }

        for (int i = 0; i < result.size(); ++i)
        {
            if (mask[i] == 1)
                if (flattened_result.size() == 1)
                    result[i] = flattened_result[0];
                else
                    result[i] = *d++;
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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
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
#endif

    template <typename T>
    primitive_argument_type insert::insert_flatten_nd(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values) const
    {
        auto ndim = arg.num_dimensions();
        switch (ndim)
        {
        case 0:
            HPX_FALLTHROUGH;
        case 1:
            return flatten_nd_helper(
                std::move(arg), std::move(indices), std::move(values));
        case 2:
            return insert_flatten_2d(
                std::move(arg), std::move(indices), std::move(values));
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return insert_flatten_3d(
                std::move(arg), std::move(indices), std::move(values));
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_flatten_0d",
            generate_error_message("index is out of bounds"));
    }

    template <typename T>
    primitive_argument_type insert::insert_1d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        std::int64_t axis) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_1d",
            generate_error_message("not implemented yet"));
    }

    template <typename T>
    primitive_argument_type insert::insert_2d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        std::int64_t axis) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_2"
            "d",
            generate_error_message("not implemented yet"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type insert::insert_3d(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        std::int64_t axis) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "insert::insert_3d",
            generate_error_message("not implemented yet"));
    }
#endif

    template <typename T>
    primitive_argument_type insert::insert_nd(ir::node_data<T>&& arg,
        ir::node_data<std::int64_t>&& indices, ir::node_data<T>&& values,
        hpx::util::optional<std::int64_t> axis) const
    {
        if (!axis)
            return insert_flatten_nd(
                std::move(arg), std::move(indices), std::move(values));

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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return insert_3d(std::move(arg), std::move(indices),
                std::move(values), axis.value());
#endif
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
        if (operands.empty() || (operands.size() < 3 || operands.size() > 4))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "insert::eval",
                generate_error_message(
                    "the insert primitive requires three or four operands"));
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
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                    -> primitive_argument_type {
                    hpx::util::optional<std::int64_t> axis;

                    if (args.size() == 4 && valid(args[3]))
                    {
                        axis = extract_scalar_integer_value_strict(
                            args[3], this_->name_, this_->codename_);
                    }

                    switch (extract_common_type(args[0]))
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
                    case node_data_type_unknown:
                        HPX_FALLTHROUGH;
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
