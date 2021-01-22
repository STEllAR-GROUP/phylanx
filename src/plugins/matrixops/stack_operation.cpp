// Copyright (c) 2018 Bibek Wagle
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/stack_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const stack_operation::match_data =
    {
        match_pattern_type{
            "hstack",
            std::vector<std::string>{"hstack(_1, __arg(_2_dtype, nil))"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args, dtype
            Args:

                *args (list, optional) : a list of array-like objects
                dtype (optional, string) : the data-type of the returned array,
                  defaults to dtype of input arrays.

            Returns:

            A horizontally (column wise) stacked sequence of array-like objects)"
        },
        match_pattern_type{
            "vstack",
            std::vector<std::string>{"vstack(_1, __arg(_2_dtype, nil))"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args, dtype
            Args:

                *args (list, optional) : a list of array-like objects
                dtype (optional, string) : the data-type of the returned array,
                  defaults to dtype of input arrays.

            Returns:

            A vertically (row wise) stacked sequence of array-like objects)"
        },
        match_pattern_type{
            "stack",
            std::vector<std::string>{
                "stack(_1, __arg(_2_axis, 0), __arg(_3_dtype, nil))"
            },
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args, axis, dtype
            Args:

                *args (list, optional) : a list of array-like objects
                axis (int, optional) : the axis along which to stack input
                    values, the default value is '0'
                dtype (optional, string) : the data-type of the returned array,
                  defaults to dtype of input arrays.

            Returns:

            A joined sequence of array-like objects along a new axis.)"
        }
      , match_pattern_type{
            "dstack",
            std::vector<std::string>{"dstack(_1, __arg(_2_dtype, nil))"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args, dtype
            Args:

                *args (list, optional) : a list of array-like objects
                dtype (optional, string) : the data-type of the returned array,
                  defaults to dtype of input arrays.

            Returns:

            A vertically (depth wise) stacked sequence of array-like objects)"
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        stack_operation::stacking_mode extract_stacking_mode(
            std::string const& name)
        {
            stack_operation::stacking_mode result =
                stack_operation::stacking_mode_axis;

            if (name.find("vstack") != std::string::npos)
            {
                result = stack_operation::stacking_mode_row_wise;
            }
            else if (name.find("hstack") != std::string::npos)
            {
                result = stack_operation::stacking_mode_column_wise;
            }
            else if (name.find("dstack") != std::string::npos)
            {
                result = stack_operation::stacking_mode_depth_wise;
            }
            return result;
        }

        template <typename T>
        primitive_argument_type empty_helper(std::size_t dims,
            std::string const& name, std::string const& codename)
        {
            switch (dims)
            {
            case 1:
                {
                    // hstack() without arguments returns an empty 1D vector
                    using storage1d_type =
                        typename ir::node_data<T>::storage1d_type;
                    return primitive_argument_type{
                        ir::node_data<T>{storage1d_type(0)}};
                }

            case 2:
                {
                    // vstack() without arguments returns an empty 2D matrix
                    using storage2d_type =
                        typename ir::node_data<T>::storage2d_type;
                    return primitive_argument_type{
                        ir::node_data<T>{storage2d_type(0, 0)}};
                }

            case 3:
                {
                    // dstack without arguments returns an empty 3D tensor
                    using storage3d_type =
                        typename ir::node_data<T>::storage3d_type;
                    return primitive_argument_type{
                        ir::node_data<T>{storage3d_type(0, 0, 0)}};
                }

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "detail::empty_helper",
                util::generate_error_message(
                    "unsupported stacking mode requested", name, codename));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    hpx::future<primitive_argument_type> stack_operation::empty_helper() const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            {
                // hstack() without arguments returns an empty 1D vector
                return hpx::make_ready_future(
                    detail::empty_helper<T>(1, name_, codename_));
            }

        case stacking_mode_row_wise:
            {
                // vstack() without arguments returns an empty 2D matrix
                return hpx::make_ready_future(
                    detail::empty_helper<T>(2, name_, codename_));
            }

        case stacking_mode_depth_wise:
            {
                // dstack without arguments returns an empty 3D tensor
                return hpx::make_ready_future(
                    detail::empty_helper<T>(3, name_, codename_));
            }

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::empty",
            generate_error_message("unsupported stacking mode requested"));
    }

    ///////////////////////////////////////////////////////////////////////////
    stack_operation::stack_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , mode_(detail::extract_stacking_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    std::size_t stack_operation::get_vecsize(
        primitive_arguments_type const& args) const
    {
        std::size_t vec_size = 0;

        for (std::size_t i = 0; i != args.size(); ++i)
        {
            std::size_t num_dims = std::size_t(-1);
            if (is_list_operand_strict(args[i]))
            {
                num_dims = 1;
            }
            else if (is_numeric_operand(args[i]))
            {
                num_dims =
                    extract_numeric_value_dimension(args[i], name_, codename_);
            }

            if (num_dims != 0 && num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::get_vecsize",
                    generate_error_message(
                        "for 0d/1d stacking, the stack_operation primitive "
                        "requires the input to be either a list, a vector or "
                        "a scalar"));
            }

            if (num_dims == 0)
            {
                ++vec_size;
            }
            else if (is_list_operand_strict(args[i]))
            {
                vec_size += extract_list_value_size(args[i], name_, codename_);
            }
            else
            {
                vec_size += extract_numeric_value_dimensions(
                    args[i], name_, codename_)[0];
            }
        }

        return vec_size;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::hstack0d1d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(1, name_, codename_);
        }

        blaze::DynamicVector<T> result(get_vecsize(args));

        auto iter = result.begin();
        for (auto && arg : args)
        {
            if (is_list_operand_strict(arg))
            {
                auto&& val =
                    extract_list_value_strict(std::move(arg), name_, codename_);
                for (auto&& v : val)
                {
                    *iter++ =
                        extract_scalar_data<T>(std::move(v), name_, codename_);
                }
            }
            else
            {
                auto&& val =
                    extract_node_data<T>(std::move(arg), name_, codename_);
                std::size_t num_d = val.num_dimensions();
                if (num_d == 0)
                {
                    *iter++ = val.scalar();
                }
                else
                {
                    iter = std::copy(
                        val.vector().begin(), val.vector().end(), iter);
                }
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::hstack0d1d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return hstack0d1d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return hstack0d1d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return hstack0d1d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::hstack0d1d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::hstack2d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(2, name_, codename_);
        }

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
                        "stack_operation::hstack2d_helper",
                    generate_error_message(
                        "for 2d stacking, the stack_operation primitive "
                            "requires all the inputs be a matrices"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            if (i != 0 && prevdim[0] != dim[0])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::hstack2d_helper",
                    generate_error_message(
                        "the stack_operation primitive requires the "
                            "number of rows to be equal for all matrices "
                            "being stacked"));
            }

            total_cols += dim[1];
            prevdim = dim;
        }

        blaze::DynamicMatrix<T> result(prevdim[0], total_cols);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

            std::size_t num_cols = val.dimension(1);
            for (std::size_t j = 0; j != num_cols; ++j)
            {
                blaze::column(result, j + step) = blaze::column(val.matrix(), j);
            }

            step += num_cols;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::hstack2d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return hstack2d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return hstack2d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return hstack2d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::hstack2d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::hstack3d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);
        if (num_dims != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "stack_operation::hstack3d",
                generate_error_message(
                    "the (h)stack_operation primitive can not stack "
                    "tensors with anything else"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t total_rows = dim[1];
        std::size_t num_pages = dim[0];
        std::size_t num_cols = dim[2];

        std::size_t args_size = args.size();
        for (std::size_t i = 1; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::vstack3d",
                    generate_error_message(
                        "the (h)stack_operation primitive can not stack "
                        "tensors with anything else"));
            }

            dim = extract_numeric_value_dimensions(args[i], name_, codename_);

            if (num_pages != dim[0] || num_cols != dim[2])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::hstack3d",
                    generate_error_message(
                        "the (h)stack_operation primitive requires for the "
                        "number of rows/columns to be equal for all "
                        "tensors being stacked"));
            }

            total_rows += dim[1];
        }

        blaze::DynamicTensor<T> result(num_pages, total_rows, num_cols);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

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

    primitive_argument_type stack_operation::hstack3d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return hstack3d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return hstack3d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return hstack3d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::hstack3d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::vstack0d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(2, name_, codename_);
        }

        std::size_t vec_size = args.size();

        blaze::DynamicMatrix<T> result(vec_size, 1);
        auto col = blaze::column(result, 0);

        std::size_t i = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

            if (val.num_dimensions() != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::vstack0d",
                    generate_error_message(
                        "the stack_operation primitive requires all the "
                        "inputs be a scalar for 0d stacking"));
            }

            col[i++] = val.scalar();
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::vstack0d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return vstack0d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return vstack0d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return vstack0d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::vstack0d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::vstack1d2d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(2, name_, codename_);
        }

        std::size_t args_size = args.size();

        std::size_t num_dims_first =
            extract_numeric_value_dimension(args[0], name_, codename_);

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> prevdim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t total_rows = 1;
        std::size_t num_cols = prevdim[0];

        if (num_dims_first == 2)
        {
            total_rows = prevdim[0];
            num_cols = prevdim[1];
        }

        std::size_t first_size = num_cols;

        for (std::size_t i = 1; i != args_size; ++i)
        {
            std::size_t num_dims_second =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims_first == 0 || num_dims_second == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::vstack2d",
                    generate_error_message(
                        "the stack_operation primitive can not stack "
                        "matrices/vectors with a scalar"));
            }

            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
                extract_numeric_value_dimensions(args[i], name_, codename_);

            std::size_t second_size = dim[0];
            if (num_dims_second == 2)
            {
                total_rows += dim[0];
                second_size = dim[1];
            }
            else
            {
                ++total_rows;
            }

            if (first_size != second_size)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::vstack2d",
                    generate_error_message(
                        "the stack_operation primitive requires for the "
                        "number of columns/size to be equal for all "
                        "matrices/vectors being stacked"));
            }

            num_dims_first = num_dims_second;
            first_size = second_size;
        }

        blaze::DynamicMatrix<double> result(total_rows, num_cols);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));
            if (val.num_dimensions() == 2)
            {
                std::size_t num_rows = val.dimension(0);
                for (std::size_t j = 0; j != num_rows; ++j)
                {
                    blaze::row(result, j + step) =
                        blaze::row(val.matrix(), j);
                }
                step += num_rows;
            }
            else
            {
                blaze::row(result, step) = blaze::trans(val.vector());
                ++step;
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::vstack1d2d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return vstack1d2d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return vstack1d2d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return vstack1d2d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::vstack1d2d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::vstack3d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);
        if (num_dims != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "stack_operation::vstack3d",
                generate_error_message(
                    "the (d)stack_operation primitive can not stack "
                    "tensors with anything else"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t total_pages = dim[0];
        std::size_t num_rows = dim[1];
        std::size_t num_cols = dim[2];

        std::size_t args_size = args.size();
        for (std::size_t i = 1; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::vstack3d",
                    generate_error_message(
                        "the (d)stack_operation primitive can not stack "
                        "tensors with anything else"));
            }

            dim = extract_numeric_value_dimensions(args[i], name_, codename_);

            if (num_rows != dim[1] || num_cols != dim[2])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::vstack3d",
                    generate_error_message(
                        "the (v)stack_operation primitive requires for the "
                        "number of rows/columns to be equal for all "
                        "tensors being stacked"));
            }

            total_pages += dim[0];
        }

        blaze::DynamicTensor<T> result(total_pages, num_rows, num_cols);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

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

    primitive_argument_type stack_operation::vstack3d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return vstack3d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return vstack3d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return vstack3d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::vstack3d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::dstack0d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t vec_size = args.size();

        blaze::DynamicTensor<T> result(vec_size, 1, 1);
        auto row = blaze::row(blaze::rowslice(result, 0), 0);

        std::size_t i = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));

            if (val.num_dimensions() != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::dstack0d",
                    generate_error_message(
                        "the stack_operation primitive requires for all "
                        "inputs to be a scalar for 0d stacking"));
            }

            row[i++] = val.scalar();
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::dstack0d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return dstack0d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return dstack0d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dstack0d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::dstack0d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    primitive_argument_type stack_operation::dstack1d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);
        if (num_dims != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "stack_operation::dstack0d",
                generate_error_message(
                    "the (d)stack_operation primitive can not stack "
                    "vectors with a scalar"));
        }

        std::size_t size =
            extract_numeric_value_dimensions(args[0], name_, codename_)[0];

        std::size_t args_size = args.size();
        for (std::size_t i = 1; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::dstack1d",
                    generate_error_message(
                        "the (d)stack_operation primitive can not stack "
                        "matrices/vectors with a scalar"));
            }

            if (size !=
                extract_numeric_value_dimensions(args[i], name_, codename_)[0])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::dstack1d",
                    generate_error_message(
                        "the (d)stack_operation primitive requires for the "
                        "number of columns/size to be equal for all "
                        "vectors being stacked"));
            }
        }

        blaze::DynamicTensor<T> result(1, size, args_size);
        auto page = blaze::pageslice(result, 0);

        std::size_t j = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));
            blaze::column(page, j) = val.vector();
            ++j;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::dstack1d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return dstack1d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return dstack1d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dstack1d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::dstack1d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    primitive_argument_type stack_operation::dstack2d3d_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);

        if (num_dims != 2 && num_dims != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "stack_operation::dstack2d3d",
                generate_error_message(
                    "the (d)stack_operation primitive can not stack "
                    "matrices with a vector or a scalar"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t total_columns = 1;
        std::size_t num_rows = dim[0];
        std::size_t num_cols = dim[1];

        if (num_dims == 3)
        {
            total_columns = dim[2];
        }

        std::size_t args_size = args.size();
        for (std::size_t i = 1; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims != 2 && num_dims != 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::dstack2d3d",
                    generate_error_message(
                        "the (d)stack_operation primitive can not stack "
                        "matrices with a vector or a scalar"));
            }

            dim = extract_numeric_value_dimensions(args[i], name_, codename_);
            if (num_rows != dim[0] || num_cols != dim[1])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::dstack2d3d",
                    generate_error_message(
                        "the (d)stack_operation primitive requires for the "
                        "number of rows/columns to be equal for all "
                        "matrices and tensors being stacked"));
            }

            if (num_dims == 3)
            {
                total_columns += dim[2];
            }
            else
            {
                ++total_columns;
            }
        }

        blaze::DynamicTensor<T> result(num_rows, num_cols, total_columns);

        std::size_t step = 0;
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));
            if (val.num_dimensions() == 3)
            {
                std::size_t num_columns = val.dimension(2);
                for (std::size_t j = 0; j != num_columns; ++j)
                {
                    blaze::columnslice(result, j + step) =
                        blaze::columnslice(val.tensor(), j);
                }
                step += num_columns;
            }
            else
            {
                blaze::columnslice(result, step) = val.matrix();
                ++step;
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::dstack2d3d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return dstack2d3d_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return dstack2d3d_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return dstack2d3d_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::dstack2d3d",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type stack_operation::stack0d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack0d1d(std::move(args), std::move(dtype));

        case stacking_mode_row_wise:
            return vstack0d(std::move(args), std::move(dtype));

        case stacking_mode_depth_wise:
            return dstack0d(std::move(args), std::move(dtype));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack0d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

    primitive_argument_type stack_operation::stack0d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype,
        std::int64_t axis) const
    {
        if (axis == 0 || axis == -1)
            return hstack0d1d(std::move(args), std::move(dtype));

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack0d",
            generate_error_message("unsupported axis requested"));
    }

    template <typename T>
    primitive_argument_type stack_operation::stack1d_axis1_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(2, name_, codename_);
        }

        std::size_t args_size = args.size();

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);

        if (num_dims != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "stack_operation::stack1d_axis1_helper",
                generate_error_message(
                    "the stack_operation primitive can not stack "
                    "vectors with a scalar/matrix"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t vector_size_first = dim[0];

        blaze::DynamicMatrix<T> result(vector_size_first, args.size());
        std::size_t step = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::stack1d_axis1_helper",
                    generate_error_message(
                        "the stack_operation primitive can not stack "
                        "vectors with a scalar/matrix"));
            }

            dim = extract_numeric_value_dimensions(args[i], name_, codename_);

            if (dim[0] != vector_size_first)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::stack1d_axis1_helper",
                    generate_error_message(
                        "the stack_operation primitive requires for the "
                        "size to be equal for all vectors being stacked"));
            }

            auto&& val = extract_node_data<T>(std::move(args[i]));
            blaze::column(result, step) = val.vector();
            ++step;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::stack1d_axis1(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return stack1d_axis1_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return stack1d_axis1_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return stack1d_axis1_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::stack1d_"
            "axis1",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    primitive_argument_type stack_operation::stack1d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack0d1d(std::move(args), std::move(dtype));

        case stacking_mode_row_wise:
            return vstack1d2d(std::move(args), std::move(dtype));

        case stacking_mode_depth_wise:
            return dstack1d(std::move(args), std::move(dtype));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack1d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

    primitive_argument_type stack_operation::stack1d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype,
        std::int64_t axis) const
    {
        if (args.size() != 1)
        {
            if (axis == 0 || axis == -2)
                return vstack1d2d(std::move(args), std::move(dtype));
            if (axis == 1 || axis == -1)
                return stack1d_axis1(std::move(args), std::move(dtype));
        }
        else if (axis == 0 || axis == -1)
            return primitive_argument_type{std::move(args[0])};

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack1d",
            generate_error_message("unsupported axis requested"));
    }

    template <typename T>
    primitive_argument_type stack_operation::stack2d_axis0_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);
        if (num_dims != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "stack_operation::stack2d_axis0_helper",
                generate_error_message(
                    "the stack_operation primitive can not stack "
                    "matrices with anything else"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t num_rows = dim[0];
        std::size_t num_cols = dim[1];
        std::size_t args_size = args.size();

        if (args.size() == 1)
        {
            return primitive_argument_type{std::move(args[0])};
        }

        blaze::DynamicTensor<T> result(args_size, num_rows, num_cols);
        std::size_t j = 0;

        for (std::size_t i = 0; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::stack2d_axis0_helper",
                    generate_error_message(
                        "the stack_operation primitive can not stack "
                        "tensors with anything else"));
            }

            dim = extract_numeric_value_dimensions(args[i], name_, codename_);

            if (num_rows != dim[0] || num_cols != dim[1])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::stack2d_axis0_helper",
                    generate_error_message(
                        "the (stack_operation primitive requires for the "
                        "number of rows/columns to be equal for all "
                        "matrices being stacked"));
            }
            auto&& val = extract_node_data<T>(std::move(args[i]));

            blaze::pageslice(result, j) = val.matrix();
            ++j;
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::stack2d_axis0(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return stack2d_axis0_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return stack2d_axis0_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return stack2d_axis0_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::stack2d_"
            "axis0_helper",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    primitive_argument_type stack_operation::stack2d_axis1_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);
        if (num_dims != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "stack_operation::stack2d_axis1_helper",
                generate_error_message(
                    "the stack_operation primitive can not stack "
                    "matrices with anything else"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t num_rows = dim[0];
        std::size_t num_cols = dim[1];
        std::size_t args_size = args.size();

        if (args.size() == 1)
        {
            blaze::DynamicMatrix<T> result(num_cols, num_rows);

            auto&& val = extract_node_data<T>(std::move(args[0]));
            auto arr = val.matrix();

            for (std::size_t k = 0; k < num_rows; ++k)
            {
                blaze::column(result, k) = blaze::trans(blaze::row(arr, k));
            }

            return primitive_argument_type{ir::node_data<T>{std::move(result)}};
        }

        blaze::DynamicTensor<T> result(num_rows, args_size, num_cols);

        for (std::size_t i = 0; i != args_size; ++i)
        {
            num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);
            if (num_dims != 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::stack2d_axis1_helper",
                    generate_error_message(
                        "the (d)stack_operation primitive can not stack "
                        "tensors with anything else"));
            }

            dim = extract_numeric_value_dimensions(args[i], name_, codename_);

            if (num_rows != dim[0] || num_cols != dim[1])
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "stack_operation::stack2d_axis1_helper",
                    generate_error_message(
                        "the stack_operation primitive requires for the "
                        "number of rows/columns to be equal for all "
                        "matrices being stacked"));
            }

            auto&& val = extract_node_data<T>(std::move(args[i]));
            auto arr = val.matrix();

            for (std::size_t k = 0; k < num_rows; ++k)
            {
                blaze::row(blaze::pageslice(result, k), i) = blaze::row(arr, k);
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::stack2d_axis1(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return stack2d_axis1_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return stack2d_axis1_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return stack2d_axis1_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::stack2d_axis1",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    primitive_argument_type stack_operation::stack2d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack2d(std::move(args), std::move(dtype));

        case stacking_mode_row_wise:
            return vstack1d2d(std::move(args), std::move(dtype));

        case stacking_mode_depth_wise:
            return dstack2d3d(std::move(args), std::move(dtype));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack2d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

    primitive_argument_type stack_operation::stack2d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype,
        std::int64_t axis) const
    {
        if (args.size() == 1)
        {
            if (axis == 0 || axis == -2)
                return primitive_argument_type{std::move(args[0])};
            if (axis == 1 || axis == -1)
                return stack2d_axis1(std::move(args), std::move(dtype));
        }
        else
        {
            if (axis == 0 || axis == -3)
                return stack2d_axis0(std::move(args), std::move(dtype));
            if (axis == 1 || axis == -2)
                return stack2d_axis1(std::move(args), std::move(dtype));
            if (axis == 2 || axis == -1)
                return dstack2d3d(std::move(args), std::move(dtype));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack2d",
            generate_error_message("unsupported axis requested"));
    }

    template <typename T>
    primitive_argument_type stack_operation::stack3d_axis1_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);
        if (num_dims != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "stack_operation::stack3d_axis1",
                generate_error_message(
                    "the stack_operation primitive can not stack "
                    "matrices with anything else"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t num_pages = dim[0];
        std::size_t num_rows = dim[1];
        std::size_t num_cols = dim[2];

        blaze::DynamicTensor<T> result(num_rows, num_pages, num_cols);

        auto&& val = extract_node_data<T>(std::move(args[0]));
        auto arr = val.tensor();

        for (std::size_t i = 0; i < num_pages; ++i)
        {
            for (std::size_t k = 0; k < num_rows; ++k)
            {
                blaze::row(blaze::pageslice(result, k), i) =
                        blaze::row(blaze::pageslice(arr, i), k);
            }
        }
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::stack3d_axis1(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return stack3d_axis1_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return stack3d_axis1_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return stack3d_axis1_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::stack3d_"
            "axis1",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    template <typename T>
    primitive_argument_type stack_operation::stack3d_axis2_helper(
        primitive_arguments_type&& args) const
    {
        if (args.empty())
        {
            return detail::empty_helper<T>(3, name_, codename_);
        }

        std::size_t num_dims =
            extract_numeric_value_dimension(args[0], name_, codename_);

        if (num_dims != 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "stack_operation::stack3d_axis2",
                generate_error_message(
                    "the stack_operation primitive can not stack "
                    "matrices with anything else"));
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dim =
            extract_numeric_value_dimensions(args[0], name_, codename_);

        std::size_t num_pages = dim[0];
        std::size_t num_rows = dim[1];
        std::size_t num_cols = dim[2];

        blaze::DynamicTensor<T> result(num_rows, num_cols, num_pages);

        auto&& val = extract_node_data<T>(std::move(args[0]));
        auto arr = val.tensor();

        for (std::size_t i = 0; i < num_pages; ++i)
        {
            for (std::size_t k = 0; k < num_rows; ++k)
            {
                blaze::column(blaze::pageslice(result, k), i) =
                    blaze::trans(blaze::row(blaze::pageslice(arr, i), k));
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::stack3d_axis2(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        // last argument is a dtype
        node_data_type t = node_data_type_unknown;
        if (is_string_operand(dtype))
        {
            t = map_dtype(
                extract_string_value(std::move(dtype), name_, codename_));
        }

        if (t == node_data_type_unknown)
        {
            t = extract_common_type(args);
        }

        switch (t)
        {
        case node_data_type_bool:
            return stack3d_axis2_helper<std::uint8_t>(std::move(args));

        case node_data_type_int64:
            return stack3d_axis2_helper<std::int64_t>(std::move(args));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return stack3d_axis2_helper<double>(std::move(args));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::stack3d_"
            "axis2",
            generate_error_message(
                "the stack_operation primitive requires for all arguments to "
                "be numeric data types"));
    }

    primitive_argument_type stack_operation::stack3d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack3d(std::move(args), std::move(dtype));

        case stacking_mode_row_wise:
            return vstack3d(std::move(args), std::move(dtype));

        case stacking_mode_depth_wise:
            return dstack2d3d(std::move(args), std::move(dtype));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack3d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

    primitive_argument_type stack_operation::stack3d(
        primitive_arguments_type&& args, primitive_argument_type&& dtype,
        std::int64_t axis) const
    {
        if (args.size() == 1)
        {
            if (axis == 0 || axis == -3)
                return primitive_argument_type{std::move(args[0])};
            if (axis == 1 || axis == -2)
                return stack3d_axis1(std::move(args), std::move(dtype));
            if (axis == 2 || axis == -1)
                return stack3d_axis2(std::move(args), std::move(dtype));
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::stack3d",
                generate_error_message("unsupported axis requested"));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack3d",
            generate_error_message("unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        bool has_list_operand_strict(primitive_argument_type const& arg,
            std::size_t& numargs, std::string const& name,
            std::string const& codename)
        {
            bool result = false;
            numargs = 0;
            if (is_list_operand_strict(arg))
            {
                numargs += extract_list_value_size(arg, name, codename);
                result = true;
            }
            else
            {
                ++numargs;
            }
            return result;
        }
    }

    primitive_argument_type stack_operation::handle_hvdstack(
        primitive_arguments_type&& operands, primitive_arguments_type && args,
        eval_context ctx) const
    {
        primitive_arguments_type ops;

        if (mode_ == stacking_mode_column_wise)
        {
            std::size_t numargs = 0;
            if (detail::has_list_operand_strict(
                    operands[0], numargs, name_, codename_))
            {
                ops.reserve(numargs);
                if (is_list_operand_strict(operands[0]))
                {
                    auto&& list = extract_list_value_strict(
                        std::move(operands[0]), name_, codename_);

                    for (auto&& l : list)
                    {
                        ops.emplace_back(std::move(l));
                    }
                }
                else
                {
                    ops.emplace_back(std::move(operands[0]));
                }
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "stack_operation::handle_hvdstack",
                    generate_error_message(
                        "the first argument to the hstack "
                        "primitive has to be a list of arrays to stack",
                        std::move(ctx)));
            }
        }
        else
        {
            if (is_list_operand_strict(operands[0]))
            {
                auto&& r = extract_list_value_strict(
                    std::move(operands[0]), name_, codename_);

                ops.reserve(r.size());
                for (auto&& op : r)
                {
                    if (is_list_operand_strict(op))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "stack_operation::handle_stack",
                            generate_error_message(
                                "lists cannot be stacked", std::move(ctx)));
                    }
                    ops.push_back(std::move(op));
                }
            }
            else
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "stack_operation::hvdhandle_stack",
                    generate_error_message(
                        hpx::util::format(
                            "the first argument to the {}stack "
                            "primitive has to be a list of arrays to stack",
                            mode_ == stacking_mode_row_wise ? 'v' : 'd'),
                        std::move(ctx)));
            }
        }

        primitive_argument_type dtype;
        if (operands.size() >= 2 && valid(operands[1]))
        {
            dtype = value_operand_sync(
                std::move(operands[1]), args, name_, codename_, ctx);
        }

        ops = detail::map_operands(ops, functional::value_operand_sync{},
            std::move(args), name_, codename_, ctx);

        switch (extract_largest_dimension(ops, name_, codename_))
        {
        case 0:
            return stack0d(std::move(ops), std::move(dtype));

        case 1:
            return stack1d(std::move(ops), std::move(dtype));

        case 2:
            return stack2d(std::move(ops), std::move(dtype));

        case 3:
            return stack3d(std::move(ops), std::move(dtype));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::handle_hvdstack",
                generate_error_message(
                    "left hand side operand has unsupported "
                    "number of dimensions", std::move(ctx)));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type stack_operation::handle_stack(
        primitive_arguments_type&& operands, primitive_arguments_type&& args,
        eval_context ctx) const
    {
        primitive_arguments_type ops;

        if (is_list_operand_strict(operands[0]))
        {
            auto&& r = extract_list_value_strict(
                std::move(operands[0]), name_, codename_);

            ops.reserve(r.size());
            for (auto&& op : r)
            {
                if (is_list_operand_strict(op))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "stack_operation::handle_stack",
                        generate_error_message("lists cannot be stacked"));
                }
                ops.push_back(std::move(op));
            }
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::handle_stack",
                generate_error_message("the first argument to the stack "
                    "primitive has to be a list of arrays to stack"));
        }

        std::int64_t axis = 0;
        if (operands.size() >= 2)
        {
            axis = extract_scalar_integer_value_strict(
                value_operand_sync(
                    std::move(operands[1]), args, name_, codename_, ctx),
                name_, codename_);
        }

        primitive_argument_type dtype;
        if (operands.size() >= 3 && valid(operands[2]))
        {
            dtype = value_operand_sync(
                std::move(operands[2]), args, name_, codename_, ctx);
        }

        ops = detail::map_operands(std::move(ops),
            functional::value_operand_sync{}, std::move(args), name_, codename_,
            std::move(ctx));

        std::size_t matrix_dims = extract_largest_dimension(
            ops, name_, codename_);

        switch (matrix_dims)
        {
        case 0:
            return stack0d(std::move(ops), std::move(dtype), axis);

        case 1:
            return stack1d(std::move(ops), std::move(dtype), axis);

        case 2:
            return stack2d(std::move(ops), std::move(dtype), axis);

        case 3:
            return stack3d(std::move(ops), std::move(dtype), axis);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::handle_stack",
                generate_error_message("unsupported number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> stack_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() ||
            (mode_ == stacking_mode_axis && operands.size() > 3) ||
            (mode_ != stacking_mode_axis && operands.size() > 2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::eval",
                generate_error_message(
                    "the stack_operation primitive requires to be "
                        "invoked with one or two arguments"));
        }

        if (operands.empty())
        {
            return empty_helper<double>();
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::eval",
                generate_error_message(
                    "the stack_operation primitive requires that "
                        "the arguments given by the operands array "
                        "are valid"));
        }

        auto ctx_copy = ctx;

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_), args = args, ctx = std::move(ctx)](
                    primitive_arguments_type&& ops) mutable
            -> primitive_argument_type
            {
                if (this_->mode_ != stacking_mode_axis)
                {
                    return this_->handle_hvdstack(
                        std::move(ops), std::move(args), std::move(ctx));
                }
                else
                {
                    return this_->handle_stack(
                        std::move(ops), std::move(args), std::move(ctx));
                }
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx_copy)));
    }
}}}
