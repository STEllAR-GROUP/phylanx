// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_DATA_SLICING_JUL_19_2018_1248PM)
#define PHYLANX_IR_NODE_DATA_SLICING_JUL_19_2018_1248PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>

#include <string>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // extract a slice from the given node_data instance
    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> slice_extract(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
#endif

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given node_data instance
    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name = "",
        std::string const& codename = "<unknown>");

    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name = "",
        std::string const& codename = "<unknown>");

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    PHYLANX_EXPORT ir::node_data<T> slice_assign(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& pages,
        execution_tree::primitive_argument_type const& rows,
        execution_tree::primitive_argument_type const& columns,
        ir::node_data<T>&& value, std::string const& name = "",
        std::string const& codename = "<unknown>");
#endif
}}

#endif
