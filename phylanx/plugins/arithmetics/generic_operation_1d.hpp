// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_1D_HPP
#define PHYLANX_GENERIC_OPERATION_1D_HPP

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
    std::map<std::string,
        generic_operation::matrix_vector_function_ptr<T>> const&
    generic_operation::get_1d_map()
    {
        static std::map<std::string, matrix_vector_function_ptr<T>> map1d = {
            {"amin",
                [](arg_type<T>&& m) -> arg_type<T> {
                    return arg_type<T>((blaze::min)(m.vector()));
                }},
            {"amax",
                [](arg_type<T>&& m) -> arg_type<T> {
                    return arg_type<T>((blaze::max)(m.vector()));
                }},
            {"absolute",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::abs(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::abs(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"floor",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::floor(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::floor(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"ceil",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::ceil(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::ceil(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"trunc",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::trunc(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::trunc(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"rint",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::round(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::round(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"conj",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::conj(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::conj(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"real",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::real(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::real(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"imag",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::imag(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::imag(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"sqrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::sqrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::sqrt(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"invsqrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::invsqrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::invsqrt(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"cbrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::cbrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::cbrt(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"invcbrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::invcbrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::invcbrt(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"exp",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::exp(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::exp(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"exp2",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::exp2(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::exp2(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"exp10",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::exp10(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::exp10(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"log",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::log(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::log(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"log2",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::log2(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::log2(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"log10",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::log10(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::log10(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"sin",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::sin(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::sin(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"cos",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::cos(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::cos(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"tan",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::tan(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::tan(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arcsin",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::asin(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::asin(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arccos",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::acos(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::acos(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arctan",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::atan(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::atan(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arcsinh",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::asinh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::asinh(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arccosh",
                [](arg_type<T>&& m) -> arg_type<T> {
#if defined(PHYLANX_DEBUG)
                    for (auto a : m.vector())
                    {
                        if (a < 1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter, "arccosh",
                                "vector arccosh: domain error");
                        }
                    }
#endif
                    if (m.is_ref())
                    {
                        m = blaze::acosh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::acosh(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arctanh",
                [](arg_type<T>&& m) -> arg_type<T> {
#if defined(PHYLANX_DEBUG)
                    for (auto a : m.vector())
                    {
                        if (a >= 1 || a <= -1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter, "arctanh",
                                "vector arctanh: domain error");
                        }
                    }
#endif
                    if (m.is_ref())
                    {
                        m = blaze::atanh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::atanh(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"erf",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::erf(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::erf(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"erfc",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::erfc(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::erfc(m.vector());
                    }
                    return arg_type<T>(std::move(m));
                }},
//             {"normalize",
//                 [](arg_type<T>&& m) -> arg_type<T> {
//                     if (m.is_ref())
//                     {
//                         m = blaze::normalize(m.vector());
//                     }
//                     else
//                     {
//                         m.vector() = blaze::normalize(m.vector());
//                     }
//                     return arg_type<T>(std::move(m));
//                 }},
            {"trace",
                [](arg_type<T>&& m) -> arg_type<T> {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "trace",
                        "vector operation is not supported for trace()");
                }}
        };
        return map1d;
    }

    template <typename T>
    generic_operation::matrix_vector_function_ptr<T>
    generic_operation::get_1d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_1d_map<T>();
        auto it = func_map.find(name);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::get_1d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }
}}}

#endif
