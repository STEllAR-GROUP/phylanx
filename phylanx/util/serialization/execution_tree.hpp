//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_SERIALIZE_EXECUTION_TREE_HPP)
#define PHYLANX_SERIALIZE_EXECUTION_TREE_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>

#include <vector>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    PHYLANX_EXPORT std::vector<char> serialize(
        execution_tree::primitive_argument_type const&);
    PHYLANX_EXPORT std::vector<char> serialize(
        execution_tree::primitive_result_type const&);

    PHYLANX_EXPORT void unserialize(
        std::vector<char> const&, execution_tree::primitive_argument_type&);
    PHYLANX_EXPORT void unserialize(
        std::vector<char> const&, execution_tree::primitive_result_type&);
}}

#endif

