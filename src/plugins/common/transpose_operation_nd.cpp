// Copyright (c) 2017-2019 Hartmut Kaiser
// Copyright (c) 2019 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/common/transpose_operation_nd.hpp>
#include <phylanx/plugins/matrixops/transpose_operation.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/throw_exception.hpp>

#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common
{
    ////////////////////////////////////////////////////////////////////////////
    execution_tree::primitive_argument_type transpose0d1d(
        execution_tree::primitive_argument_type&& arg)
    {
        return execution_tree::primitive_argument_type{std::move(arg)};       // no-op
    }

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type transpose2d(ir::node_data<T>&& arg)
    {
        if (arg.is_ref())
        {
            arg = blaze::trans(arg.matrix());
        }
        else
        {
            blaze::transpose(arg.matrix_non_ref());
        }

        return execution_tree::primitive_argument_type{std::move(arg)};
    }

    execution_tree::primitive_argument_type transpose2d(
        execution_tree::primitive_argument_type&& arg,
        std::string const& name, std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(
                extract_boolean_value_strict(std::move(arg), name, codename));

        case node_data_type_int64:
            return transpose2d(
                extract_integer_value_strict(std::move(arg), name, codename));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(
                extract_numeric_value(std::move(arg), name, codename));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose2d",
            util::generate_error_message(
                "the transpose primitive requires for its argument to "
                    "be numeric data type", name, codename));
    }

    template <typename T>
    execution_tree::primitive_argument_type transpose2d(
        ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes)
    {
        auto v = axes.vector();
        for (auto& it : v)
        {
            if (it < 0)
                it += 2;
        }

        if (v[0] == 0 && v[1] == 1)
            return execution_tree::primitive_argument_type{std::move(arg)};

        return transpose2d(std::move(arg));
    }

    execution_tree::primitive_argument_type transpose2d(
        execution_tree::primitive_argument_type&& arg,
        ir::node_data<std::int64_t>&& axes, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose2d(
                extract_boolean_value_strict(std::move(arg), name, codename),
                std::move(axes));

        case node_data_type_int64:
            return transpose2d(
                extract_integer_value_strict(std::move(arg), name, codename),
                std::move(axes));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose2d(
                extract_numeric_value(std::move(arg), name, codename),
                std::move(axes));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose2d",
            util::generate_error_message(
                "the transpose primitive requires for its argument to "
                "be numeric data type", name, codename));
    }

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    execution_tree::primitive_argument_type transpose3d(ir::node_data<T>&& arg)
    {
        if (arg.is_ref())
            arg = blaze::trans(arg.tensor(), {2, 1, 0});

        else
            blaze::transpose(arg.tensor_non_ref(), {2, 1, 0});

        return execution_tree::primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    execution_tree::primitive_argument_type transpose3d_axes102(
        ir::node_data<T>&& arg)
    {
        if (arg.is_ref())
            arg = blaze::trans(arg.tensor(), {1, 0, 2});

        else
            blaze::transpose(arg.tensor_non_ref(), {1, 0, 2});

        return execution_tree::primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    execution_tree::primitive_argument_type transpose3d_axes021(
        ir::node_data<T>&& arg)
    {
        if (arg.is_ref())
            arg = blaze::trans(arg.tensor(), {0, 2, 1});

        else
            blaze::transpose(arg.tensor_non_ref(), {0, 2, 1});

        return execution_tree::primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    execution_tree::primitive_argument_type transpose3d_axes120(
        ir::node_data<T>&& arg)
    {
        if (arg.is_ref())
            arg = blaze::trans(arg.tensor(), {1, 2, 0});

        else
            blaze::transpose(arg.tensor_non_ref(), {1, 2, 0});

        return execution_tree::primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    execution_tree::primitive_argument_type transpose3d_axes201(
        ir::node_data<T>&& arg)
    {
        if (arg.is_ref())
            arg = blaze::trans(arg.tensor(), {2, 0, 1});

        else
            blaze::transpose(arg.tensor_non_ref(), {2, 0, 1});

        return execution_tree::primitive_argument_type{std::move(arg)};
    }

    execution_tree::primitive_argument_type transpose3d(
        execution_tree::primitive_argument_type&& arg, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose3d(
                extract_boolean_value_strict(std::move(arg), name, codename));

        case node_data_type_int64:
            return transpose3d(
                extract_integer_value_strict(std::move(arg), name, codename));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose3d(
                extract_numeric_value(std::move(arg), name, codename));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose3d",
            util::generate_error_message(
                "the transpose primitive requires for its argument to "
                    "be numeric data type", name, codename));
    }

    template <typename T>
    execution_tree::primitive_argument_type transpose3d(
        ir::node_data<T>&& arg, ir::node_data<std::int64_t>&& axes)
    {
        auto v = axes.vector();
        for (auto& it : v)
        {
            if (it < 0)
                it += 3;
        }

        if (v[0] == 0 && v[1] == 1 && v[2] == 2)
            return execution_tree::primitive_argument_type{std::move(arg)};
        else if (v[0] == 2 && v[1] == 1 && v[2] == 0)
            return transpose3d(std::move(arg));
        else if (v[0] == 1 && v[1] == 0 && v[2] == 2)
            return transpose3d_axes102(std::move(arg));
        else if (v[0] == 0 && v[1] == 2 && v[2] == 1)
            return transpose3d_axes021(std::move(arg));
        else if (v[0] == 1 && v[1] == 2 && v[2] == 0)
            return transpose3d_axes120(std::move(arg));

        return transpose3d_axes201(std::move(arg));
    }

    execution_tree::primitive_argument_type transpose3d(
        execution_tree::primitive_argument_type&& arg,
        ir::node_data<std::int64_t>&& axes, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return transpose3d(
                extract_boolean_value_strict(std::move(arg), name, codename),
                std::move(axes));

        case node_data_type_int64:
            return transpose3d(
                extract_integer_value_strict(std::move(arg), name, codename),
                std::move(axes));

        case node_data_type_unknown:
            HPX_FALLTHROUGH;
        case node_data_type_double:
            return transpose3d(
                extract_numeric_value(std::move(arg), name, codename),
                std::move(axes));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "transpose3d",
            util::generate_error_message(
                "the transpose primitive requires for its argument to "
                "be numeric data type", name, codename));
    }
}}

