// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_NODE_SLICE_NODE_DATA_0D_SEP_18_2018_1135AM)
#define PHYLANX_IR_NODE_SLICE_NODE_DATA_0D_SEP_18_2018_1135AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_assign.hpp>
#include <phylanx/execution_tree/primitives/detail/slice_identity.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/slicing_helpers.hpp>

#include <hpx/throw_exception.hpp>

#include <string>
#include <utility>

#include <blaze/Math.h>
#include <blaze/math/Elements.h>

// Slicing functionality for 0d data
namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    // Extracting slice functionality
    template <typename T, typename F>
    ir::node_data<T> slice0d_basic(T data, ir::slicing_indices const& indices,
        F const& f, std::string const& name, std::string const& codename)
    {
        if (indices.start() != 0 || indices.span() != 1 || indices.step() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::slice0d",
                util::generate_error_message(
                    "cannot extract anything but the first element from a "
                    "scalar",
                    name, codename));
        }
        return f.scalar(data, data);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice1d_extract0d(ir::node_data<T> const& data,
        execution_tree::primitive_argument_type const& indices,
        std::string const& name, std::string const& codename)
    {
        return slice0d_basic<T>(data.scalar(),
            util::slicing_helpers::extract_slicing(indices, 1, name, codename),
            detail::slice_identity<T>{}, name, codename);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> slice1d_assign0d(ir::node_data<T>&& data,
        execution_tree::primitive_argument_type const& indices,
        ir::node_data<T>&& value, std::string const& name,
        std::string const& codename)
    {
        switch (value.num_dimensions())
        {
        case 0:
            {
                typename ir::node_data<T>::storage0d_type result;
                extract_value_scalar(result, std::move(value), name, codename);

                ir::node_data<T> rhs(std::move(result));
                return slice0d_basic<T>(data.scalar(),
                    util::slicing_helpers::extract_slicing(
                        indices, 1, name, codename),
                    detail::slice_assign_scalar<T>{rhs}, name, codename);
            }

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::invalid_status,
            "phylanx::execution_tree::slice0d_assign",
            util::generate_error_message(
                "source ir::node_data object holds unsupported data type", name,
                codename));
    }
}}

#endif
