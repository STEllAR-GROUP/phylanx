////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2018 Bibek Wagle
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#ifndef PHYLANX_SLICING_HELPERS_HPP
#define PHYLANX_SLICING_HELPERS_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/primitive_component_base.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace phylanx { namespace util { namespace slicing_helpers
{
    ///////////////////////////////////////////////////////////////////////////
    // extract a single integer from the given node_data instance
    PHYLANX_EXPORT std::int64_t extract_integer(
        execution_tree::primitive_argument_type const& val,
        std::int64_t default_value, std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    // generate a list of indices to extract from a given vector
    PHYLANX_EXPORT std::vector<std::size_t> create_list_slice(
        std::int64_t start, std::int64_t stop, std::int64_t step);

    ///////////////////////////////////////////////////////////////////////////
    // extract a list of indices that correspond to the given slicing parameters
    PHYLANX_EXPORT ir::slicing_indices extract_slicing(
        execution_tree::primitive_argument_type const& arg,
        std::size_t arg_size, std::string const& name = "",
        std::string const& codename = "<unknown>");

    PHYLANX_EXPORT std::size_t slicing_size(
        execution_tree::primitive_argument_type const& arg,
        std::size_t arg_size, std::string const& name = "",
        std::string const& codename = "<unknown>");
}}}

#endif //PHYLANX_SLICING_HELPERS_HPP
