// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    node_data_type extract_common_type(primitive_argument_type const& arg)
    {
        node_data_type result = node_data_type_bool;
        if (is_numeric_operand_strict(arg))
        {
            result = node_data_type_double;
        }
        else if (is_integer_operand_strict(arg))
        {
            result = node_data_type_int64;
        }
        else if (!is_boolean_operand_strict(arg))
        {
            result = node_data_type_unknown;
        }
        return result;
    }
}}
