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

namespace phylanx { namespace execution_tree
{
    struct primitive_argument_type;
}}

namespace phylanx { namespace ir
{
    //////////////////////////////////////////////////////////////////////////
    class PHYLANX_EXPORT reverse_range_iterator
        : public hpx::util::iterator_facade<reverse_range_iterator,
        execution_tree::primitive_argument_type,
        std::input_iterator_tag,
        execution_tree::primitive_argument_type>
    {
    private:
        using int_range_type = std::pair<std::int64_t, std::int64_t>;
        using args_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::reverse_iterator;
        using args_const_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::const_reverse_iterator;
        using iterator_type = util::variant<
            int_range_type,
            args_iterator_type,
            args_const_iterator_type>;

    public:
        reverse_range_iterator(std::int64_t reverse_start, std::int64_t step)
          : it_(std::make_pair(reverse_start, step))
        {
        }

        reverse_range_iterator(args_iterator_type it)
          : it_(it)
        {
        }

        reverse_range_iterator(args_const_iterator_type it)
          : it_(it)
        {
        }

    private:
        friend class hpx::util::iterator_core_access;

        execution_tree::primitive_argument_type dereference() const;
        bool equal(reverse_range_iterator const& other) const;
        void increment();

    private:
        iterator_type it_;
    };

    //////////////////////////////////////////////////////////////////////////
    class PHYLANX_EXPORT range_iterator
      : public hpx::util::iterator_facade<range_iterator,
            execution_tree::primitive_argument_type,
            std::input_iterator_tag,
            execution_tree::primitive_argument_type>
    {
    private:
        using int_range_type = std::pair<std::int64_t, std::int64_t>;
        using args_iterator_type =
            std::vector<execution_tree::primitive_argument_type>::iterator;
        using args_const_iterator_type =
            std::vector<execution_tree::primitive_argument_type>::const_iterator;
        using args_reverse_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::reverse_iterator;
        using args_reverse_const_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::const_reverse_iterator;
        using iterator_type = util::variant<
            int_range_type,
            args_iterator_type,
            args_const_iterator_type>;

    public:
        range_iterator(std::int64_t start, std::int64_t step)
          : it_(std::make_pair(start, step))
        {
        }

        range_iterator(args_iterator_type it)
          : it_(it)
        {
        }

        range_iterator(args_const_iterator_type it)
          : it_(it)
        {
        }

        reverse_range_iterator invert();

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

        //////////////////////////////////////////////////////////////////////////
        range_iterator begin();
        range_iterator end();

        const range_iterator begin() const;
        const range_iterator end() const;

        reverse_range_iterator rbegin();
        reverse_range_iterator rend();

        bool empty() const;

        args_type& args();
        args_type const& args() const;

        //////////////////////////////////////////////////////////////////////////
        range() = default;

        explicit range(
            std::vector<execution_tree::primitive_argument_type> const& data)
          : data_(data)
        {
        }

        explicit range(
            std::vector<execution_tree::primitive_argument_type>&& data)
          : data_(std::move(data))
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

        bool operator==(range const& other) const;

        void serialize(hpx::serialization::output_archive& ar, unsigned);
        void serialize(hpx::serialization::input_archive& ar, unsigned);

    private:
        range_type data_;
    };
}}

#endif
