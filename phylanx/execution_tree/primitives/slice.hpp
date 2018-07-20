// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_SLICING_JUL_19_2018_1248PM)
#define PHYLANX_EXECUTION_TREE_SLICING_JUL_19_2018_1248PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/ranges.hpp>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given node_data instance
    PHYLANX_EXPORT execution_tree::primitive_argument_type slice(
        execution_tree::primitive_argument_type const& data,
        ir::range const& indices);

    PHYLANX_EXPORT execution_tree::primitive_argument_type slice(
        execution_tree::primitive_argument_type const& data,
        ir::range const& rows, ir::range const& columns);
}}

#endif
