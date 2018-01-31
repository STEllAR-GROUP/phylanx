//  Copyright (c) 2001-2011 Joel de Guzman
//  Copyright (c) 2001-2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// phylanxinspect:nodeprecatedname:boost::mpl

#if !defined(PHYLANX_AST_PARSER_EXTENDED_VARIANT_HPP)
#define PHYLANX_AST_PARSER_EXTENDED_VARIANT_HPP

#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>

#include <boost/mpl/vector.hpp>

#include <cstddef>
#include <type_traits>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace ast { namespace parser
{
    template <typename ... Ts>
    struct extended_variant
    {
        // tell spirit that this is an adapted variant
        struct adapted_variant_tag;

        using variant_type = util::variant<Ts...>;
        using types = boost::mpl::vector<Ts...>;
        using base_type = extended_variant<Ts...>;

        extended_variant() = default;

        template <typename T,
            typename U = typename std::enable_if<
               !std::is_same<
                    typename std::decay<T>::type, extended_variant
                >::value
            >::type>
        extended_variant(T&& var)
          : var(std::forward<T>(var))
        {
        }

        template <typename F>
        auto apply_visitor(F && v) -> decltype(
            util::visit(std::forward<F>(v), std::declval<variant_type>()))
        {
            return util::visit(std::forward<F>(v), var);
        }

        template <typename F>
        auto apply_visitor(F && v) const -> decltype(
            util::visit(std::forward<F>(v), std::declval<variant_type>()))
        {
            return util::visit(std::forward<F>(v), var);
        }

        variant_type const& get() const
        {
            return var;
        }

        variant_type& get()
        {
            return var;
        }

        constexpr std::size_t index() const
        {
            return var.index();
        }

        void swap(extended_variant& rhs) noexcept(
            std::declval<variant_type>().swap(std::declval<variant_type>()))
        {
            var.swap(rhs.var);
        }

        variant_type var;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename... Ts>
    bool operator==(
        extended_variant<Ts...> const& lhs, extended_variant<Ts...> const& rhs)
    {
        return lhs.var == rhs.var;
    }
    template <typename... Ts>
    bool operator!=(
        extended_variant<Ts...> const& lhs, extended_variant<Ts...> const& rhs)
    {
        return !(lhs == rhs);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Archive, typename ... Ts>
    void serialize(Archive& ar, extended_variant<Ts...>& v, unsigned)
    {
        ar & v.var;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename F, typename ... Ts>
    auto visit(F && f, extended_variant<Ts...> const& v)
    {
        return util::visit(std::forward<F>(f), v.var);
    }
    template <typename F, typename ... Ts>
    auto visit(F && f, extended_variant<Ts...>& v)
    {
        return util::visit(std::forward<F>(f), v.var);
    }

    template <typename F, typename... Ts1, typename... Ts2>
    auto visit(F&& f, extended_variant<Ts1...> const& v1,
        extended_variant<Ts2...> const& v2)
    {
        return util::visit(std::forward<F>(f), v1.var, v2.var);
    }
    template <typename F, typename... Ts1, typename... Ts2>
    auto visit(
        F&& f, extended_variant<Ts1...>& v1, extended_variant<Ts2...>& v2)
    {
        return util::visit(std::forward<F>(f), v1.var, v2.var);
    }
}}}

namespace boost
{
    template <typename T, typename... Types>
    inline T const&
    get(phylanx::ast::parser::extended_variant<Types...> const& x)
    {
        return boost::get<T>(x.get());
    }

    template <typename T, typename... Types>
    inline T&
    get(phylanx::ast::parser::extended_variant<Types...>& x)
    {
        return boost::get<T>(x.get());
    }

    template <typename T, typename... Types>
    inline T const*
    get(phylanx::ast::parser::extended_variant<Types...> const* x)
    {
        return boost::get<T>(&x->get());
    }

    template <typename T, typename... Types>
    inline T*
    get(phylanx::ast::parser::extended_variant<Types...>* x)
    {
        return boost::get<T>(&x->get());
    }
}

#endif
