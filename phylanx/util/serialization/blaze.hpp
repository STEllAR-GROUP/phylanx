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
    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void load(input_archive& archive, blaze::DynamicVector<T>& target, unsigned)
    {
        // De-serialize Header
        std::size_t count = 0UL;
        archive >> count;

        target.resize(count, false);
        archive >>
            hpx::serialization::make_array(target.data(), target.spacing());
    }

    template <typename T>
    void load(input_archive& archive, blaze::DynamicMatrix<T>& target, unsigned)
    {
        // DeserializeHeader
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        archive >> rows >> columns;

        target.resize(rows, columns, false);
        archive >> hpx::serialization::make_array(
                       target.data(), target.spacing() * rows);
    }

    template <typename T, bool AF, bool PF>
    void load(input_archive& archive, blaze::CustomVector<T, AF, PF>& target,
        unsigned)
    {
    }

    template <typename T, bool AF, bool PF>
    void load(input_archive& archive, blaze::CustomMatrix<T, AF, PF>& target,
        unsigned)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void save(output_archive& archive, blaze::DynamicVector<T> const& target,
        unsigned)
    {
        // SerializeVector
        archive << target.size();
        archive << hpx::serialization::make_array(
            target.data(), target.spacing());
    }

    template <typename T>
    void save(output_archive& archive, blaze::DynamicMatrix<T> const& target,
        unsigned)
    {
        // Serialize header
        archive << target.rows() << target.columns();
        archive << hpx::serialization::make_array(
            target.data(), target.spacing() * target.rows());
    }

    template <typename T, bool AF, bool PF>
    void save(output_archive& archive,
        blaze::CustomVector<T, AF, PF> const& target, unsigned)
    {
    }

    template <typename T, bool AF, bool PF>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF> const& target, unsigned)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (blaze::DynamicVector<T>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (blaze::DynamicMatrix<T>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF>),
        (blaze::CustomVector<T, AF, PF>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF>),
        (blaze::CustomMatrix<T, AF, PF>));
}}

#endif
