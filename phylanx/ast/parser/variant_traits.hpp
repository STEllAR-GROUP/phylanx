//  Copyright (c) 2001-2010 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_AST_PARSER_VARIANT_TRAITS_HPP)
#define PHYLANX_AST_PARSER_VARIANT_TRAITS_HPP

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <boost/spirit/home/qi/detail/assign_to.hpp>
#include <boost/spirit/home/qi/detail/pass_container.hpp>
#include <boost/spirit/home/support/attributes.hpp>
#include <boost/mpl/bool.hpp>

namespace boost { namespace spirit { namespace traits
{
    template <typename Domain, typename ... Ts>
    struct not_is_variant<phylanx::util::variant<Ts...>, Domain>
      : mpl::false_
    {};

//     template <typename T, typename ... Ts>
//     struct assign_to_attribute_from_value<phylanx::util::variant<Ts...>, T>
//     {
//         static void
//         call(T const& val, phylanx::util::variant<Ts...>& attr)
//         {
//             attr = val;
//         }
//     };

    template <typename T, typename Expected>
    struct is_weak_substitute<phylanx::util::variant<T>, Expected>
      : is_weak_substitute<T, Expected>
    {};

    template <typename T0, typename T1, typename... TN, typename Expected>
    struct is_weak_substitute<phylanx::util::variant<T0, T1, TN...>, Expected>
      : mpl::bool_<
            is_weak_substitute<T0, Expected>::type::value &&
            is_weak_substitute<
                phylanx::util::variant<T1, TN...>, Expected
            >::type::value>
    {};

    template <typename T>
    struct is_container<phylanx::util::variant<T>>
      : is_container<T>
    {};

    template <typename T0, typename T1, typename... TN>
    struct is_container<phylanx::util::variant<T0, T1, TN...>>
      : mpl::bool_<is_container<T0>::value ||
            is_container<phylanx::util::variant<T1, TN...>>::value>
    {};

    ///////////////////////////////////////////////////////////////////////////
    // return the type currently stored in the given variant
    template <typename ... Ts>
    struct variant_which<phylanx::util::variant<Ts...> >
    {
        static int call(phylanx::util::variant<Ts...> const& v)
        {
            return v.index();
        }
    };
}}}

namespace boost { namespace spirit { namespace qi { namespace detail
{
    template <typename Container, typename ValueType, typename Sequence,
        typename T>
    struct pass_through_container<Container, ValueType,
            phylanx::util::variant<T>, Sequence>
      : pass_through_container<Container, ValueType, T, Sequence>
    {};

    template <typename Container, typename ValueType, typename Sequence,
        typename T0, typename... TN>
    struct pass_through_container<Container, ValueType,
            phylanx::util::variant<T0, TN...>, Sequence>
      : mpl::bool_<
            pass_through_container<
                Container, ValueType, T0, Sequence
            >::type::value ||
            pass_through_container<
                Container, ValueType, phylanx::util::variant<TN...>, Sequence
            >::type::value>
    {};
}}}}

#endif
