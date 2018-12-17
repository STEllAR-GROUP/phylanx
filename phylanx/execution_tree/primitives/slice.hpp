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

#include <string>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // return a slice of the given node_data instance
    PHYLANX_EXPORT primitive_argument_type slice(
        primitive_argument_type const& data,
        primitive_argument_type const& indices, std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT primitive_argument_type slice(
        primitive_argument_type const& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, std::string const& name = "",
        std::string const& codename = "<unknown>");

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    PHYLANX_EXPORT primitive_argument_type slice(
        primitive_argument_type const& data,
        primitive_argument_type const& pages,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns,
        std::string const& name = "",
        std::string const& codename = "<unknown>");
#endif

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given primitive_argument_type instance
    PHYLANX_EXPORT primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& indices, primitive_argument_type&& value,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns, primitive_argument_type&& value,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    PHYLANX_EXPORT primitive_argument_type slice(primitive_argument_type&& data,
        primitive_argument_type const& pages,
        primitive_argument_type const& rows,
        primitive_argument_type const& columns,
        primitive_argument_type&& value, std::string const& name = "",
        std::string const& codename = "<unknown>");
#endif
}}

#endif
