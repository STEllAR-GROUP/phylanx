//  Copyright (c) 2017-2018 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/plugins/common/dot_operation_nd_impl.hpp>

#include <cstdint>
#include <string>

///////////////////////////////////////////////////////////////////////////////
// explicitly instantiate the required functions
namespace phylanx { namespace common
{
    ///////////////////////////////////////////////////////////////////////////
    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d(
        ir::node_data<std::uint8_t>&&, ir::node_data<std::uint8_t>&&,
        std::string const&, std::string const&);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d(
        ir::node_data<std::uint8_t>&&, ir::node_data<std::uint8_t>&&,
        std::string const&, std::string const&);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d(
        ir::node_data<std::uint8_t>&&, ir::node_data<std::uint8_t>&&,
        std::string const&, std::string const&);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d(
        ir::node_data<std::uint8_t>&&, ir::node_data<std::uint8_t>&&,
        std::string const&, std::string const&);
#endif

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    dot2d2d(ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs, std::string const& name,
        std::string const& codename);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    dot2dt2d(ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs, std::string const& name,
        std::string const& codename);

    template PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type
    dot2d2dt(ir::node_data<std::uint8_t>&& lhs,
        ir::node_data<std::uint8_t>&& rhs, std::string const& name,
        std::string const& codename);
}}
