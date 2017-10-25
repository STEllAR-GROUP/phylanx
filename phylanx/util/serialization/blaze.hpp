//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP)
#define PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP

#include <phylanx/config.hpp>
#include <hpx/include/serialization.hpp>
#include <blaze/Math.h>


namespace hpx { namespace serialization
{
    template <typename T>
    void load(input_archive& archive,
        blaze::DynamicVector<T, blaze::rowVector>& mat,
        unsigned)
    {
        // DeserializeHeader
        size_t count_ = 0UL;

        archive >> count_;
        // DeserializeVector
        T value{};
        size_t j = 0UL;
        // NOTE: I've assumed archive >> value always returns something
        while (j != count_) {
            archive >> value;
            mat[j] = value;
            ++j;
        }
    }

    template <typename T>
    void load(input_archive& archive,
        blaze::DynamicMatrix<T>& mat,
        unsigned)
    {
        // DeserializeHeader
        size_t rows_ = 0UL;
        size_t columns_ = 0UL;

        archive >> rows_ >> columns_;
        mat = blaze::DynamicMatrix<T>(rows_, columns_);
        // DeserializeMatrix
        T value{};
        for (size_t i = 0UL; i < rows_; ++i)
        {
            size_t j = 0UL;
            // NOTE: I've assumed archive >> value always returns something
            while (j != columns_) {
                archive >> value;
                mat(i, j) = value;
                ++j;
            }
        }
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicVector<T, blaze::rowVector> const& v,
        unsigned)
    {
        // SerializeVector
        archive << v.size();

        // SerializeVector
        for (size_t j = 0UL; j < v.size(); ++j) {
            archive << v(j);
        }
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicMatrix<T> const& mat,
        unsigned)
    {
        // Serializeheader
        archive << mat.rows() << mat.columns();

        // SerializeMatrix
        for (size_t i = 0UL; i < mat.rows(); ++i) {
            for (size_t j = 0UL; j < mat.columns(); ++j) {
                archive << mat(i, j);
            }
        }
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>),
        (blaze::DynamicVector<T, blaze::rowVector>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>),
        (blaze::DynamicMatrix<T>));
}}

#endif
