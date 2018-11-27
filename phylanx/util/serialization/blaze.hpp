//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:noinclude:HPX_ASSERT

#if !defined(PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP)
#define PHYLANX_UTIL_BLAZE_SERIALIZATION_HPP

#include <phylanx/config.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    void load(input_archive& archive, blaze::DynamicTensor<T>& target,
        unsigned)
    {
        // De-serialize matrix
        std::size_t rows = 0UL;
        std::size_t columns = 0UL;
        std::size_t pages = 0UL;
        std::size_t spacing = 0UL;
        archive >> rows >> columns >> pages >> spacing;

        target.resize(rows, columns, pages, false);
        archive >> hpx::serialization::make_array(
                       target.data(), rows * spacing * pages);
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool AF, bool PF, bool TF, typename RT>
    void load(input_archive& archive,
        blaze::CustomVector<T, AF, PF, TF, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

    template <typename T, bool AF, bool PF, bool SO, typename RT>
    void load(input_archive& archive,
        blaze::CustomMatrix<T, AF, PF, SO, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T, bool AF, bool PF, typename RT>
    void load(input_archive& archive,
        blaze::CustomTensor<T, AF, PF, RT>& target, unsigned)
    {
        HPX_ASSERT(false);      // shouldn't ever be called
    }
#endif

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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    void save(output_archive& archive, blaze::DynamicTensor<T> const& target,
        unsigned)
    {
        // Serialize tensor
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t pages = target.pages();
        std::size_t spacing = target.spacing();
        archive << rows << columns << pages << spacing;

        archive << hpx::serialization::make_array(
            target.data(), rows * spacing * pages);
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, bool AF, bool PF, bool TF, typename RT>
    void save(output_archive& archive,
        blaze::CustomVector<T, AF, PF, TF, RT> const& target, unsigned)
    {
        // Serialize vector
        std::size_t count = target.size();
        std::size_t spacing = target.spacing();
        archive << count << spacing;

        archive << hpx::serialization::make_array(target.data(), spacing);
    }

    template <typename T, bool AF, bool PF, typename RT>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF, true, RT> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), spacing * columns);
    }

    template <typename T, bool AF, bool PF, typename RT>
    void save(output_archive& archive,
        blaze::CustomMatrix<T, AF, PF, false, RT> const& target, unsigned)
    {
        // Serialize matrix
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t spacing = target.spacing();
        archive << rows << columns << spacing;

        archive << hpx::serialization::make_array(
            target.data(), rows * spacing);
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T, bool AF, bool PF, typename RT>
    void save(output_archive& archive,
        blaze::CustomTensor<T, AF, PF, RT> const& target, unsigned)
    {
        // Serialize tensor
        std::size_t rows = target.rows();
        std::size_t columns = target.columns();
        std::size_t pages = target.pages();
        std::size_t spacing = target.spacing();
        archive << rows << columns << pages << spacing;

        archive << hpx::serialization::make_array(
            target.data(), rows * spacing * pages);
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool TF>), (blaze::DynamicVector<T, TF>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool SO>), (blaze::DynamicMatrix<T, SO>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF, bool TF, typename RT>),
        (blaze::CustomVector<T, AF, PF, TF, RT>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF, bool SO, typename RT>),
        (blaze::CustomMatrix<T, AF, PF, SO, RT>));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (blaze::DynamicTensor<T>));

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T, bool AF, bool PF, typename RT>),
        (blaze::CustomTensor<T, AF, PF, RT>));
#endif
}}

#endif
