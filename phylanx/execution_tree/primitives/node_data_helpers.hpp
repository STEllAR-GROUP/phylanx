// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_EXECUTION_TREE_NODE_DATA_HELPERS_AUG_05_2018_0446PM)
#define PHYLANX_EXECUTION_TREE_NODE_DATA_HELPERS_AUG_05_2018_0446PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    enum node_data_type
    {
        node_data_type_unknown = -1,
        node_data_type_double = 0,
        node_data_type_int64 = 1,
        node_data_type_bool = 2,
    };

    /// Return the common data type to be used for the result of an operation
    /// involving the given argument.
    PHYLANX_EXPORT node_data_type extract_common_type(
        primitive_argument_type const& args);
}}

#endif
