// Copyright (c) 2018 Tianyi Zhang
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_3D_HPP
#define PHYLANX_GENERIC_OPERATION_3D_HPP

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
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
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::map<std::string,
        generic_operation::matrix_vector_function_ptr<T>> const&
    generic_operation::get_3d_map()
    {
        static std::map<std::string, matrix_vector_function_ptr<T>> map3d = {
            {"amin",
                [](arg_type<T>&& t) -> arg_type<T> {
                    return arg_type<T>((blaze::min)(t.tensor()));
                }},
            {"amax",
                [](arg_type<T>&& t) -> arg_type<T> {
                    return arg_type<T>((blaze::max)(t.tensor()));
                }},
            {"absolute",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::abs(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::abs(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"floor",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::floor(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::floor(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"ceil",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::ceil(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::ceil(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"trunc",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::trunc(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::trunc(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"rint",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::round(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::round(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"conj",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::conj(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::conj(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"real",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::real(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::real(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"imag",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::imag(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::imag(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"sqrt",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::sqrt(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::sqrt(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"invsqrt",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::invsqrt(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::invsqrt(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"cbrt",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::cbrt(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::cbrt(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"invcbrt",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::invcbrt(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::invcbrt(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"exp",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::exp(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::exp(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"exp2",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::exp2(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::ceil(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"exp10",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::exp10(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::exp10(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"log",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::log(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::log(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"log2",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::log2(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::log2(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"log10",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::log10(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::log10(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"sin",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::sin(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::sin(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"cos",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::cos(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::cos(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"tan",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::tan(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::tan(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"arcsin",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::asin(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::asin(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"arccos",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::acos(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::acos(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"arctan",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::atan(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::atan(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"arcsinh",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::asinh(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::asinh(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"arccosh",
                [](arg_type<T>&& t) -> arg_type<T> {
#if defined(PHYLANX_DEBUG)
                    for (size_t k = 0UL; k < t.tensor().pages(); ++k)
                    {
                        for (size_t i = 0UL; i < t.tensor().rows(); ++i)
                        {
                            for (size_t j = 0UL; j < t.tensor().columns(); ++j)
                            {
                                if (t.tensor()(k, i, j) < 1)
                                {
                                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                        "arccosh", "tensor arccosh: domain error");
                                }
                            }
                        }
                    }
#endif
                    if (t.is_ref())
                    {
                        t = blaze::acosh(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::acosh(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"arctanh",
                [](arg_type<T>&& t) -> arg_type<T> {
#if defined(PHYLANX_DEBUG)
                    for (size_t k = 0UL; k < t.tensor().pages(); ++k)
                    {
                        for (size_t i = 0UL; i < t.tensor().rows(); ++i)
                        {
                            for (size_t j = 0UL; j < t.tensor().columns(); ++j)
                            {
                                if (t.tensor()(k, i, j) <= -1 ||
                                    t.tensor()(k, i, j) >= 1)
                                {
                                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                        "arctanh", "tensor arctanh: domain error");
                                }
                            }
                        }
                    }
#endif
                    if (t.is_ref())
                    {
                        t = blaze::atanh(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::atanh(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"erf",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::erf(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::erf(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"erfc",
                [](arg_type<T>&& t) -> arg_type<T> {
                    if (t.is_ref())
                    {
                        t = blaze::erfc(t.tensor());
                    }
                    else
                    {
                        t.tensor() = blaze::erfc(t.tensor());
                    }
                    return arg_type<T>(std::move(t));
                }},
            {"normalize",
                [](arg_type<T>&& t) -> arg_type<T> {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize() is not a supported tensor operation");
                }},
            {"trace", [](arg_type<T>&& t) -> arg_type<T> {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "trace() is not a supported tensor operation");
             }}
        };
        return map3d;
    }

    template <typename T>
    generic_operation::matrix_vector_function_ptr<T>
    generic_operation::get_3d_function(std::string const& funcname,
        std::string const& name, std::string const& codename)
    {
        auto func_map = get_3d_map<T>();
        auto it = func_map.find(name);
        if (it == func_map.end())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::get_3d_function",
                util::generate_error_message(
                    "unknown function requested: " + funcname, name, codename));
        }
        return it->second;
    }
}}}

#endif
#endif
