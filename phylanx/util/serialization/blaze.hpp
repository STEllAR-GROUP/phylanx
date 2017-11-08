//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP)
#define PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP

#include <phylanx/config.hpp>
#include <hpx/include/serialization.hpp>
#include <blaze/Math.h>

#include <cstddef>

namespace hpx { namespace serialization
{
    template <typename T>
    void load(input_archive& archive,
        blaze::DynamicVector<T, blaze::columnVector>& target,
        unsigned)
    {
        // De-serialize Header
        std::size_t count_ = 0UL;
        archive >> count_;
        target = blaze::DynamicVector<T, blaze::columnVector>(count_);

        // DeserializeVector
        T value{};
        std::size_t j = 0UL;
        // NOTE: I've assumed archive >> value always returns something
        while (j != count_) {
            archive >> value;
            target[j] = value;
            ++j;
        }
    }

    template <typename T>
    void load(input_archive& archive,
        blaze::DynamicMatrix<T>& target,
        unsigned)
    {
        // DeserializeHeader
        std::size_t rows_ = 0UL;
        std::size_t columns_ = 0UL;

        archive >> rows_ >> columns_;
        target = blaze::DynamicMatrix<T>(rows_, columns_);

        // DeserializeMatrix
        T value{};
        for (std::size_t i = 0UL; i < rows_; ++i)
        {
            std::size_t j = 0UL;
            // NOTE: I've assumed archive >> value always returns something
            while (j != columns_) {
                archive >> value;
                target(i, j) = value;
                ++j;
            }
        }
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicVector<T, blaze::columnVector> const& target,
        unsigned)
    {
        // SerializeVector
        archive << target.size();

        // SerializeVector
        for (std::size_t j = 0UL; j < target.size(); ++j)
        {
            archive << target[j];
        }
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicMatrix<T> const& target,
        unsigned)
    {
        // Serialize header
        archive << target.rows() << target.columns();

        // SerializeMatrix
        for (std::size_t i = 0UL; i < target.rows(); ++i)
        {
            for (std::size_t j = 0UL; j < target.columns(); ++j)
            {
                archive << target(i, j);
            }
        }
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>),
        (blaze::DynamicVector<T, blaze::columnVector>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>),
        (blaze::DynamicMatrix<T>));
}}

#endif
