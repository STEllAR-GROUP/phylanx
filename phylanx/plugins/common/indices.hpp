// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>

#include <string>

namespace phylanx { namespace common {

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type indices0d(
        ir::range const& shape, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type indices1d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type indices2d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type indices3d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type indices4d(
        ir::range const& shape, execution_tree::node_data_type dtype,
        std::string const& name, std::string const& codename,
        execution_tree::eval_context ctx);

    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    sparse_indices0d(ir::range const& shape, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    sparse_indices1d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    sparse_indices2d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    sparse_indices3d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    sparse_indices4d(ir::range const& shape,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx);

}}    // namespace phylanx::common
