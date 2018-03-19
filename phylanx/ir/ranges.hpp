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

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace ir
{
    //////////////////////////////////////////////////////////////////////////
    class PHYLANX_EXPORT range_iterator
      : public hpx::util::iterator_facade<range_iterator,
            execution_tree::primitive_argument_type,
            std::input_iterator_tag,
            execution_tree::primitive_argument_type>
    {
    private:
        using int_range_type = std::pair<std::int64_t, std::int64_t>;
        using arg_range_type =
            std::vector<execution_tree::primitive_argument_type>::iterator;
        using iterator_type = util::variant<int_range_type, arg_range_type>;

    public:
        range_iterator(int_range_type it)
          : it_(it)
        {
        }

        range_iterator(std::int64_t start, std::int64_t step)
          : it_(std::make_pair(start, step))
        {
        }

        range_iterator(arg_range_type it)
          : it_(it)
        {
        }

    private:
        friend class hpx::util::iterator_core_access;

        execution_tree::primitive_argument_type dereference() const;
        bool equal(range_iterator const& other) const;
        void increment();

    private:
        iterator_type it_;
    };

    //////////////////////////////////////////////////////////////////////////
    class PHYLANX_EXPORT range
    {
    public:
        using int_range_type =
            hpx::util::tuple<std::int64_t, std::int64_t, std::int64_t>;
        using args_type = std::vector<execution_tree::primitive_argument_type>;
        using arg_pair_type = std::pair<range_iterator, range_iterator>;
        using range_type =
            util::variant<int_range_type, args_type, arg_pair_type>;

        range_iterator begin();
        range_iterator end();

        explicit range(
            std::vector<execution_tree::primitive_argument_type> data)
          : data_(data)
        {
        }

        explicit range(
            std::vector<execution_tree::primitive_argument_type>::iterator x,
            std::vector<execution_tree::primitive_argument_type>::iterator y)
          : data_(std::make_pair(range_iterator{x}, range_iterator{y}))
        {
        }

        explicit range(
            std::int64_t start, std::int64_t stop, std::int64_t step = 1)
          : data_(hpx::util::make_tuple(start, stop, step))
        {
        }

        explicit range(std::int64_t stop)
          : data_(hpx::util::make_tuple(
              static_cast<std::int64_t>(0),
              stop,
              static_cast<std::int64_t>(1)))
        {
        }

    private:
        range_type data_;
    };
}}

#endif
