// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_RANGES)
#define PHYLANX_IR_RANGES

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/serialization.hpp>
#include <hpx/include/util.hpp>

#include <array>
#include <cstddef>
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
    ///////////////////////////////////////////////////////////////////////////
    // Represent a slice as a triplet of numbers (start/stop/step) and a flag
    // indicating whether stop/step were given in the first place.
    // Note: some slicing operations on vectors and matrices behave differently
    // for a[1] (which returns a scalar) and a[1, 2] (which returns a vector
    // consisting of one element).
    struct slicing_indices
    {
        explicit slicing_indices(
                std::int64_t start = 0, bool single_value = true)
          : slice_{start, 0, 0}
          , single_value_(single_value)
        {}

        slicing_indices(std::int64_t start, std::int64_t stop, std::int64_t step)
          : slice_{start, stop, step}
          , single_value_(false)
        {}

        void start(std::int64_t val)
        {
            slice_[0] = val;
        }
        void start(std::int64_t val, bool single_value)
        {
            single_value_ = single_value;
            slice_[0] = val;
        }
        std::int64_t start() const
        {
            return slice_[0];
        }

        void stop(std::int64_t val)
        {
            slice_[1] = val;
        }
        void stop(std::int64_t val, bool single_value)
        {
            single_value_ = single_value;
            slice_[1] = val;
        }
        std::int64_t stop() const
        {
            return single_value_ ? slice_[0] + 1 : slice_[1];
        }

        void step(std::int64_t val)
        {
            slice_[2] = val;
        }
        void step(std::int64_t val, bool single_value)
        {
            single_value_ = single_value;
            slice_[2] = val;
        }
        std::int64_t step() const
        {
            return single_value_ ? 1ll : slice_[2];
        }

        bool single_value() const
        {
            return single_value_;
        }

        std::int64_t span() const
        {
            return slice_[2] > 0 ? slice_[1] - slice_[0] : slice_[0] - slice_[1];
        }

    private:
        friend bool operator==(
            slicing_indices const& lhs, slicing_indices const& rhs);
        friend bool operator!=(
            slicing_indices const& lhs, slicing_indices const& rhs);

        std::array<std::int64_t, 3> slice_;     // start/stop/step
        bool single_value_;                     // ignore stop/step
    };

    inline bool operator==(
        slicing_indices const& lhs, slicing_indices const& rhs)
    {
        return lhs.single_value_ == rhs.single_value_ &&
            lhs.slice_ == rhs.slice_;
    }

    inline bool operator!=(
        slicing_indices const& lhs, slicing_indices const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    class PHYLANX_EXPORT reverse_range_iterator
      : public hpx::util::iterator_facade<reverse_range_iterator,
            execution_tree::primitive_argument_type,
            std::input_iterator_tag,
            execution_tree::primitive_argument_type>
    {
    private:
        using int_range_type = slicing_indices;
        using args_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::reverse_iterator;
        using args_const_iterator_type = std::vector<
            execution_tree::primitive_argument_type>::const_reverse_iterator;
        using iterator_type = util::variant<
            int_range_type, args_iterator_type, args_const_iterator_type>;

    public:
        reverse_range_iterator(std::int64_t reverse_start, std::int64_t step)
          : it_(int_range_type{reverse_start, 0ll, step})
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
        using int_range_type = slicing_indices;
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
          : it_(int_range_type{start, 0ll, step})
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

        reverse_range_iterator invert() const;

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
    private:
        using int_range_type = slicing_indices;
        using args_type = std::vector<execution_tree::primitive_argument_type>;
        using wrapped_args_type = phylanx::util::recursive_wrapper<args_type>;
        using arg_pair_type = std::pair<range_iterator, range_iterator>;
        using range_type =
            util::variant<int_range_type, wrapped_args_type, arg_pair_type>;

    public:
        ///////////////////////////////////////////////////////////////////////
        range_iterator begin() const;
        range_iterator end() const;

        reverse_range_iterator rbegin() const;
        reverse_range_iterator rend() const;

        std::ptrdiff_t size() const;

        bool empty() const;

        args_type& args();
        args_type const& args() const;

        args_type copy() const;

        range ref() const;

        bool is_ref() const;

        std::size_t index() const { return data_.index(); }

        int_range_type& xrange();
        int_range_type const& xrange() const;

        //////////////////////////////////////////////////////////////////////////
        range()
          : data_(args_type{})
        {
        }

        range(args_type const& data)
          : data_(data)
        {
        }

        range(args_type&& data)
          : data_(std::move(data))
        {
        }

        range(args_type::iterator x, args_type::iterator y)
          : data_(std::make_pair(range_iterator{x}, range_iterator{y}))
        {
        }

        range(range_iterator x, range_iterator y)
          : data_(std::make_pair(x, y))
        {
        }

        range(std::int64_t start, std::int64_t stop, std::int64_t step = 1)
            : data_(int_range_type{start, stop, step})
        {
        }

        range(std::int64_t stop)
          : data_(int_range_type{0ll, stop, 1ll})
        {
        }

        range(std::int64_t start, bool single_value)
          : data_(int_range_type{start, single_value})
        {
        }

        friend bool operator==(range const&, range const&);
        friend bool operator!=(range const&, range const&);

        void serialize(hpx::serialization::output_archive& ar, unsigned);
        void serialize(hpx::serialization::input_archive& ar, unsigned);

    private:
        range_type data_;
    };
}}

#endif
