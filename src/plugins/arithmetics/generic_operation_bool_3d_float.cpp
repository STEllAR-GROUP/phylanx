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
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <>
    std::map<std::string,
        generic_operation_bool::matrix_vector_function_ptr<double>> const&
    generic_operation_bool::get_3d_map()
    {
        static std::map<std::string, matrix_vector_function_ptr<double>> map1d =
        {
            {"isnan",
                [](arg_type<double>&& m) -> arg_type<std::uint8_t> {
                    return arg_type<std::uint8_t>{blaze::map(
                        m.tensor(), [](double val) -> std::uint8_t {
                            return std::isnan(val);
                        })};
                }},
            {"isinf",
                [](arg_type<double>&& m) -> arg_type<std::uint8_t> {
                    return arg_type<std::uint8_t>{blaze::map(
                        m.tensor(), [](double val) -> std::uint8_t {
                            return std::isinf(val);
                        })};
                }},
            {"isfinite",
                [](arg_type<double>&& m) -> arg_type<std::uint8_t> {
                    return arg_type<std::uint8_t>{blaze::map(
                        m.tensor(), [](double val) -> std::uint8_t {
                            return std::isfinite(val);
                        })};
                }},
            {"isneginf",
                [](arg_type<double>&& m) -> arg_type<std::uint8_t> {
                    return arg_type<std::uint8_t>{blaze::map(
                        m.tensor(), [](double val) -> std::uint8_t {
                            return std::isinf(val) && std::signbit(val);
                        })};
                }},
            {"isposinf",
                [](arg_type<double>&& m) -> arg_type<std::uint8_t> {
                    return arg_type<std::uint8_t>{blaze::map(
                        m.tensor(), [](double val) -> std::uint8_t {
                            return std::isinf(val) && !std::signbit(val);
                        })};
                }},
        };
        return map1d;
    }

    template generic_operation_bool::matrix_vector_function_ptr<double>
    generic_operation_bool::get_3d_function(std::string const& funcname,
        std::string const& name, std::string const& codename);
}}}

