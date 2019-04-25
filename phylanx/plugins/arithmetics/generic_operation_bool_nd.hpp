// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_BOOL_ND_HPP
#define PHYLANX_GENERIC_OPERATION_BOOL_ND_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/parse_primitive_name.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_bool.hpp>

#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    generic_operation_bool::scalar_function_ptr<T>
    generic_operation_bool::get_0d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_0d_map<T>();
        auto it = func_map.find(funcname);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation_bool::get_0d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    generic_operation_bool::matrix_vector_function_ptr<T>
    generic_operation_bool::get_1d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_1d_map<T>();
        auto it = func_map.find(funcname);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation_bool::get_1d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    generic_operation_bool::matrix_vector_function_ptr<T>
    generic_operation_bool::get_2d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_2d_map<T>();
        auto it = func_map.find(funcname);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation_bool::get_2d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    generic_operation_bool::matrix_vector_function_ptr<T>
    generic_operation_bool::get_3d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_3d_map<T>();
        auto it = func_map.find(funcname);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation_bool::get_3d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }
#endif
}}}

#endif
