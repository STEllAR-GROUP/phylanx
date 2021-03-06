// Copyright (c) 2019-2020 Bita Hasheminezhad
// Copyright (c) 2019-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_CONV1D_OPERATION)
#define PHYLANX_COMMON_CONV1D_OPERATION

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>

#include <cstdint>
#include <string>

namespace phylanx { namespace common {

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type conv1d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type conv1d_valid(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    conv1d_valid_dilation(ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel, std::int64_t dilation_rate);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type conv1d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type conv1d_same(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    conv1d_same_dilation(ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel, std::int64_t dilation_rate);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type conv1d_causal(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type conv1d_causal(
        ir::node_data<double>&& arg, ir::node_data<double>&& kernel,
        std::int64_t strides);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    conv1d_causal_dilation(ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel, std::int64_t dilation_rate);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    conv1d_all_paddings(ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel, std::string&& padding,
        std::string const& name, std::string const& codename);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    conv1d_all_paddings(ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel, std::string&& padding,
        std::int64_t strides, std::string const& name,
        std::string const& codename);
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    conv1d_all_paddings_dilation(ir::node_data<double>&& arg,
        ir::node_data<double>&& kernel, std::string&& padding,
        std::int64_t dilation_rate, std::string const& name,
        std::string const& codename);

}}    // namespace phylanx::common

#endif
