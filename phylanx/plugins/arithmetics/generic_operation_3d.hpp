// Copyright (c) 2018 Tianyi Zhang
// Copyright (c) 2018 Bita Hasheminezhad
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef PHYLANX_GENERIC_OPERATION_3D_HPP
#define PHYLANX_GENERIC_OPERATION_3D_HPP

#include <phylanx/config.hpp>

#include <phylanx/execution_tree/compiler/primitive_name.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/generic_operation.hpp>

#include <hpx/assertion.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

////////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ////////////////////////////////////////////////////////////////////////////
#define GENERIC_OPERATION_3D(func)                                             \
    template <typename T>                                                      \
    ir::node_data<T> func##_3d(ir::node_data<T>&& t)                           \
    {                                                                          \
        if (t.is_ref())                                                        \
        {                                                                      \
            t = blaze::func(t.tensor());                                       \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            t.tensor() = blaze::func(t.tensor());                              \
        }                                                                      \
        return ir::node_data<T>(std::move(t));                                 \
    }                                                                          \
    /**/


    GENERIC_OPERATION_3D(abs);
    GENERIC_OPERATION_3D(floor);
    GENERIC_OPERATION_3D(ceil);
    GENERIC_OPERATION_3D(trunc);
    GENERIC_OPERATION_3D(round);
    GENERIC_OPERATION_3D(conj);
    GENERIC_OPERATION_3D(real);
    GENERIC_OPERATION_3D(imag);
    GENERIC_OPERATION_3D(sqrt);
    GENERIC_OPERATION_3D(invsqrt);
    GENERIC_OPERATION_3D(cbrt);
    GENERIC_OPERATION_3D(invcbrt);
    GENERIC_OPERATION_3D(exp);
    GENERIC_OPERATION_3D(exp2);
    GENERIC_OPERATION_3D(exp10);
    GENERIC_OPERATION_3D(log);
    GENERIC_OPERATION_3D(log2);
    GENERIC_OPERATION_3D(log10);
    GENERIC_OPERATION_3D(sin);
    GENERIC_OPERATION_3D(cos);
    GENERIC_OPERATION_3D(tan);
    GENERIC_OPERATION_3D(sinh);
    GENERIC_OPERATION_3D(cosh);
    GENERIC_OPERATION_3D(tanh);
    GENERIC_OPERATION_3D(asin);
    GENERIC_OPERATION_3D(acos);
    GENERIC_OPERATION_3D(atan);
    GENERIC_OPERATION_3D(asinh);
    GENERIC_OPERATION_3D(erf);
    GENERIC_OPERATION_3D(erfc);
    GENERIC_OPERATION_3D(sign);

#undef GENERIC_OPERATION_3D

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> acosh_3d(ir::node_data<T>&& t)
    {
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
        return ir::node_data<T>(std::move(t));
    }

    template <typename T>
    ir::node_data<T> atanh_3d(ir::node_data<T>&& t)
    {
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
        return ir::node_data<T>(std::move(t));
    }

    template <typename T>
    ir::node_data<T> square_3d(ir::node_data<T>&& t)
    {
        if (t.is_ref())
        {
            t = t.tensor() % t.tensor();
        }
        else
        {
            t.tensor() %= t.tensor();
        }
        return ir::node_data<T>(std::move(t));
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helper to explicitly instantiate one of the functions above
#define GENERIC_OPERATION_3D_INSTANTIATION(func, type)                         \
    template ir::node_data<type> func##_3d<type>(ir::node_data<type> && t);    \
    /**/

    ////////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::map<std::string,
        generic_operation::matrix_vector_function_ptr<T>> const&
    generic_operation::get_3d_map()
    {
        static std::map<std::string, matrix_vector_function_ptr<T>> map3d = {
            {"absolute", &abs_3d<T>},
            {"floor", &floor_3d<T>},
            {"ceil", &ceil_3d<T>},
            {"trunc", &trunc_3d<T>},
            {"rint", round_3d<T>},
            {"conj", conj_3d<T>},
            {"real", real_3d<T>},
            {"imag", imag_3d<T>},
            {"sqrt", sqrt_3d<T>},
            {"invsqrt", &invsqrt_3d<T>},
            {"cbrt", &cbrt_3d<T>},
            {"invcbrt", &invcbrt_3d<T>},
            {"exp", &exp_3d<T>},
            {"exp2", &exp2_3d<T>},
            {"exp10", &exp10_3d<T>},
            {"log", &log_3d<T>},
            {"log2", &log2_3d<T>},
            {"log10", &log10_3d<T>},
            {"sin", &sin_3d<T>},
            {"cos", &cos_3d<T>},
            {"tan", &tan_3d<T>},
            {"sinh", &sinh_3d<T>},
            {"cosh", &cosh_3d<T>},
            {"tanh", &tanh_3d<T>},
            {"arcsin", &asin_3d<T>},
            {"arccos", &acos_3d<T>},
            {"arctan", &atan_3d<T>},
            {"arcsinh", &asinh_3d<T>},
            {"arccosh", &acosh_3d<T>},
            {"arctanh", &atanh_3d<T>},
            {"erf", &erf_3d<T>},
            {"erfc", &erfc_3d<T>},
            {"square", &square_3d<T>},
            {"sign", &sign_3d<T>},
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
        auto it = func_map.find(funcname);
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
