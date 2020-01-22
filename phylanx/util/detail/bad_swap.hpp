// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_DETAIL_BAD_SWAP)
#define PHYLANX_UTIL_DETAIL_BAD_SWAP

#include <phylanx/config.hpp>

#include <algorithm>

#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////
namespace blaze
{
    namespace _row_swap
    {
        using std::swap;

        template <typename T>
        void check_swap() noexcept(
            noexcept(swap(std::declval<T&>(), std::declval<T&>())))
        {}
    }

    // BADBAD: This overload of swap is necessary to work around the problems
    //         caused by matrix_row_iterator not being a real random access
    //         iterator. Dereferencing matrix_row_iterator does not yield a
    //         true reference but only a temporary blaze::Row holding true
    //         references.
    //
    // A real fix for this problem is proposed in PR0022R0
    // (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r0.html)
    //
    using std::iter_swap;

    template <typename T>
    void swap(Row<T>&& x, Row<T>&& y) noexcept(
        noexcept(_row_swap::check_swap<T>()) &&
        noexcept(*std::declval<typename T::Iterator>()))
    {
        for (auto a = x.begin(), b = y.begin(); a != x.end() && b != y.end();
            ++a, ++b)
        {
            iter_swap(a, b);
        }
    }

    namespace _column_swap
    {
        using std::swap;

        template <typename T>
        void check_swap() noexcept(
            noexcept(swap(std::declval<T&>(), std::declval<T&>())))
        {}
    }

    // BADBAD: This overload of swap is necessary to work around the problems
    //         caused by matrix_column_iterator not being a real random access
    //         iterator. Dereferencing matrix_column_iterator does not yield a
    //         true reference but only a temporary blaze::Row holding true
    //         references.
    //
    // A real fix for this problem is proposed in PR0022R0
    // (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0022r0.html)
    //
    using std::iter_swap;

    template <typename T>
    void swap(Column<T>&& x, Column<T>&& y) noexcept(
        noexcept(_column_swap::check_swap<T>()) &&
        noexcept(*std::declval<typename T::Iterator>()))
    {
        for (auto a = x.begin(), b = y.begin(); a != x.end() && b != y.end();
            ++a, ++b)
        {
            iter_swap(a, b);
        }
    }
}
#endif
