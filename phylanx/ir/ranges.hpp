// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_RANGES)
#define PHYLANX_IR_RANGES

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/base_primitive.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>

namespace phylanx { namespace ir
{
    //////////////////////////////////////////////////////////////////////////
    template <typename T>
    class range_iterator
        : public hpx::util::iterator_facade<range_iterator<T>>,
        T,
        std::forward_iterator_tag,
        T const&,
        std::ptrdiff_t,
        T const*>
    {
    public:
        friend class hpx::util::iterator_core_access;

        typename base_type::reference dereference() const
        {
            ;
        }

        bool equal(range_iterator const& other) const
        {
            ;
        }

        void advance(typename base_type::difference_type n)
        {
            ;
        }

        void increment()
        {
            ;
        }

        void decrement()
        {
            ;
        }

        typename base_type::difference_type distance_to(
            range_iterator const& other) const
        {
            ;
        }

        std::size_t index_;
    };

    template <typename T>
    class range
    {
    public:
        using args_type = std::vector<execution_tree::primitive_argument_type>;
        using range_type = util::variant<args_type>;

        range_iterator<T> begin();
        range_iterator<T> end();

        explicit range(std::vector<execution_tree::primitive_argument_type> data) : data_(std::move(data));

    private:
        range_type data_;
    };
}}

#endif
