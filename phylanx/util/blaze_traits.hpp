// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_BLAZE_TRAITS_NOV_01_2018_0931AM)
#define PHYLANX_UTIL_BLAZE_TRAITS_NOV_01_2018_0931AM

#include <phylanx/config.hpp>

#include <cstdint>
#include <type_traits>

#include <blaze/Math.h>

namespace phylanx { namespace traits
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct is_scalar : std::false_type
    {
    };

    template <typename T>
    struct is_vector : std::false_type
    {
    };

    template <typename T>
    struct is_matrix : std::false_type
    {
    };

    template <typename T>
    using is_scalar_t = typename is_scalar<T>::type;

    template <typename T>
    using is_vector_t = typename is_vector<T>::type;

    template <typename T>
    using is_matrix_t = typename is_matrix<T>::type;

    ///////////////////////////////////////////////////////////////////////////
    template <>
    struct is_scalar<std::uint8_t> : std::true_type
    {
    };

    template <>
    struct is_scalar<std::int64_t> : std::true_type
    {
    };

    template <>
    struct is_scalar<double> : std::true_type
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool TF>
    struct is_vector<blaze::DynamicVector<T, TF>> : std::true_type
    {
    };

    template <typename T, bool AF, bool PF, bool TF, typename RT>
    struct is_vector<blaze::CustomVector<T, AF, PF, TF, RT>> : std::true_type
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool SO>
    struct is_matrix<blaze::DynamicMatrix<T, SO>> : std::true_type
    {
    };

    template <typename T, bool AF, bool PF, bool SO, typename RT>
    struct is_matrix<blaze::CustomMatrix<T, AF, PF, SO, RT>> : std::true_type
    {
    };
}}

#endif
