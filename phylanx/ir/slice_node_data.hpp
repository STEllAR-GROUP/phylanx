// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_SLICING_JUL_19_2018_1248PM)
#define PHYLANX_IR_SLICING_JUL_19_2018_1248PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>

namespace phylanx { namespace ir
{
    ///////////////////////////////////////////////////////////////////////////
    // extract a slice from the given node_data instance
    template <typename T>
    PHYLANX_EXPORT node_data<T> slice(node_data<T> const& data,
        ir::range const& indices);

    template <typename T>
    PHYLANX_EXPORT node_data<T> slice(node_data<T> const& data,
        ir::range const& rows, ir::range const& columns);

    // modify a slice of the given node_data instance
    template <typename T>
    PHYLANX_EXPORT node_data<T> slice(
        node_data<T>&& data, ir::range const& indices, node_data<T>&& value);

    template <typename T>
    PHYLANX_EXPORT node_data<T> slice(node_data<T>&& data,
        ir::range const& rows, ir::range const& columns,
        node_data<T>&& value);
}}

#endif
