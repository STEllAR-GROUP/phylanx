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
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
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
    std::vector<match_pattern_type> const stack_operation::match_data =
    {
        match_pattern_type{
            "hstack",
            std::vector<std::string>{"hstack(__1)"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args
            Args:

                *args (list, optional) : a list of array-like objects

            Returns:

            A horizontally (column wise) stacked sequence of array-like objects)",
            true
        },
        match_pattern_type{
            "vstack",
            std::vector<std::string>{"vstack(__1)"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args
            Args:

                *args (list, optional) : a list of array-like objects

            Returns:

            A vertically (row wise) stacked sequence of array-like objects)",
            true
        },
        match_pattern_type{
            "stack",
            std::vector<std::string>{"stack(_1)", "stack(_1, _2)"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args, axis
            Args:

                *args (list, optional) : a list of array-like objects
                axis (int, optional) : the axis along which to stack input values

            Returns:

            A joined sequence of array-like objects along a new axis.)",
            true
        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
      , match_pattern_type{
            "dstack",
            std::vector<std::string>{"dstack(__1)"},
            &create_stack_operation, &create_primitive<stack_operation>, R"(
            args
            Args:

                *args (list, optional) : a list of array-like objects

            Returns:

            A vertically (depth wise) stacked sequence of array-like objects)",
            true
        }
#endif
    };

    ///////////////////////////////////////////////////////////////////////////
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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        else if (name.find("dstack") != std::string::npos)
        {
            result = stack_operation::stacking_mode_depth_wise;
        }
#endif
        return result;
    }

    stack_operation::stack_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
      , mode_(extract_stacking_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    std::size_t stack_operation::get_vecsize(
        primitive_arguments_type const& args) const
    {
        std::size_t vec_size = 0;

        for (std::size_t i = 0; i != args.size(); ++i)
        {
            std::size_t num_dims =
                extract_numeric_value_dimension(args[i], name_, codename_);

            if (num_dims != 0 && num_dims != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "stack_operation::get_vecsize",
                    generate_error_message(
                        "for 0d/1d stacking, the stack_operation primitive "
                        "requires the input be either a vector or a scalar"));
            }

            if (num_dims == 0)
            {
                ++vec_size;
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
        blaze::DynamicVector<T> result(get_vecsize(args));

        auto iter = result.begin();
        for (auto && arg : args)
        {
            auto && val = extract_node_data<T>(std::move(arg));
            std::size_t num_d = val.num_dimensions();
            if (num_d == 0)
            {
                *iter++ = val.scalar();
            }
            else
            {
                std::copy(val.vector().begin(), val.vector().end(), iter);
                iter += val.size();
            }
        }

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type stack_operation::hstack0d1d(
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::hstack3d_helper(
        primitive_arguments_type&& args) const
    {
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

        blaze::DynamicTensor<double> result(num_pages, total_rows, num_cols);

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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::vstack0d_helper(
        primitive_arguments_type&& args) const
    {
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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type stack_operation::vstack3d_helper(
        primitive_arguments_type&& args) const
    {
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

        blaze::DynamicTensor<double> result(total_pages, num_rows, num_cols);

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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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
        std::size_t vec_size = args.size();

        blaze::DynamicTensor<T> result(1, 1, vec_size);
        auto row = blaze::row(blaze::pageslice(result, 0), 0);

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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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

        blaze::DynamicTensor<double> result(1, size, args_size);
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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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

        blaze::DynamicTensor<double> result(num_rows, num_cols, total_columns);

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
        primitive_arguments_type&& args) const
    {
        node_data_type t = dtype_;
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
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type stack_operation::stack0d(
        primitive_arguments_type&& args) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack0d1d(std::move(args));

        case stacking_mode_row_wise:
            return vstack0d(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case stacking_mode_depth_wise:
            return dstack0d(std::move(args));
#endif
        case stacking_mode_axis:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack0d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

    primitive_argument_type stack_operation::stack1d(
        primitive_arguments_type&& args) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack0d1d(std::move(args));

        case stacking_mode_row_wise:
            return vstack1d2d(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case stacking_mode_depth_wise:
            return dstack1d(std::move(args));
#endif
        case stacking_mode_axis:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack1d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

    primitive_argument_type stack_operation::stack2d(
        primitive_arguments_type&& args) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack2d(std::move(args));

        case stacking_mode_row_wise:
            return vstack1d2d(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case stacking_mode_depth_wise:
            return dstack2d3d(std::move(args));
#endif
        case stacking_mode_axis:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack2d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type stack_operation::stack3d(
        primitive_arguments_type&& args) const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            return hstack3d(std::move(args));

        case stacking_mode_row_wise:
            return vstack3d(std::move(args));

        case stacking_mode_depth_wise:
            return dstack2d3d(std::move(args));

        case stacking_mode_axis:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::stack3d",
            generate_error_message(
                "unsupported stacking mode requested"));
    }
#endif

    template <typename T>
    hpx::future<primitive_argument_type> stack_operation::empty_helper() const
    {
        switch (mode_)
        {
        case stacking_mode_column_wise:
            {
                // hstack() without arguments returns an empty 1D vector
                using storage1d_type = typename ir::node_data<T>::storage1d_type;
                return hpx::make_ready_future(primitive_argument_type{
                    ir::node_data<T>{storage1d_type(0)}});
            }

        case stacking_mode_row_wise:
            {
                // vstack() without arguments returns an empty 2D matrix
                using storage2d_type = typename ir::node_data<T>::storage2d_type;
                return hpx::make_ready_future(primitive_argument_type{
                    ir::node_data<T>{storage2d_type(0, 0)}});
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case stacking_mode_depth_wise:
            {
                // dstack() without arguments returns an empty 3D tensor
                using storage3d_type = typename ir::node_data<T>::storage3d_type;
                return hpx::make_ready_future(primitive_argument_type{
                    ir::node_data<T>{storage3d_type(0, 0, 0)}});
            }
#endif

        case stacking_mode_axis:
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "stack_operation::empty",
            generate_error_message("unsupported stacking mode requested"));
    }

    hpx::future<primitive_argument_type> stack_operation::empty() const
    {
        switch (dtype_)
        {
        case node_data_type_bool:
            return empty_helper<std::uint8_t>();

        case node_data_type_int64:
            return empty_helper<std::int64_t>();

        case node_data_type_unknown:
        case node_data_type_double:
            return empty_helper<double>();

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::stack_operation::empty",
            generate_error_message("invalid dtype detected"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> stack_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (mode_ == stacking_mode_axis && (operands.empty() ||
            operands.size() > 2))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::eval",
                generate_error_message(
                    "the stack_operation primitive requires to be "
                        "invoked with one or two arguments "));
        }

        if (operands.empty())
        {
            return empty();
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "stack_operation::eval",
                generate_error_message(
                    "the stack_operation primitive requires that "
                        "the arguments given by the operands array "
                        "are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                if (is_list_operand_strict(args[0]))
                {
                    args = extract_list_value_strict(args[0],
                        this_->name_, this_->codename_).args();
                }

                std::size_t matrix_dims = extract_largest_dimension(
                    args, this_->name_, this_->codename_);
                switch (matrix_dims)
                {
                case 0:
                    return this_->stack0d(std::move(args));

                case 1:
                    return this_->stack1d(std::move(args));

                case 2:
                    return this_->stack2d(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->stack3d(std::move(args));
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "stack_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
