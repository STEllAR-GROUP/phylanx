// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_IR_RANGES)
#define PHYLANX_IR_RANGES

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/include/util.hpp>
#include <hpx/util/internal_allocator.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    struct primitive_argument_type;

    using primitive_argument_allocator = hpx::util::internal_allocator<>;
//        std::allocator<primitive_argument_type>;

    template <typename T>
    using arguments_allocator = typename std::allocator_traits<
        primitive_argument_allocator>::template rebind_alloc<T>;

    using primitive_arguments_type = std::vector<primitive_argument_type,
        arguments_allocator<primitive_argument_type>>;
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
          : start_(start), stop_(0), step_(0)
          , single_value_(single_value)
        {}

        slicing_indices(std::int64_t start, std::int64_t stop, std::int64_t step)
          : start_(start), stop_(stop), step_(step)
          , single_value_(false)
        {}

        void start(std::int64_t val)
        {
            start_ = val;
        }
        void start(std::int64_t val, bool single_value)
        {
            single_value_ = single_value;
            start_ = val;
        }
        std::int64_t start() const
        {
            return start_;
        }

        void stop(std::int64_t val)
        {
            stop_ = val;
        }
        void stop(std::int64_t val, bool single_value)
        {
            single_value_ = single_value;
            stop_ = val;
        }
        std::int64_t stop() const
        {
            return single_value_ ? start_ + 1 : stop_;
        }

        void step(std::int64_t val)
        {
            step_ = val;
        }
        void step(std::int64_t val, bool single_value)
        {
            single_value_ = single_value;
            step_ = val;
        }
        std::int64_t step() const
        {
            return single_value_ ? 1ll : step_;
        }

        bool single_value() const
        {
            return single_value_;
        }
        void single_value(bool single_value)
        {
            single_value_ = single_value;
        }

        std::int64_t span() const
        {
            if (single_value_)
            {
                return 1ll;
            }
            return step_ > 0 ? stop_ - start_ : start_ - stop_;
        }

        std::int64_t size() const
        {
            if (single_value_)
            {
                return 1ll;
            }

            if (step_ > 0)
            {
                return (stop_ - start_ + step_ - 1ll) / step_;
            }

            return (start_ - stop_ - step_ - 1ll) / -step_;
        }

    private:
        friend PHYLANX_EXPORT bool operator==(
            slicing_indices const& lhs, slicing_indices const& rhs);
        friend PHYLANX_EXPORT bool operator!=(
            slicing_indices const& lhs, slicing_indices const& rhs);

        std::int64_t start_;        // start/stop/step
        std::int64_t stop_;        // start/stop/step
        std::int64_t step_;        // start/stop/step
        bool single_value_;         // ignore stop/step
    };

    inline bool operator==(
        slicing_indices const& lhs, slicing_indices const& rhs)
    {
        return lhs.single_value_ == rhs.single_value_ &&
            lhs.start_ == rhs.start_ && lhs.stop_ == rhs.stop_ &&
            lhs.step_ == rhs.step_;
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
            execution_tree::primitive_arguments_type::iterator;
        using args_const_iterator_type =
            execution_tree::primitive_arguments_type::const_iterator;
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
        using args_type = execution_tree::primitive_arguments_type;
        using wrapped_args_type = phylanx::util::recursive_wrapper<args_type>;
        using arg_pair_type = std::pair<range_iterator, range_iterator>;
        using range_type =
            util::variant<int_range_type, wrapped_args_type, arg_pair_type>;

    private:
        template <typename... Ts>
        static execution_tree::primitive_arguments_type
            convert_span(std::string key, Ts&&... ts)
        {
            execution_tree::primitive_arguments_type data;
            data.reserve(sizeof...(Ts) + 1);

            data.emplace_back(std::move(key));

            int dummy[] = {
                0, (data.emplace_back(std::forward<Ts>(ts)), 0)...
            };
            (void)dummy;

            return data;
        }

    public:
        ///////////////////////////////////////////////////////////////////////
        range_iterator begin() const;
        range_iterator end() const;

        reverse_range_iterator rbegin() const;
        reverse_range_iterator rend() const;

        std::ptrdiff_t size() const;

        bool empty() const;

        bool is_args() const;
        args_type& args();
        args_type const& args() const;

        bool is_args_ref() const;
        arg_pair_type& args_ref();
        arg_pair_type const& args_ref() const;

        args_type copy() const;

        bool is_ref() const;
        range ref() const;

        bool is_xrange() const;
        int_range_type& xrange();
        int_range_type const& xrange() const;

        std::size_t index() const { return data_.index(); }

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
            if (step == 0)
            {
                HPX_THROW_EXCEPTION(hpx::invalid_status,
                    "phylanx::ir::range::range",
                    "range step must not be zero");
            }
        }

        range(std::int64_t stop)
          : data_(int_range_type{0ll, stop, 1ll})
        {
        }

        range(std::int64_t start, bool single_value)
          : data_(int_range_type{start, single_value})
        {
        }

        // construct an annotation
        template <typename... Ts>
        range(char const* key, Ts&& ... ts)
          : data_(convert_span(std::string(key), std::forward<Ts>(ts)...))
        {
        }

        template <typename... Ts>
        range(std::string const& key, Ts&& ... ts)
          : data_(convert_span(key, std::forward<Ts>(ts)...))
        {
        }

        friend PHYLANX_EXPORT bool operator==(range const&, range const&);
        friend PHYLANX_EXPORT bool operator!=(range const&, range const&);

    private:
        friend class hpx::serialization::access;
        void serialize(hpx::serialization::output_archive& ar, unsigned);
        void serialize(hpx::serialization::input_archive& ar, unsigned);

        range_type data_;
    };
}}

#endif
