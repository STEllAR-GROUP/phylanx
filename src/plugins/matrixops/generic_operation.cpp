// Copyright (c) 2018 Tianyi Zhang
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/generic_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
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
///////////////////////////////////////////////////////////////////////////////
#define PHYLANX_GEN_MATCH_DATA(name)                                           \
    match_pattern_type{name, std::vector<std::string>{name "(_1)"},            \
        &create_generic_operation, &create_primitive<generic_operation>,       \
        "arg\n"                                                                \
        "Args:\n"                                                              \
        "\n"                                                                   \
        "    arg (float) : a floating point number\n"                          \
        "\n"                                                                   \
        "Returns:\n"                                                           \
        "\n"                                                                   \
        "This function implements function `" name "` from Python's "          \
        "math library."                                                        \
    }                                                                          \
    /**/

    std::vector<match_pattern_type> const generic_operation::match_data = {
        PHYLANX_GEN_MATCH_DATA("amin"),
        PHYLANX_GEN_MATCH_DATA("amax"),
        PHYLANX_GEN_MATCH_DATA("absolute"),
        PHYLANX_GEN_MATCH_DATA("floor"),
        PHYLANX_GEN_MATCH_DATA("ceil"),
        PHYLANX_GEN_MATCH_DATA("trunc"),
        PHYLANX_GEN_MATCH_DATA("rint"),
        PHYLANX_GEN_MATCH_DATA("conj"),
        PHYLANX_GEN_MATCH_DATA("real"),
        PHYLANX_GEN_MATCH_DATA("imag"),
        PHYLANX_GEN_MATCH_DATA("sqrt"),
        PHYLANX_GEN_MATCH_DATA("invsqrt"),
        PHYLANX_GEN_MATCH_DATA("cbrt"),
        PHYLANX_GEN_MATCH_DATA("invcbrt"),
        PHYLANX_GEN_MATCH_DATA("exp"),
        PHYLANX_GEN_MATCH_DATA("exp2"),
        PHYLANX_GEN_MATCH_DATA("exp10"),
        PHYLANX_GEN_MATCH_DATA("log"),
        PHYLANX_GEN_MATCH_DATA("log2"),
        PHYLANX_GEN_MATCH_DATA("log10"),
        PHYLANX_GEN_MATCH_DATA("sin"),
        PHYLANX_GEN_MATCH_DATA("cos"),
        PHYLANX_GEN_MATCH_DATA("tan"),
        PHYLANX_GEN_MATCH_DATA("sinh"),
        PHYLANX_GEN_MATCH_DATA("cosh"),
        PHYLANX_GEN_MATCH_DATA("tanh"),
        PHYLANX_GEN_MATCH_DATA("arcsin"),
        PHYLANX_GEN_MATCH_DATA("arccos"),
        PHYLANX_GEN_MATCH_DATA("arctan"),
        PHYLANX_GEN_MATCH_DATA("arcsinh"),
        PHYLANX_GEN_MATCH_DATA("arccosh"),
        PHYLANX_GEN_MATCH_DATA("arctanh"),
        PHYLANX_GEN_MATCH_DATA("erf"),
        PHYLANX_GEN_MATCH_DATA("erfc"),
        PHYLANX_GEN_MATCH_DATA("normalize"),
        PHYLANX_GEN_MATCH_DATA("trace")
    };

#undef PHYLANX_GEN_MATCH_DATA

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::scalar_function_ptr generic_operation::get_0d_map(
        std::string const& name) const
    {
        static std::map<std::string, scalar_function_ptr> map0d = {
            {"amin", [](double m) -> double { return m; }},
            {"amax", [](double m) -> double { return m; }},
            {"absolute", [](double m) -> double { return blaze::abs(m); }},
            {"floor", [](double m) -> double { return blaze::floor(m); }},
            {"ceil", [](double m) -> double { return blaze::ceil(m); }},
            {"trunc", [](double m) -> double { return blaze::trunc(m); }},
            {"rint", [](double m) -> double { return blaze::round(m); }},
            {"conj", [](double m) -> double { return blaze::conj(m); }},
            {"real", [](double m) -> double { return blaze::real(m); }},
            {"imag", [](double m) -> double { return blaze::imag(m); }},
            {"sqrt", [](double m) -> double { return blaze::sqrt(m); }},
            {"invsqrt", [](double m) -> double { return blaze::invsqrt(m); }},
            {"cbrt", [](double m) -> double { return blaze::cbrt(m); }},
            {"invcbrt", [](double m) -> double { return blaze::invcbrt(m); }},
            {"exp", [](double m) -> double { return blaze::exp(m); }},
            {"exp2", [](double m) -> double { return blaze::exp2(m); }},
            {"exp10", [](double m) -> double { return blaze::pow(10, m); }},
            {"log", [](double m) -> double { return blaze::log(m); }},
            {"log2", [](double m) -> double { return blaze::log2(m); }},
            {"log10", [](double m) -> double { return blaze::log10(m); }},
            {"sin", [](double m) -> double { return blaze::sin(m); }},
            {"cos", [](double m) -> double { return blaze::cos(m); }},
            {"tan", [](double m) -> double { return blaze::tan(m); }},
            {"sinh", [](double m) -> double { return blaze::sinh(m); }},
            {"cosh", [](double m) -> double { return blaze::cosh(m); }},
            {"tanh", [](double m) -> double { return blaze::tanh(m); }},
            {"arcsin", [](double m) -> double { return blaze::asin(m); }},
            {"arccos", [](double m) -> double { return blaze::acos(m); }},
            {"arctan", [](double m) -> double { return blaze::atan(m); }},
            {"arcsinh", [](double m) -> double { return blaze::asinh(m); }},
            {"arccosh",
                [](double m) -> double {
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
                [](double m) -> double {
#if defined(PHYLANX_DEBUG)
                    if (m <= -1 || m >= 1)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter, "arctanh",
                            "scalar arctanh: domain error");
                    }
#endif
                    return blaze::atanh(m);
                }},
            {"erf", [](double m) -> double { return blaze::erf(m); }},
            {"erfc", [](double m) -> double { return blaze::erfc(m); }},
            {"normalize",
                [](double m) -> double {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize does not support scalars");
                }},
            {"trace", [](double m) -> double { return m; }}};
        return map0d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::matrix_vector_function_ptr generic_operation::get_1d_map(
        std::string const& name) const
    {
        static std::map<std::string, matrix_vector_function_ptr> map1d = {
            {"amin",
                [](arg_type&& m) -> arg_type {
                    return arg_type(
                        dynamic_vector_type(1, (blaze::min)(m.vector())));
                }},
            {"amax",
                [](arg_type&& m) -> arg_type {
                    return arg_type(
                        dynamic_vector_type(1, (blaze::max)(m.vector())));
                }},
            {"absolute",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::abs(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::abs(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"floor",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::floor(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::floor(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"ceil",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::ceil(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::ceil(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"trunc",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::trunc(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::trunc(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"rint",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::round(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::round(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"conj",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::conj(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::conj(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"real",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::real(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::real(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"imag",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::imag(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::imag(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"sqrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::sqrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::sqrt(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"invsqrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::invsqrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::invsqrt(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"cbrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::cbrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::cbrt(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"invcbrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::invcbrt(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::invcbrt(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"exp",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::exp(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::exp(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"exp2",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::exp2(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::exp2(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"exp10",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::exp10(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::exp10(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"log",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::log(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::log(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"log2",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::log2(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::log2(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"log10",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::log10(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::log10(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"sin",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::sin(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::sin(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"cos",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::cos(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::cos(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"tan",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::tan(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::tan(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            { "sinh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::sinh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::sinh(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            { "cosh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::cosh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::cosh(m.vector());
                    }
                    return arg_type(std::move(m));
                    }},
            { "tanh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::tanh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::tanh(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"arcsin",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::asin(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::asin(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"arccos",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::acos(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::acos(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"arctan",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::atan(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::atan(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"arcsinh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::asinh(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::asinh(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"arccosh",
                [](arg_type&& m) -> arg_type {
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
                    return arg_type(std::move(m));
                }},
            {"arctanh",
                [](arg_type&& m) -> arg_type {
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
                    return arg_type(std::move(m));
                }},
            {"erf",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::erf(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::erf(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"erfc",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::erfc(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::erfc(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"normalize",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::normalize(m.vector());
                    }
                    else
                    {
                        m.vector() = blaze::normalize(m.vector());
                    }
                    return arg_type(std::move(m));
                }},
            {"trace",
                [](arg_type&& m) -> arg_type {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "trace",
                        "vector operation is not supported for trace()");
                }}

        };
        return map1d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    generic_operation::matrix_vector_function_ptr generic_operation::get_2d_map(
        std::string const& name) const
    {
        static std::map<std::string, matrix_vector_function_ptr> map2d = {
            {"amin",
                [](arg_type&& m) -> arg_type {
                    return arg_type(
                        dynamic_matrix_type(1, 1, (blaze::min)(m.matrix())));
                }},
            {"amax",
                [](arg_type&& m) -> arg_type {
                    return arg_type(
                        dynamic_matrix_type(1, 1, (blaze::max)(m.matrix())));
                }},
            {"absolute",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::abs(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::abs(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"floor",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::floor(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::floor(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"ceil",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::ceil(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::ceil(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"trunc",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::trunc(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::trunc(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"rint",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::round(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::round(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"conj",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::conj(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::conj(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"real",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::real(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::real(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"imag",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::imag(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::imag(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"sqrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::sqrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::sqrt(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"invsqrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::invsqrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::invsqrt(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"cbrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::cbrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::cbrt(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"invcbrt",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::invcbrt(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::invcbrt(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"exp",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::exp(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::exp(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"exp2",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::exp2(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::ceil(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"exp10",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::exp10(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::exp10(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"log",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::log(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::log(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"log2",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::log2(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::log2(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"log10",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::log10(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::log10(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"sin",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::sin(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::sin(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"cos",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::cos(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::cos(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"tan",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::tan(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::tan(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"sinh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::sinh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::sinh(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"cosh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::cosh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::cosh(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"tanh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::tanh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::tanh(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"arcsin",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::asin(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::asin(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"arccos",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::acos(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::acos(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"arctan",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::atan(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::atan(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"arcsinh",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::asinh(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::asinh(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"arccosh",
                [](arg_type&& m) -> arg_type {
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
                    return arg_type(std::move(m));
                }},
            {"arctanh",
                [](arg_type&& m) -> arg_type {
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
                    return arg_type(std::move(m));
                }},
            {"erf",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::erf(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::erf(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"erfc",
                [](arg_type&& m) -> arg_type {
                    if (m.is_ref())
                    {
                        m = blaze::erfc(m.matrix());
                    }
                    else
                    {
                        m.matrix() = blaze::erfc(m.matrix());
                    }
                    return arg_type(std::move(m));
                }},
            {"normalize",
                [](arg_type&& m) -> arg_type {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter, "normalize",
                        "normalize() is not a supported matrix operation");
                }},
            {"trace", [](arg_type&& m) -> arg_type {
                 return arg_type(dynamic_matrix_type(
                     1, 1, blaze::trace(std::move(m.matrix()))));
             }}};
        return map2d[name];
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {
        std::string extract_function_name(std::string const& name)
        {
            compiler::primitive_name_parts name_parts;
            if (!compiler::parse_primitive_name(name, name_parts))
            {
                return name;
            }

            return name_parts.primitive;
        }
    }

    generic_operation::generic_operation(
        primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
        std::string func_name = detail::extract_function_name(name);

        func0d_ = get_0d_map(func_name);
        func1d_ = get_1d_map(func_name);
        func2d_ = get_2d_map(func_name);

        HPX_ASSERT(
            func0d_ != nullptr && func1d_ != nullptr && func2d_ != nullptr);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type generic_operation::generic0d(arg_type && op) const
    {
        return primitive_argument_type{func0d_(op.scalar())};
    }

    primitive_argument_type generic_operation::generic1d(arg_type && op) const
    {
        return primitive_argument_type{
            ir::node_data<double>{func1d_(std::move(op))}};
    }

    primitive_argument_type generic_operation::generic2d(arg_type && op) const
    {
        return primitive_argument_type{
            ir::node_data<double>{func2d_(std::move(op))}};
    }

    hpx::future<primitive_argument_type> generic_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                generate_error_message(
                    "the generic_operation primitive requires"
                    "exactly one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "generic_operation::eval",
                generate_error_message(
                    "the generic_operation primitive requires "
                    "that the arguments given by the operands "
                    "array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](arg_type&& op)
                -> primitive_argument_type
                {
                    std::size_t dims = op.num_dimensions();
                    switch (dims)
                    {
                    case 0:
                        return this_->generic0d(std::move(op));

                    case 1:
                        return this_->generic1d(std::move(op));

                    case 2:
                        return this_->generic2d(std::move(op));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "generic_operation::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                }),
            numeric_operand(operands[0], args,
                name_, codename_, std::move(ctx)));
    }
}}}
