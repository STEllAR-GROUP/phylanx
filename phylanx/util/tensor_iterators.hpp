// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_TENSOR_ITERATORS)
#define PHYLANX_TENSOR_ITERATORS

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)

#include <cstddef>
#include <utility>

#include <hpx/iterator_support/iterator_facade.hpp>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the rowslices (as a whole) of a tensor
    template <typename T>
    class tensor_rowslice_iterator
      : public hpx::util::iterator_facade<tensor_rowslice_iterator<T>,
            blaze::RowSlice<T>, std::random_access_iterator_tag,
            blaze::RowSlice<T>>
    {
    public:
        explicit tensor_rowslice_iterator(T& t, const std::size_t index = 0)
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

        bool equal(tensor_rowslice_iterator const& other) const
        {
            return index_ == other.index_;
        }

        blaze::RowSlice<T> dereference() const
        {
            return blaze::rowslice(*data_, index_);
        }

        std::ptrdiff_t distance_to(tensor_rowslice_iterator const& other) const
        {
            return other.index_ - index_;
        }

    private:
        T* data_;
        std::size_t index_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the columnslices (as a whole) of a tensor
    template <typename T>
    class tensor_columnslice_iterator
      : public hpx::util::iterator_facade<tensor_columnslice_iterator<T>,
            blaze::ColumnSlice<T>, std::random_access_iterator_tag,
            blaze::ColumnSlice<T>>
    {
    public:
        explicit tensor_columnslice_iterator(T& t, const std::size_t index = 0)
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

        bool equal(tensor_columnslice_iterator const& other) const
        {
            return index_ == other.index_;
        }

        blaze::ColumnSlice<T> dereference() const
        {
            return blaze::columnslice(*data_, index_);
        }

        std::ptrdiff_t distance_to(tensor_columnslice_iterator const& other) const
        {
            return other.index_ - index_;
        }

    private:
        T* data_;
        std::size_t index_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the pageslices (as a whole) of a tensor
    template <typename T>
    class tensor_pageslice_iterator
      : public hpx::util::iterator_facade<tensor_pageslice_iterator<T>,
            blaze::PageSlice<T>, std::random_access_iterator_tag,
            blaze::PageSlice<T>>
    {
    public:
        explicit tensor_pageslice_iterator(T& t, const std::size_t index = 0)
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

        bool equal(tensor_pageslice_iterator const& other) const
        {
            return index_ == other.index_;
        }

        blaze::PageSlice<T> dereference() const
        {
            return blaze::pageslice(*data_, index_);
        }

        std::ptrdiff_t distance_to(tensor_pageslice_iterator const& other) const
        {
            return other.index_ - index_;
        }

    private:
        T* data_;
        std::size_t index_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Iterate over the elements of the whole tensor page/row-wise.
    template <typename Tensor>
    class tensor_iterator
      : public hpx::util::iterator_facade<tensor_iterator<Tensor>,
            typename Tensor::ElementType, std::bidirectional_iterator_tag>
    {
        using base_type = hpx::util::iterator_facade<tensor_iterator<Tensor>,
            typename Tensor::ElementType, std::bidirectional_iterator_tag>;

        using base_iterator = typename Tensor::Iterator;

    public:
        explicit tensor_iterator(
                Tensor& t, std::size_t row = 0, std::size_t page = 0)
          : tensor_(&t)
          , page_(page)
          , row_(row)
          , pos_()
        {
            if (page_ != t.pages() && row_ != t.rows())
                pos_ = t.begin(row_, page_);
        }

    private:
        friend class hpx::util::iterator_core_access;

        void increment()
        {
            if (++pos_ == tensor_->end(row_, page_))
            {
                if (++row_ == tensor_->rows())
                {
                    if (++page_ == tensor_->pages())
                    {
                        pos_ = base_iterator();
                    }
                    else
                    {
                        row_ = 0;
                        pos_ = tensor_->begin(row_, page_);
                    }
                }
                else
                {
                    pos_ = tensor_->begin(row_, page_);
                }
            }
        }

        void decrement()
        {
            if (pos_ == tensor_->begin(row_))
            {
                if (row_ == 0)
                {
                    if (page_ == 0)
                    {
                        pos_ = base_iterator();
                    }
                    else
                    {
                        row_ = tensor_->rows() - 1;
                        pos_ = tensor_->end(row_, --page_) - 1;
                    }
                }
                else
                {
                    pos_ = tensor_->end(--row_, page_) - 1;
                }
            }
            else
            {
                --pos_;
            }
        }

        bool equal(tensor_iterator const& other) const
        {
            return pos_ == other.pos_;
        }

        typename base_type::reference dereference() const
        {
            return *pos_;
        }

        std::ptrdiff_t distance_to(tensor_iterator const& other) const
        {
            return
                ((other.page_ - page_) * tensor_->rows() + (other.row_ - row_)) *
                    tensor_->columns() +
                (other.pos_ - other->tensor_->begin(other->row_, other->page_)) -
                (pos_ - tensor_->begin(row_, page_));
        }

    private:
        Tensor* tensor_;
        std::size_t page_;
        std::size_t row_;
        typename Tensor::Iterator pos_;
    };

}}

#endif
#endif
