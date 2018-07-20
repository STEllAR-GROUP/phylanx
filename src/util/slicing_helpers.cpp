////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////


#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/util/slicing_helpers.hpp>
#include <phylanx/ir/node_data.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace phylanx { namespace util {
  namespace slicing_helpers {

    std::int64_t extract_integer_value_local(
        execution_tree::primitive_argument_type const& val,
        std::int64_t default_value)
    {
        if (execution_tree::valid(val))
        {
            return execution_tree::extract_scalar_integer_value(
                val);
        }
        return default_value;
    }

    std::vector<std::int64_t> extract_slicing(
        execution_tree::primitive_argument_type&& arg,
        std::size_t arg_size)
    {
        std::vector<std::int64_t> indices;

        // Extract the list or the single integer index
        // from second argument (row-> start, stop, step)
        if (execution_tree::is_list_operand_strict(arg))
        {
            auto arg_list =
                execution_tree::extract_list_value(std::move(arg));

            std::size_t size = arg_list.size();
            if (size > 3)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                    "slicing_operation::extract_list_slicing_args",
                    execution_tree::generate_error_message("too many indicies given"));
            }

            auto it = arg_list.begin();

            // default first index is '0'
            if (size > 0)
            {
                indices.push_back(extract_integer_value_local(*it, 0));
            }
            else
            {
                indices.push_back(0);
            }

            // default last index is 'size'
            if (size > 1)
            {
                indices.push_back(extract_integer_value_local(*++it, arg_size));
            }
            else
            {
                indices.push_back(arg_size);
            }

            // default step is '1'
            if (size > 2)
            {
                indices.push_back(extract_integer_value_local(*++it, 1ll));
            }
            else
            {
                indices.push_back(1ll);
            }
        }
        else if (!execution_tree::valid(arg))
        {
            // no arguments given means return all of the argument
            indices.push_back(0);
            indices.push_back(arg_size);
            indices.push_back(1ll);
        }
        else
        {
            indices.push_back(execution_tree::extract_scalar_integer_value(
                std::move(arg)));
        }

        return indices;
    }

  }
}}
