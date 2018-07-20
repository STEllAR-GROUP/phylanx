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
#include <utility>
#include <vector>

namespace phylanx { namespace util {
  namespace slicing_helpers
  {

    std::int64_t extract_integer_value(
        execution_tree::primitive_argument_type const& val,
        std::int64_t default_value);

    std::vector<std::int64_t> extract_slicing(
        execution_tree::primitive_argument_type&& arg,
        std::size_t arg_size);

  }
}}

#endif //PHYLANX_SLICING_HELPERS_HPP
