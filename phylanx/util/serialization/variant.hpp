//  Copyright (c) 2015 Anton Bikineev
//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_VARIANT_SERIALIZATION_HPP)
#define PHYLANX_UTIL_VARIANT_SERIALIZATION_HPP

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <utility>

namespace hpx { namespace serialization
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct variant_save_visitor
        {
            variant_save_visitor(output_archive& ar)
              : ar_(ar)
            {}

            template <typename T>
            void operator()(T const& value) const
            {
                ar_ << value;
            }

        private:
            output_archive& ar_;
        };

        template <typename ... Ts>
        struct variant_impl;

        template <typename T, typename ... Ts>
        struct variant_impl<T, Ts...>
        {
            template <typename V>
            static void load(input_archive& ar, std::size_t which, V& v)
            {
                // note: A non-intrusive implementation (such as this one)
                // necessary has to copy the value.  This wouldn't be necessary
                // with an implementation that de-serialized to the address of
                // the aligned storage included in the variant.
                if (which == 0)
                {
                    T value;
                    ar >> value;
                    v.template emplace<T>(std::move(value));
                    return;
                }
                variant_impl<Ts...>::load(ar, which - 1, v);
            }
        };

        template <>
        struct variant_impl<>
        {
            template <typename V>
            static void load(
                input_archive& /*ar*/, std::size_t /*which*/, V& /*v*/)
            {
            }
        };
    }

    template <typename ... Ts>
    void save(output_archive& ar, phylanx::util::variant<Ts...> const& v, unsigned)
    {
        std::size_t which = v.index();
        ar << which;
        detail::variant_save_visitor visitor(ar);
        phylanx::util::visit(visitor, v);
    }

    template <typename ... Ts>
    void load(input_archive& ar, phylanx::util::variant<Ts...>& v, unsigned)
    {
        std::size_t which;
        ar >> which;
        if (which >= sizeof...(Ts))
        {
            // this might happen if a type was removed from the list of variant
            // types
            HPX_THROW_EXCEPTION(serialization_error
              , "load<Archive, Variant, version>"
              , "type was removed from the list of variant types");
        }
        detail::variant_impl<Ts...>::load(ar, which, v);
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename ... Ts>), (phylanx::util::variant<Ts...>));

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    void save(output_archive& ar, phylanx::util::recursive_wrapper<T> const& rw,
        unsigned)
    {
        ar << rw.get();
    }

    template <typename T>
    void load(
        input_archive& ar, phylanx::util::recursive_wrapper<T>& rw, unsigned)
    {
        T value;
        ar >> value;
        rw = std::move(value);
    }

    HPX_SERIALIZATION_SPLIT_FREE_TEMPLATE(
        (template <typename T>), (phylanx::util::recursive_wrapper<T>));
}}

#endif
