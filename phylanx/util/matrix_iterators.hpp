// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_MATRIX_ITERATORS)
#define PHYLANX_MATRIX_ITERATORS

#include <cstddef>
#include <utility>

#include <hpx/iterator_support/iterator_facade.hpp>

#include <blaze/Math.h>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the rows (as a whole) of a matrix
    template <typename T>
    class matrix_row_iterator
      : public hpx::util::iterator_facade<matrix_row_iterator<T>, blaze::Row<T>,
            std::random_access_iterator_tag, blaze::Row<T>>
    {
    public:
        explicit matrix_row_iterator(T& t, const std::size_t index = 0)
          : data_(&t)
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
            return blaze::row(*data_, index_);
        }

        std::ptrdiff_t distance_to(matrix_row_iterator const& other) const
        {
            return other.index_ - index_;
        }

    private:
        T* data_;
        std::size_t index_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the columns (as a whole) of a matrix
    template <typename T>
    class matrix_column_iterator
      : public hpx::util::iterator_facade<matrix_column_iterator<T>,
            blaze::Column<T>, std::random_access_iterator_tag, blaze::Column<T>>
    {
    public:
        explicit matrix_column_iterator(T& t, const std::size_t index = 0)
          : data_(&t)
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
            return blaze::column(*data_, index_);
        }

        std::ptrdiff_t distance_to(matrix_column_iterator const& other) const
        {
            return other.index_ - index_;
        }

    private:
        T* data_;
        std::size_t index_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the elements of the whole matrix row-wise.
    template <typename Matrix>
    class matrix_iterator
      : public hpx::util::iterator_facade<matrix_iterator<Matrix>,
            typename Matrix::ElementType, std::bidirectional_iterator_tag>
    {
        using base_type = hpx::util::iterator_facade<matrix_iterator<Matrix>,
            typename Matrix::ElementType, std::bidirectional_iterator_tag>;

        using base_iterator = typename Matrix::Iterator;

    public:
        explicit matrix_iterator(Matrix& m, std::size_t row = 0)
          : matrix_(&m)
          , row_(row)
          , pos_()
        {
            if (row_ != m.rows())
                pos_ = m.begin(row_);
        }

    private:
        friend class hpx::util::iterator_core_access;

        void increment()
        {
            if (++pos_ == matrix_->end(row_))
            {
                if (++row_ == matrix_->rows())
                {
                    pos_ = base_iterator();
                }
                else
                {
                    pos_ = matrix_->begin(row_);
                }
            }
        }

        void decrement()
        {
            if (pos_ == matrix_->begin(row_))
            {
                if (row_ == 0)
                {
                    pos_ = base_iterator();
                }
                else
                {
                    pos_ = matrix_->end(--row_) - 1;
                }
            }
            else
            {
                --pos_;
            }
        }

        bool equal(matrix_iterator const& other) const
        {
            return pos_ == other.pos_;
        }

        typename base_type::reference dereference() const
        {
            return *pos_;
        }

        std::ptrdiff_t distance_to(matrix_iterator const& other) const
        {
            return ((other.row_ - row_) * matrix_->columns()) +
                (other.pos_ - other->matrix_->begin(other->row_)) -
                (pos_ - matrix_->begin(row_));
        }

    private:
        Matrix* matrix_;
        std::size_t row_;
        typename Matrix::Iterator pos_;
    };

}}

#endif
