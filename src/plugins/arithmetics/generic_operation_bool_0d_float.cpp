// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_bool.hpp>
#include <phylanx/plugins/arithmetics/generic_operation_bool_nd.hpp>

#include <cmath>
#include <map>
#include <string>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <>
    std::map<std::string,
        generic_operation_bool::scalar_function_ptr<double>> const&
    generic_operation_bool::get_0d_map()
    {
        static std::map<std::string, scalar_function_ptr<double>> map0d = {
            {"isnan", [](double val) -> bool { return std::isnan(val); }},
            {"isinf", [](double val) -> bool { return std::isinf(val); }},
            {"isfinite", [](double val) -> bool { return std::isfinite(val); }},
            {"isneginf",
                [](double val) -> bool {
                    return std::isinf(val) && std::signbit(val);
                }},
            {"isposinf", [](double val) -> bool {
                 return std::isinf(val) && !std::signbit(val);
             }}};
        return map0d;
    }

    template generic_operation_bool::scalar_function_ptr<double>
    generic_operation_bool::get_0d_function(std::string const& funcname,
        std::string const& name, std::string const& codename);
}}}
