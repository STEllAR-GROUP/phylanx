// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_DETAIL_ADVANCED_INDEXING_SEP_18_2018_1214PM)
#define PHYLANX_DETAIL_ADVANCED_INDEXING_SEP_18_2018_1214PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include <blaze/Math.h>

namespace phylanx { namespace execution_tree { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    HPX_FORCEINLINE std::int64_t
    check_index(std::int64_t index, std::size_t size, std::string const& name,
        std::string const& codename)
    {
        if (index < 0)
        {
            index += size;
        }

#if defined(PHYLANX_DEBUG)
        if (index < 0 || index >= std::int64_t(size))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::detail::check_index",
                util::generate_error_message(
                    "index out of range", name, codename));
        }
#endif
        return index;
    }

    ///////////////////////////////////////////////////////////////////////////
    // An advanced slicing index is a list with one or two integer or
    // boolean array arguments.
    enum slicing_index_type
    {
        slicing_index_basic,
        slicing_index_advanced_integer,
        slicing_index_advanced_boolean
    };

    slicing_index_type is_advanced_slicing_index(
        primitive_argument_type const& indices)
    {
        ir::range const& list = util::get<7>(indices);
        if (list.size() == 1)
        {
            if (is_boolean_operand_strict(*list.begin()))
            {
                return slicing_index_advanced_boolean;
            }

            if (is_integer_operand_strict(*list.begin()))
            {
                return slicing_index_advanced_integer;
            }
        }
        return slicing_index_basic;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Normally an advanced integer slicing index is provided as a
    // integer array wrapped into a list.
    primitive_argument_type extract_advanced_integer_index(
        primitive_argument_type const& indices, std::string const& name,
        std::string const& codename)
    {
        ir::range const& list = util::get<7>(indices);
        return extract_integer_value_strict(*list.begin(), name, codename);
    }

    // Normally an advanced boolean slicing index is provided as a
    // boolean array wrapped into a list.
    primitive_argument_type extract_advanced_boolean_index(
        primitive_argument_type const& indices, std::string const& name,
        std::string const& codename)
    {
        ir::range const& list = util::get<7>(indices);
        return extract_boolean_value_strict(*list.begin(), name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t extract_advanced_index_size(
        primitive_argument_type const& indices, std::string const& name,
        std::string const& codename)
    {
        ir::range const& list = util::get<7>(indices);
        return extract_numeric_value_dimensions(
            *list.begin(), name, codename)[0];
    }

    ///////////////////////////////////////////////////////////////////////////
    std::size_t slicing_size(
        execution_tree::primitive_argument_type const& indices,
        std::size_t arg_size, std::string const& name,
        std::string const& codename)
    {
        if (is_list_operand_strict(indices))
        {
            slicing_index_type t = is_advanced_slicing_index(indices);

            if (t == slicing_index_basic)
            {
                // basic slicing and indexing
                return util::slicing_helpers::slicing_size(
                    indices, arg_size, name, codename);
            }

            if (t == slicing_index_advanced_integer)
            {
                return extract_advanced_index_size(indices, name, codename);
            }

            if (t == slicing_index_advanced_boolean)
            {
                return arg_size;
            }
        }
        else if (valid(indices))
        {
            if (is_boolean_operand_strict(indices))
            {
                // advanced indexing (boolean array indexing)
                return arg_size;
            }

            if (is_integer_operand(indices))
            {
                // advanced indexing (integer array indexing)
                return extract_numeric_value_dimensions(
                    indices, name, codename)[0];
            }
        }
        else
        {
            return arg_size;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::slicing_size",
            util::generate_error_message(
                "unsupported indexing type", name, codename));
    }
}}}

#endif
