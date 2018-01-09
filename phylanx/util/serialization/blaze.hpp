//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP)
#define PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP

#include <phylanx/config.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <blaze/Math.h>

#include <cstddef>

namespace hpx { namespace serialization
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool TF>
    void load(
        input_archive& archive, blaze::DynamicVector<T, TF>& target, unsigned)
    {
        // De-serialize vector
        std::size_t count = 0UL;
        std::size_t spacing = 0UL;
        archive >> count >> spacing;

        target.resize(count, false);
        archive >>
            hpx::serialization::make_array(target.data(), spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void load(input_archive& archive, blaze::DynamicMatrix<T, true>& target,
        unsigned)
    {
        // De-serialize matrix
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        std::size_t spacing = 0UL;
        archive >> rows >> columns >> spacing;

        target.resize(rows, columns, false);
        archive >>
            hpx::serialization::make_array(target.data(), spacing * columns);
    }

    template <typename T>
    void load(input_archive& archive, blaze::DynamicMatrix<T, false>& target,
        unsigned)
    {
        // De-serialize matrix
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        std::size_t spacing = 0UL;
        archive >> rows >> columns >> spacing;

        target.resize(rows, columns, false);
        archive >>
            hpx::serialization::make_array(target.data(), rows * spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool AF, bool PF, bool TF>
    void load(input_archive& archive,
        blaze::CustomVector<T, AF, PF, TF>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    template <typename T, bool AF, bool PF, bool SO>
    void load(input_archive& archive,
        blaze::CustomMatrix<T, AF, PF, SO>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool TF>
    void save(output_archive& archive,
        blaze::DynamicVector<T, TF> const& target, unsigned)
    {
        // Serialize vector
        std::size_t count = target.size();
        std::size_t spacing = target.spacing();
        archive << count << spacing;
        archive << hpx::serialization::make_array(target.data(), spacing);
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicMatrix<T, true> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;
        archive << hpx::serialization::make_array(
            target.data(), spacing * columns);
    }

    template <typename T>
    void save(output_archive& archive,
        blaze::DynamicMatrix<T, false> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;
        archive << hpx::serialization::make_array(
            target.data(), rows * spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool AF, bool PF, bool TF>
    void save(output_archive& archive,
        blaze::CustomVector<T, AF, PF, TF> const& target, unsigned)
    {
        // Serialize vector
        std::size_t count = target.size();
        std::size_t spacing = target.spacing();
        archive << count << spacing;
        archive << hpx::serialization::make_array(target.data(), spacing);
    }

    template <typename T, bool AF, bool PF>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF, true> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;
        archive << hpx::serialization::make_array(
            target.data(), spacing * columns);
    }

    template <typename T, bool AF, bool PF>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF, false> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;
        archive << hpx::serialization::make_array(
            target.data(), rows * spacing);
    }

    ///////////////////////////////////////////////////////////////////////////
    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool TF>), (blaze::DynamicVector<T, TF>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool SO>), (blaze::DynamicMatrix<T, SO>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF, bool TF>),
        (blaze::CustomVector<T, AF, PF, TF>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF, bool SO>),
        (blaze::CustomMatrix<T, AF, PF, SO>));
}}

#endif
