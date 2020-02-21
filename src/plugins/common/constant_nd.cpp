// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/constant_nd.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/matrixops/constant.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/errors/throw_exception.hpp>

#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common
{
    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant0d_helper(
        execution_tree::primitive_argument_type&& op, std::string const& name_,
        std::string const& codename_)
    {
        if (valid(op))
        {
            return ir::node_data<T>{execution_tree::extract_scalar_data<T>(
                std::move(op), name_, codename_)};
        }

        // create an empty scalar
        return ir::node_data<T>{T{}};
    }

    execution_tree::primitive_argument_type constant0d(
        execution_tree::primitive_argument_type&& op,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        //if (dtype == node_data_type_unknown)
        //{
        //    HPX_ASSERT(implements_like_operations_);
        //    dtype = extract_common_type(op);
        //}

        switch (dtype)
        {
        case node_data_type_bool:
            return constant0d_helper<std::uint8_t>(
                std::move(op), name, codename);

        case node_data_type_int64:
            return constant0d_helper<std::int64_t>(
                std::move(op), name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant0d_helper<double>(std::move(op), name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::constant_nd::"
                "constant0d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant1d_helper(
        execution_tree::primitive_argument_type&& op, std::size_t dim,
        std::string const& name_, std::string const& codename_)
    {
        if (valid(op))
        {
            return ir::node_data<T>{blaze::UniformVector<T>(dim,
                execution_tree::extract_scalar_data<T>(
                    std::move(op), name_, codename_))};
        }

        // create an empty vector
        return ir::node_data<T>{blaze::UniformVector<T>(dim)};
    }

    execution_tree::primitive_argument_type constant1d(
        execution_tree::primitive_argument_type&& op, std::size_t dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        //if (dtype == node_data_type_unknown)
        //{
        //    HPX_ASSERT(implements_like_operations_);
        //    dtype = extract_common_type(op);
        //}

        switch (dtype)
        {
        case node_data_type_bool:
            return constant1d_helper<std::uint8_t>(
                std::move(op), dim, name, codename);

        case node_data_type_int64:
            return constant1d_helper<std::int64_t>(
                std::move(op), dim, name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant1d_helper<double>(
                std::move(op), dim, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::constant_nd::"
                "constant1d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant2d_helper(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, std::string const& name_,
        std::string const& codename_)
    {
        if (valid(op))
        {
            return ir::node_data<T>{blaze::UniformMatrix<T>(dim[0], dim[1],
                execution_tree::extract_scalar_data<T>(
                    std::move(op), name_, codename_))};
        }

        // create an empty matrix
        return ir::node_data<T>{blaze::UniformMatrix<T>(dim[0], dim[1])};
    }

    execution_tree::primitive_argument_type constant2d(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        //if (dtype == node_data_type_unknown)
        //{
        //    HPX_ASSERT(implements_like_operations_);
        //    dtype = extract_common_type(op);
        //}

        switch (dtype)
        {
        case node_data_type_bool:
            return constant2d_helper<std::uint8_t>(
                std::move(op), dim, name, codename);

        case node_data_type_int64:
            return constant2d_helper<std::int64_t>(
                std::move(op), dim, name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant2d_helper<double>(
                std::move(op), dim, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::constant_nd::"
                "constant2d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant3d_helper(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, std::string const& name_,
        std::string const& codename_)
    {
        if (valid(op))
        {
            return ir::node_data<T>{
                blaze::UniformTensor<T>(dim[0], dim[1], dim[2],
                    execution_tree::extract_scalar_data<T>(
                        std::move(op), name_, codename_))};
        }

        // create an empty tensor
        return ir::node_data<T>{
            blaze::UniformTensor<T>(dim[0], dim[1], dim[2])};
    }

    execution_tree::primitive_argument_type constant3d(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        //if (dtype == node_data_type_unknown)
        //{
        //    HPX_ASSERT(implements_like_operations_);
        //    dtype = extract_common_type(op);
        //}

        switch (dtype)
        {
        case node_data_type_bool:
            return constant3d_helper<std::uint8_t>(
                std::move(op), dim, name, codename);

        case node_data_type_int64:
            return constant3d_helper<std::int64_t>(
                std::move(op), dim, name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant3d_helper<double>(
                std::move(op), dim, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::constant_nd::"
                "constant3d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant4d_helper(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, std::string const& name_,
        std::string const& codename_)
    {
        if (valid(op))
        {
            auto a = ir::node_data<T>{
                blaze::DynamicArray<4UL, T>(blaze::init_from_value,
                    execution_tree::extract_scalar_data<T>(
                        std::move(op), name_, codename_),
                    dim[0], dim[1], dim[2], dim[3])};
            auto b = execution_tree::extract_scalar_data<T>(
                std::move(op), name_, codename_);
            return ir::node_data<T>{
                blaze::DynamicArray<4UL, T>(blaze::init_from_value,
                    execution_tree::extract_scalar_data<T>(
                        std::move(op), name_, codename_),
                    dim[0], dim[1], dim[2], dim[3])};
        }

        // create an empty 4d array
        return ir::node_data<T>{
            blaze::DynamicArray<4UL, T>(dim[0], dim[1], dim[2], dim[3])};
    }

    execution_tree::primitive_argument_type constant4d(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename)
    {
        using namespace execution_tree;

        //if (dtype == node_data_type_unknown)
        //{
        //    HPX_ASSERT(implements_like_operations_);
        //    dtype = extract_common_type(op);
        //}

        switch (dtype)
        {
        case node_data_type_bool:
            return constant4d_helper<std::uint8_t>(
                std::move(op), dim, name, codename);

        case node_data_type_int64:
            return constant4d_helper<std::int64_t>(
                std::move(op), dim, name, codename);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant4d_helper<double>(
                std::move(op), dim, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::constant_nd::"
                "constant4d",
            util::generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }
}}

