// Copyright (c) 2017-2020 Hartmut Kaiser
// Copyright (c) 2020 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_CONSTANT_ND)
#define PHYLANX_COMMON_CONSTANT_ND

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>

#include <cstdint>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common
{
    using operand_type = ir::node_data<double>;

    template <typename T>
    PHYLANX_COMMON_EXPORT ir::node_data<T> constant0d_helper(
        execution_tree::primitive_argument_type&& op, std::string const& name,
        std::string const& codename);
    template <typename T>
    PHYLANX_COMMON_EXPORT ir::node_data<T> constant1d_helper(
        execution_tree::primitive_argument_type&& op, std::size_t dim,
        std::string const& name, std::string const& codename);
    template <typename T>
    PHYLANX_COMMON_EXPORT ir::node_data<T> constant2d_helper(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, std::string const& name,
        std::string const& codename);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type constant0d(
        execution_tree::primitive_argument_type&& op,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type constant1d(
        execution_tree::primitive_argument_type&& op, std::size_t dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type constant2d(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT ir::node_data<T> constant3d_helper(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, std::string const& name,
        std::string const& codename);
    template <typename T>
    PHYLANX_COMMON_EXPORT ir::node_data<T> constant4d_helper(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, std::string const& name,
        std::string const& codename);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type constant3d(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type constant4d(
        execution_tree::primitive_argument_type&& op,
        operand_type::dimensions_type const& dim,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename);

}}

#endif
