// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_0D_HPP
#define PHYLANX_GENERIC_OPERATION_0D_HPP

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/generic_operation.hpp>

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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::map<std::string, generic_operation::scalar_function_ptr<T>> const&
        generic_operation::get_0d_map()
    {
        static std::map<std::string, scalar_function_ptr<T>> map0d = {
            {"amin", [](T m) -> T { return m; }},
            {"amax", [](T m) -> T { return m; }},
            {"absolute", [](T m) -> T { return blaze::abs(m); }},
            {"floor", [](T m) -> T { return blaze::floor(m); }},
            {"ceil", [](T m) -> T { return blaze::ceil(m); }},
            {"trunc", [](T m) -> T { return blaze::trunc(m); }},
            {"rint", [](T m) -> T { return blaze::round(m); }},
            {"conj", [](T m) -> T { return blaze::conj(m); }},
            {"real", [](T m) -> T { return blaze::real(m); }},
            {"imag", [](T m) -> T { return blaze::imag(m); }},
            {"sqrt", [](T m) -> T { return blaze::sqrt(m); }},
            {"invsqrt", [](T m) -> T { return blaze::invsqrt(m); }},
            {"cbrt", [](T m) -> T { return blaze::cbrt(m); }},
            {"invcbrt", [](T m) -> T { return blaze::invcbrt(m); }},
            {"exp", [](T m) -> T { return blaze::exp(m); }},
            {"exp2", [](T m) -> T { return blaze::exp2(m); }},
            {"exp10", [](T m) -> T { return blaze::pow(10, m); }},
            {"log", [](T m) -> T { return blaze::log(m); }},
            {"log2", [](T m) -> T { return blaze::log2(m); }},
            {"log10", [](T m) -> T { return blaze::log10(m); }},
            {"sin", [](T m) -> T { return blaze::sin(m); }},
            {"cos", [](T m) -> T { return blaze::cos(m); }},
            {"tan", [](T m) -> T { return blaze::tan(m); }},
            {"arcsin", [](T m) -> T { return blaze::asin(m); }},
            {"arccos", [](T m) -> T { return blaze::acos(m); }},
            {"arctan", [](T m) -> T { return blaze::atan(m); }},
            {"arcsinh", [](T m) -> T { return blaze::asinh(m); }},
            {"arccosh",
                [](T m) -> T {
#if defined(PHYLANX_DEBUG)
                    if (m < 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "arccosh",
                            "scalar arccosh: domain error");
                    }
#endif
                    return blaze::acosh(m);
                }},
            {"arctanh",
                [](T m) -> T {
#if defined(PHYLANX_DEBUG)
                    if (m <= -1 || m >= 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "arctanh",
                            "scalar arctanh: domain error");
                    }
#endif
                    return blaze::atanh(m);
                }},
            {"erf", [](T m) -> T { return blaze::erf(m); }},
            {"erfc", [](T m) -> T { return blaze::erfc(m); }},
            {"normalize",
                [](T m) -> T {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize does not support scalars");
                }},
            {"trace", [](T m) -> T { return m; }}
        };
        return map0d;
    }

    template <typename T>
    generic_operation::scalar_function_ptr<T>
    generic_operation::get_0d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_0d_map<T>();
        auto it = func_map.find(name);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::get_0d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }
}}}

#endif
