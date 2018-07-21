// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_RANGE_SLICING_JUL_20_2018_1211PM)
#define PHYLANX_IR_RANGE_SLICING_JUL_20_2018_1211PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/ir/ranges.hpp>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // extract a slice from the given range instance
    PHYLANX_EXPORT primitive_argument_type slice(ir::range const& data,
        primitive_argument_type const& indices,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    ///////////////////////////////////////////////////////////////////////////
    // modify a slice of the given range instance
    PHYLANX_EXPORT primitive_argument_type slice(ir::range&& data,
        primitive_argument_type const& indices,
        ir::range&& value, std::string const& name = "",
        std::string const& codename = "<unknown>");
}}

#endif
