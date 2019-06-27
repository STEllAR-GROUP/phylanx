//  Copyright (c) 2017-2019 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2019 Bita Hasheminezhad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIXOPS_DOT_OPERATIONS_ND_JUN_26_2019_1123AM)
#define PHYLANX_MATRIXOPS_DOT_OPERATIONS_ND_JUN_26_2019_1123AM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/export_definitions.hpp>
#include <phylanx/util/generate_error_message.hpp>

#include <hpx/throw_exception.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);
#endif

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);
#endif

    // lhs_num_dims == 1
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2dt2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d2dt(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);
#endif

    // lhs_num_dims == 2
    // Multiply a matrix with a vector
    // Regular matrix multiplication
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    // lhs_num_dims == 3
    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d0d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d1d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d2d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);

    template <typename T>
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d3d(
        ir::node_data<T>&& lhs, ir::node_data<T>&& rhs, std::string const& name,
        std::string const& codename);
#endif

    ////////////////////////////////////////////////////////////////////////////
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot0d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot1d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename);

    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot2d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    PHYLANX_COMMON_EXPORT execution_tree::primitive_argument_type dot3d(
        execution_tree::primitive_argument_type&& lhs,
        execution_tree::primitive_argument_type&& rhs, std::string const& name,
        std::string const& codename);
#endif
}}

#endif
