//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_STEP_ITERATOR_DEC_20_2018_1047AM)
#define PHYLANX_UTIL_STEP_ITERATOR_DEC_20_2018_1047AM

#include <hpx/util/iterator_adaptor.hpp>

#include <iterator>
#include <type_traits>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    template <typename Iterator>
    class step_iterator
      : public hpx::util::iterator_adaptor<step_iterator<Iterator>, Iterator>
    {
    private:
        using base_type =
            hpx::util::iterator_adaptor<step_iterator<Iterator>, Iterator>;

    public:
        step_iterator() : step_(0) {}

        explicit step_iterator(Iterator const& it, std::ptrdiff_t step = 1)
          : base_type(it), step_(step)
        {}

        template <typename OtherIterator>
        step_iterator(
                step_iterator<OtherIterator> const& it,
                typename std::enable_if<
                    std::is_convertible<OtherIterator, Iterator>::value
                >::type* = nullptr)
          : base_type(t.base()), step_(it.step())
        {}

        std::ptrdiff_t step() const
        {
            return step_;
        }

    private:
        friend class hpx::util::iterator_core_access;

        typename base_type::reference dereference() const
        {
            return transformer_(this->base());
        }

        void advance(typename base_adaptor_type::difference_type n)
        {
            std::advance(this->base(), n * step_);
        }

        void increment()
        {
            std::advance(this->base(), step_);
        }

        void decrement()
        {
            std::advance(this->base(), -step_);
        }

        std::ptrdiff_t step_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // create step iterator from a given base iterator
    template <typename Iterator>
    inline step_iterator<Iterator>
    make_step_iterator(Iterator const& it, std::ptrdiff_t step = 1)
    {
        return step_iterator<Iterator>(it, step);
    }
}}

#endif
