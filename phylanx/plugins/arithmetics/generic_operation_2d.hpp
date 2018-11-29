// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_2D_HPP
#define PHYLANX_GENERIC_OPERATION_2D_HPP

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
    generic_operation::get_2d_map()
    {
        static std::map<std::string, matrix_vector_function_ptr<T>> map2d = {
            {"amin",
                [](arg_type<T>&& m) -> arg_type<T> {
                    return arg_type<T>((blaze::min)(m.matrix()));
                }},
            {"amax",
                [](arg_type<T>&& m) -> arg_type<T> {
                    return arg_type<T>((blaze::max)(m.matrix()));
                }},
            {"absolute",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::abs(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::abs(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"floor",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::floor(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::floor(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"ceil",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::ceil(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::ceil(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"trunc",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::trunc(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::trunc(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"rint",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::round(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::round(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"conj",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::conj(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::conj(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"real",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::real(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::real(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"imag",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::imag(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::imag(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"sqrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::sqrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::sqrt(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"invsqrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::invsqrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::invsqrt(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"cbrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::cbrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::cbrt(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"invcbrt",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::invcbrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::invcbrt(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"exp",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::exp(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::exp(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"exp2",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::exp2(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::ceil(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"exp10",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::exp10(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::exp10(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"log",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::log(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::log(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"log2",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::log2(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::log2(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"log10",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::log10(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::log10(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"sin",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::sin(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::sin(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"cos",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::cos(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::cos(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"tan",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::tan(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::tan(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arcsin",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::asin(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::asin(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arccos",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::acos(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::acos(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arctan",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::atan(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::atan(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arcsinh",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::asinh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::asinh(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arccosh",
                [](arg_type<T>&& m) -> arg_type<T> {
#if defined(PHYLANX_DEBUG)
                    for (size_t i = 0UL; i < m.matrix().rows(); ++i)
                    {
                        for (size_t j = 0UL; j < m.matrix().columns(); ++j)
                        {
                            if (m.matrix()(i, j) < 1)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "arccosh", "matrix arccosh: domain error");
                            }
                        }
                    }
#endif
                    if (m.is_ref())
                    {
                        m = blaze::acosh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::acosh(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"arctanh",
                [](arg_type<T>&& m) -> arg_type<T> {
#if defined(PHYLANX_DEBUG)
                    for (size_t i = 0UL; i < m.matrix().rows(); ++i)
                    {
                        for (size_t j = 0UL; j < m.matrix().columns(); ++j)
                        {
                            if (m.matrix()(i, j) <= -1 || m.matrix()(i, j) >= 1)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "arctanh", "matrix arctanh: domain error");
                            }
                        }
                    }
#endif
                    if (m.is_ref())
                    {
                        m = blaze::atanh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::atanh(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"erf",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::erf(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::erf(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"erfc",
                [](arg_type<T>&& m) -> arg_type<T> {
                    if (m.is_ref())
                    {
                        m = blaze::erfc(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::erfc(m.matrix());
                    }
                    return arg_type<T>(std::move(m));
                }},
            {"normalize",
                [](arg_type<T>&& m) -> arg_type<T> {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize() is not a supported matrix operation");
                }},
            {"trace", [](arg_type<T>&& m) -> arg_type<T> {
                 return arg_type<T>(blaze::trace(std::move(m.matrix())));
             }}};
        return map2d;
    }

    template <typename T>
    generic_operation::matrix_vector_function_ptr<T> 
    generic_operation::get_2d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_2d_map<T>();
        auto it = func_map.find(name);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::get_2d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }
}}}

#endif
