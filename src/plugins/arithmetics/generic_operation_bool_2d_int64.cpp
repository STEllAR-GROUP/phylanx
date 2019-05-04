// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_bool.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_bool_nd.hpp>

#include <cmath>
#include <cstdint>
#include <map>
#include <string>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <>
    std::map<std::string,
        generic_operation_bool::matrix_vector_function_ptr<std::int64_t>> const&
    generic_operation_bool::get_2d_map()
    {
        static std::map<std::string, matrix_vector_function_ptr<std::int64_t>>
            map1d = {};
        return map1d;
    }

    template generic_operation_bool::matrix_vector_function_ptr<std::int64_t>
    generic_operation_bool::get_2d_function(std::string const& funcname,
        std::string const& name, std::string const& codename);
}}}
