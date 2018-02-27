//  Copyright (c) 2018 Parsa Amini
//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIX_ITERATORS)
#define PHYLANX_MATRIX_ITERATORS

#include <blaze/Math.h>

namespace phylanx { namespace util
{
    template <typename T>
    class matrix_row_iterator
        : public hpx::util::iterator_facade<
            matrix_row_iterator<T>,
            blaze::Row<T>,
            std::random_access_iterator_tag,
            blaze::Row<T>>
    {
    public:
        explicit matrix_row_iterator(T& t, const std::size_t index = 0)
            : data_(t)
            , index_(index)
        {
        }

    private:
        friend class hpx::util::iterator_core_access;

        void increment()
        {
            ++index_;
        }
        void decrement()
        {
            --index_;
        }
        void advance(std::size_t n)
        {
            index_ += n;
        }
        bool equal(matrix_row_iterator const& other) const
        {
            return index_ == other.index_;
        }
        blaze::Row<T> dereference() const
        {
            return blaze::row(data_, index_);
        }

    private:
        T & data_;
        std::size_t index_;
    };

    template <typename T>
    class matrix_column_iterator
        : public hpx::util::iterator_facade<
        matrix_column_iterator<T>,
        blaze::Column<T>,
        std::random_access_iterator_tag,
        blaze::Column<T>>
    {
    public:
        explicit matrix_column_iterator(T& t, const std::size_t index = 0)
            : data_(t)
            , index_(index)
        {
        }

    private:
        friend class hpx::util::iterator_core_access;

        void increment()
        {
            ++index_;
        }
        void decrement()
        {
            --index_;
        }
        void advance(std::size_t n)
        {
            index_ += n;
        }
        bool equal(matrix_column_iterator const& other) const
        {
            return index_ == other.index_;
        }
        blaze::Column<T> dereference() const
        {
            return blaze::column(data_, index_);
        }

    private:
        T & data_;
        std::size_t index_;
    };
}}

#endif
