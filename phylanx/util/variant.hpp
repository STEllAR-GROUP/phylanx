//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2002-2003 Eric Friedman, Itay Maman
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_VARIANT_HPP)
#define PHYLANX_UTIL_VARIANT_HPP

#include <phylanx/config.hpp>

#include <phylanx/util/detail/variant.hpp>

#include <utility>

namespace phylanx { namespace util
{
    using mpark::variant;
    using mpark::monostate;

    using mpark::holds_alternative;
    using mpark::get;
    using mpark::get_if;
    using mpark::visit;

    ///////////////////////////////////////////////////////////////////////////
    // class template recursive_wrapper
    template <typename T>
    class recursive_wrapper
    {
    public:    // typedefs
        using type = T;

    private:    // representation
        T* p_;

    public:
        ~recursive_wrapper()
        {
            delete p_;
        }

        recursive_wrapper()
          : p_(new T)
        {
        }

        recursive_wrapper(recursive_wrapper const& operand)
          : p_(new T(operand.get()))
        {
        }

        recursive_wrapper(T const& operand)
          : p_(new T(operand))
        {
        }

        recursive_wrapper(recursive_wrapper && operand) noexcept
          : p_(operand.p_)
        {
            operand.p_ = nullptr;
        }

        recursive_wrapper(T && operand)
          : p_(new T(std::move(operand)))
        {
        }

    private: // helpers, for modifiers (below)
        void assign(T const& rhs)
        {
            this->get() = rhs;
        }

    public: // modifiers
        recursive_wrapper& operator=(recursive_wrapper const& rhs)
        {
            assign(rhs.get());
            return *this;
        }
        recursive_wrapper& operator=(recursive_wrapper && rhs) noexcept
        {
            swap(rhs);
            return *this;
        }

        recursive_wrapper& operator=(T const& rhs)
        {
            assign(rhs);
            return *this;
        }
        recursive_wrapper& operator=(T && rhs)
        {
            get() = std::move(rhs);
            return *this;
        }

        void swap(recursive_wrapper& operand) noexcept
        {
            T* temp = operand.p_;
            operand.p_ = p_;
            p_ = temp;
        }

    public:    // queries
        T& get()
        {
            return *get_pointer();
        }
        T const& get() const
        {
            return *get_pointer();
        }

        T* get_pointer()
        {
            return p_;
        }
        T const* get_pointer() const
        {
            return p_;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // Swaps two recursive_wrapper<T> objects of the same type T.
    template <typename T>
    inline void swap(
        recursive_wrapper<T>& lhs, recursive_wrapper<T>& rhs) noexcept
    {
        lhs.swap(rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool operator==(
        recursive_wrapper<T> const& lhs, recursive_wrapper<T> const& rhs)
    {
        return lhs.get() == rhs.get();
    }

    template <typename T>
    bool operator!=(
        recursive_wrapper<T> const& lhs, recursive_wrapper<T> const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    std::ostream& operator<<(std::ostream& os, recursive_wrapper<T> const& val)
    {
        return os << val.get();
    }
}}

#endif
