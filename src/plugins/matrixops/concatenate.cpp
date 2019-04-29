//  Copyright (c) 2019 Shahrzad Shirzad
//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/concatenate.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
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
    match_pattern_type const concatenate::match_data = {
        hpx::util::make_tuple("concatenate",
            std::vector<std::string>{
                "concatenate(_1, __arg(_2_axis, 0))"
            },
            &create_concatenate, &create_primitive<concatenate>, R"(
            ar, axis
            Args:

                ar (array) : sequence of array_like
                axis (optional, int) : the axis along which the arrays will be
                    joined, default is 0, if axis is None arrays are flattened
                    before use

            Returns:

            The joined sequence of arrays along an existing axis."

            )")};

    ///////////////////////////////////////////////////////////////////////////
    concatenate::concatenate(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t concatenate::get_vec_size(
        primitive_arguments_type const& args) const
    {
        std::size_t vec_size = 0;

        for (std::size_t i = 0; i != args.size(); ++i)
        {
            std::size_t num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::concatenate::"
                    "get_vec_size",
                    generate_error_message("the concatenate primitive requires "
                        "for all input arrays to have the same dimension"));
            }
            vec_size +=
                extract_numeric_value_dimensions(args[i], name_, codename_)[0];
        }
        return vec_size;
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate_flatten1d(
        primitive_arguments_type&& args) const
    {
        blaze::DynamicVector<T> result(get_vec_size(args));

        auto iter = result.begin();
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));
            std::size_t num_d = val.num_dimensions();

            iter = std::copy(val.vector().begin(), val.vector().end(), iter);
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate1d_helper(
        primitive_arguments_type&& args) const
    {
        blaze::DynamicVector<T> result(get_vec_size(args));

        auto iter = result.begin();
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));
            std::size_t num_d = val.num_dimensions();

            iter = std::copy(val.vector().begin(), val.vector().end(), iter);
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type concatenate::concatenate1d(
        primitive_arguments_type&& args, std::int64_t axis) const
    {
        if (axis < -1 || axis > 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "concatenate::concatenate1d",
                generate_error_message("axis is out of bounds of dimension 1"));
        }

        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return concatenate1d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return concatenate1d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return concatenate1d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::concatenate::concatenate1d",
            generate_error_message(
                "the concatenate primitive requires for all arguments to "
                "be numeric data types"));
    }

    std::size_t concatenate::get_matrix_size(
        primitive_arguments_type const& args) const
    {
        std::size_t matrix_size = 0;

        for (std::size_t i = 0; i != args.size(); ++i)
        {
            std::size_t num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::concatenate::get_"
                    "matrix_size",
                    generate_error_message("the concatenate primitive requires "
                        "for all input arrays to have the same dimension"));
            }

            matrix_size +=
                extract_numeric_value_dimensions(args[i], name_, codename_)[0] *
                extract_numeric_value_dimensions(args[i], name_, codename_)[1];
        }

        return matrix_size;
    }
    template <typename T>
    primitive_argument_type concatenate::concatenate2d_axis0(
        primitive_arguments_type&& args) const
    {
        std::size_t args_size = args.size();

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);
        std::size_t total_rows = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            if (extract_numeric_value_dimension(args[i], name_, codename_) != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate2d_axis0",
                    generate_error_message("all the input arrays must have "
                                           "same number of dimensions"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && prevdim[1] != dim[1])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate2d_axis0",
                    generate_error_message(
                        "all the input array dimensions except for "
                        "the concatenation axis must match exactly "));
            }

            total_rows += dim[0];
            prevdim = dim;
        }

        blaze::DynamicMatrix<T> result(total_rows, prevdim[1]);

        std::size_t step = 0;
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));

            std::size_t num_rows = val.dimension(0);
            for (std::size_t j = 0; j != num_rows; ++j)
            {
                blaze::row(result, j + step) = blaze::row(val.matrix(), j);
            }

            step += num_rows;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate2d_axis1(
        primitive_arguments_type&& args) const
    {
        std::size_t args_size = args.size();

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);
        std::size_t total_cols = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            if (extract_numeric_value_dimension(args[i], name_, codename_) != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate2d_axis1",
                    generate_error_message("all the input arrays must have "
                                           "same number of dimensions"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && prevdim[0] != dim[0])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate2d_axis1",
                    generate_error_message(
                        "all the input array dimensions except for "
                        "the concatenation axis must match exactly "));
            }

            total_cols += dim[1];
            prevdim = dim;
        }

        blaze::DynamicMatrix<T> result(prevdim[0], total_cols);

        std::size_t step = 0;
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));

            std::size_t num_cols = val.dimension(1);
            for (std::size_t j = 0; j != num_cols; ++j)
            {
                blaze::column(result, j + step) =
                    blaze::column(val.matrix(), j);
            }

            step += num_cols;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate2d_helper(
        primitive_arguments_type&& args, std::int64_t axis) const
    {
        switch (axis)
        {
        case 0: HPX_FALLTHROUGH;
        case -2:
            return concatenate2d_axis0<T>(std::move(args));

        case 1: HPX_FALLTHROUGH;
        case -1:
            return concatenate2d_axis1<T>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "concatenate::concatenate2d_helper",
            generate_error_message("axis is out of bounds of dimension 2"));
    }

    primitive_argument_type concatenate::concatenate2d(
        primitive_arguments_type&& args, std::int64_t axis) const
    {
        if (axis < -2 || axis > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "concatenate::concatenate1d",
                generate_error_message("axis is out of bounds of dimension 2"));
        }

        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return concatenate2d_helper<std::uint8_t>(std::move(args), axis);

        case node_data_type_int64:
            return concatenate2d_helper<std::int64_t>(std::move(args), axis);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return concatenate2d_helper<double>(std::move(args), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::concatenate::concatenate1d",
            generate_error_message(
                "the concatenate primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate_flatten2d(
        primitive_arguments_type&& args) const
    {
        blaze::DynamicVector<T> result(get_matrix_size(args));
        using phylanx::util::matrix_row_iterator;
        auto iter = result.begin();
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));

            auto a = val.matrix();
            const matrix_row_iterator<decltype(a)> a_begin(a);
            const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

            for (auto it = a_begin; it != a_end; ++it)
            {
                iter = std::copy(it->begin(), it->end(), iter);
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate_flatten_helper(
        primitive_arguments_type&& args) const
    {
        std::size_t dims = extract_largest_dimension(args, name_, codename_);
        switch (dims)
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "concatenate::concatenate_flatten",
                generate_error_message(
                    "zero-dimensional arrays cannot be concatenated"));
            break;

        case 1:
            return concatenate_flatten1d<T>(std::move(args));

        case 2:
            return concatenate_flatten2d<T>(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return concatenate_flatten3d<T>(std::move(args));
#endif
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::concatenate::concatenate_"
            "flatten_helper",
            generate_error_message(
                "the operands have unsupported number of dimensions"));
    }

    primitive_argument_type concatenate::concatenate_flatten(
        primitive_arguments_type&& args) const
    {
        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return concatenate_flatten_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return concatenate_flatten_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return concatenate_flatten_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::concatenate::concatenate_"
            "flatten",
            generate_error_message(
                "the concatenate primitive requires for all arguments to "
                "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    std::size_t concatenate::get_tensor_size(
        primitive_arguments_type const& args) const
    {
        std::size_t tensor_size = 0;

        for (std::size_t i = 0; i != args.size(); ++i)
        {
            std::size_t num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 3)
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::concatenate::get_"
                    "tensor_size",
                    generate_error_message("the concatenate primitive requires "
                                           "for all input arrays to "
                                           "have the same dimension"));
            tensor_size +=
                extract_numeric_value_dimensions(args[i], name_, codename_)[0] *
                extract_numeric_value_dimensions(args[i], name_, codename_)[1] *
                extract_numeric_value_dimensions(args[i], name_, codename_)[2];
        }
        return tensor_size;
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate_flatten3d(
        primitive_arguments_type&& args) const
    {
        blaze::DynamicVector<T> result(get_tensor_size(args));

        using phylanx::util::matrix_column_iterator;

        auto d = result.data();
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));

            auto a = val.tensor();
            for (std::size_t i = 0; i < a.rows(); ++i)
            {
                auto slice = blaze::rowslice(a, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                for (auto it = c_begin; it != c_end; ++it)
                {
                    d = std::copy(it->begin(), it->end(), d);
                }
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate3d_axis0(
        primitive_arguments_type&& args) const
    {
        std::size_t args_size = args.size();

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);
        std::size_t total_pages = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            if (extract_numeric_value_dimension(args[i], name_, codename_) != 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate3d_axis0",
                    generate_error_message(
                        "for 3d concatenation, the concatenate primitive "
                        "requires all the inputs be tensors"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && (prevdim[1] != dim[1] || prevdim[2] != dim[2]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate3d_axis0",
                    generate_error_message(
                        "all the input array dimensions except for "
                        "the concatenation axis must match exactly "));
            }

            total_pages += dim[0];
            prevdim = dim;
        }

        blaze::DynamicTensor<T> result(total_pages, prevdim[1], prevdim[2]);
        using phylanx::util::matrix_column_iterator;

        std::size_t step = 0;
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));
            auto a = val.tensor();
            std::size_t num_pages = val.dimension(0);

            for (std::size_t j = 0; j != num_pages; ++j)
            {
                blaze::pageslice(result, j + step) =
                    blaze::pageslice(val.tensor(), j);
            }

            step += num_pages;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate3d_axis1(
        primitive_arguments_type&& args) const
    {
        std::size_t args_size = args.size();

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);
        std::size_t total_rows = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            if (extract_numeric_value_dimension(args[i], name_, codename_) != 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate3d_axis1",
                    generate_error_message(
                        "for 3d concatenation, the concatenate primitive "
                        "requires all the inputs be tensors"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && (prevdim[0] != dim[0] || prevdim[2] != dim[2]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate3d_axis1",
                    generate_error_message(
                        "all the input array dimensions except for "
                        "the concatenation axis must match exactly "));
            }

            total_rows += dim[1];
            prevdim = dim;
        }

        blaze::DynamicTensor<T> result(prevdim[0], total_rows, prevdim[2]);

        std::size_t step = 0;
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));
            auto a = val.tensor();
            std::size_t num_rows = val.dimension(1);

            for (std::size_t j = 0; j != num_rows; ++j)
            {
                blaze::rowslice(result, j + step) =
                    blaze::rowslice(val.tensor(), j);
            }

            step += num_rows;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate3d_axis2(
        primitive_arguments_type&& args) const
    {
        std::size_t args_size = args.size();

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);
        std::size_t total_cols = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            if (extract_numeric_value_dimension(args[i], name_, codename_) != 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate3d_axis2",
                    generate_error_message(
                        "for 3d concatenation, the concatenate primitive "
                        "requires all the inputs be tensors"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && (prevdim[0] != dim[0] || prevdim[1] != dim[1]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::concatenate3d_axis2",
                    generate_error_message(
                        "all the input array dimensions except for "
                        "the concatenation axis must match exactly "));
            }

            total_cols += dim[2];
            prevdim = dim;
        }

        blaze::DynamicTensor<T> result(prevdim[0], prevdim[1], total_cols);

        std::size_t step = 0;
        for (auto&& arg : args)
        {
            auto&& val = extract_node_data<T>(std::move(arg));
            auto a = val.tensor();
            std::size_t num_cols = val.dimension(2);

            for (std::size_t j = 0; j != num_cols; ++j)
            {
                blaze::columnslice(result, j + step) =
                    blaze::columnslice(val.tensor(), j);
            }

            step += num_cols;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type concatenate::concatenate3d_helper(
        primitive_arguments_type&& args, std::int64_t axis) const
    {
        switch (axis)
        {
        case 0: HPX_FALLTHROUGH;
        case -3:
            return concatenate3d_axis0<T>(std::move(args));

        case 1: HPX_FALLTHROUGH;
        case -2:
            return concatenate3d_axis1<T>(std::move(args));

        case 2: HPX_FALLTHROUGH;
        case -1:
            return concatenate3d_axis2<T>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::concatenate::concatenate3d_"
            "helper",
            generate_error_message("axis is out of bounds of dimension 3"));
    }

    primitive_argument_type concatenate::concatenate3d(
        primitive_arguments_type&& args, std::int64_t axis) const
    {
        if (axis < -3 || axis > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "concatenate::concatenate1d",
                generate_error_message("axis is out of bounds of dimension 3"));
        }

        switch (extract_common_type(args))
        {
        case node_data_type_bool:
            return concatenate3d_helper<std::uint8_t>(std::move(args), axis);

        case node_data_type_int64:
            return concatenate3d_helper<std::int64_t>(std::move(args), axis);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return concatenate3d_helper<double>(std::move(args), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::concatenate::concatenate3d",
            generate_error_message(
                "the concatenate primitive requires for all arguments to "
                "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> concatenate::handle_concatenate(
        primitive_arguments_type const& operands,
        primitive_argument_type const& axis,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        hpx::future<primitive_argument_type> axis_f =
            value_operand(axis, args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                    primitive_arguments_type&& args,
                    primitive_argument_type&& axis_arg)
            ->  primitive_argument_type
            {
                if (!valid(axis_arg))
                {
                    return this_->concatenate_flatten(std::move(args));
                }

                // last argument has to be the axis
                std::int64_t axis = extract_scalar_integer_value_strict(
                    std::move(axis_arg), this_->name_, this_->codename_);

                std::size_t dims = extract_largest_dimension(
                    args, this_->name_, this_->codename_);
                switch (dims)
                {
                case 0:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "concatenate::eval",
                        this_->generate_error_message(
                            "zero-dimensional arrays cannot be concatenated"));

                case 1:
                    return this_->concatenate1d(
                        std::move(args), std::move(axis));

                case 2:
                    return this_->concatenate2d(
                        std::move(args), std::move(axis));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->concatenate3d(
                        std::move(args), std::move(axis));
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "concatenate::eval",
                        this_->generate_error_message(
                            "first operand has unsupported number of "
                            "dimensions"));
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "concatenate::eval",
                    this_->generate_error_message(
                        "the operands have unsupported number of dimensions"));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)),
            std::move(axis_f));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> concatenate::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "concatenate::eval",
                generate_error_message(
                    "the concatenate primitive requires at most two operands"));
        }

        // one-argument invocation may be special if the the first argument is
        // a list
        primitive_argument_type arg1;
        if (is_primitive_operand(operands[0]))
        {
            arg1 = value_operand_sync(operands[0], args, name_, codename_, ctx);
        }
        else
        {
            arg1 = operands[0];
        }

        primitive_arguments_type ops;
        if (is_list_operand_strict(arg1))
        {
            auto&& r =
                extract_list_value_strict(std::move(arg1), name_, codename_);
            ops.reserve(r.size());
            for (auto && op : r)
            {
                if (is_list_operand_strict(op))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "concatenate::eval",
                        generate_error_message("lists cannot be concatenated"));
                }
                ops.push_back(std::move(op));
            }
        }
        else
        {
            ops.reserve(operands.size());
            ops.push_back(std::move(arg1));
        }

        if (operands.size() == 2)
        {
            return handle_concatenate(ops, operands[1], args, std::move(ctx));
        }

        return handle_concatenate(ops, primitive_argument_type{std::int64_t(0)},
            args, std::move(ctx));
    }
}}}
