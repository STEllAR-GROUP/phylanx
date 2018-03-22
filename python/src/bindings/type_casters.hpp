//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PYBIND11_TYPE_CASTERS_HPP)
#define PYBIND11_TYPE_CASTERS_HPP

#include <phylanx/phylanx.hpp>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// Pybind11 V2.3 changed the interface for the description strings
#if defined(PYBIND11_DESCR)
#define PHYLANX_PYBIND_DESCR PYBIND11_DESCR
#define PHYLANX_PYBIND_DESCR_NAME(x) name() { return x; }
#define PHYLANX_PYBIND_DESCR_GETNAME() name()
#else
#define PHYLANX_PYBIND_DESCR constexpr auto
#define PHYLANX_PYBIND_DESCR_NAME(x) name = x;
#define PHYLANX_PYBIND_DESCR_GETNAME() name
#endif

// older versions of pybind11 don't support variant-like types
namespace pybind11 { namespace detail
{
    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::variant and phylanx::ast::parser::extended_variant

#if !(defined(_MSC_VER) && _MSC_VER >= 1912 && defined(_HAS_CXX17) && (_HAS_CXX17 != 0))
    template <typename... Ts>
    struct type_caster<phylanx::util::variant<Ts...>>
      : variant_caster<phylanx::util::variant<Ts...>>
    {
    };
#endif

    template <typename... Ts>
    struct type_caster<phylanx::ast::parser::extended_variant<Ts...>>
      : variant_caster<phylanx::ast::parser::extended_variant<Ts...>>
    {
    };

    ///////////////////////////////////////////////////////////////////////////
    // Expose phylanx::util::recursive_wrapper
    template <typename T>
    inline PHYLANX_PYBIND_DESCR get_name()
    {
        return make_caster<T>::PHYLANX_PYBIND_DESCR_GETNAME();
    }

    // specialize get_name for vector<primitive_argument_type> to avoid infinite recursion
    template <>
    inline PHYLANX_PYBIND_DESCR
    get_name<std::vector<phylanx::execution_tree::primitive_argument_type>>()
    {
#if defined(_DEBUG)
        return _("List[_phylanxd.execution_tree.primitive_argument_type]");
#else
        return _("List[_phylanx.execution_tree.primitive_argument_type]");
#endif
    }

    template <>
    inline PHYLANX_PYBIND_DESCR
    get_name<phylanx::execution_tree::primitive_argument_type>()
    {
#if defined(_DEBUG)
        return _("_phylanxd.execution_tree.primitive_argument_type");
#else
        return _("_phylanx.execution_tree.primitive_argument_type");
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class type_caster<phylanx::ir::node_data<T>>
    {
        bool load0d(handle src, bool convert)
        {
            auto caster = make_caster<T>();
            if (caster.load(src, convert))
            {
                value = cast_op<T>(caster);
                return true;
            }
            return false;
        }

        bool load1d(handle src, bool convert)
        {
            auto caster = make_caster<std::vector<T>>();
            if (caster.load(src, convert))
            {
                value = cast_op<std::vector<T>>(caster);
                return true;
            }
            return false;
        }

        bool load2d(handle src, bool convert)
        {
            auto caster = make_caster<std::vector<std::vector<T>>>();
            if (caster.load(src, convert))
            {
                value = cast_op<std::vector<std::vector<T>>>(caster);
                return true;
            }
            return false;
        }

    public:
        bool load(handle src, bool convert)
        {
            return load0d(src, convert) || load1d(src, convert) ||
                load2d(src, convert);
        }

        template <typename NodeData>
        static handle cast(
            NodeData&& src, return_value_policy policy, handle parent)
        {
            switch (src.index())
            {
            case 0:                     // T
                return make_caster<T>::cast(
                    std::forward<NodeData>(src).scalar(), policy, parent);

            case 1: HPX_FALLTHROUGH;    // blaze::DynamicVector<T>
            case 3:                     // blaze::CustomVector<T>
                return make_caster<std::vector<T>>::cast(
                    std::forward<NodeData>(src).as_vector(), policy, parent);

            case 2: HPX_FALLTHROUGH;    // blaze::DynamicMatrix<T>
            case 4:                     // blaze::CustomMatrix<T>
                return make_caster<std::vector<std::vector<T>>>::cast(
                    std::forward<NodeData>(src).as_matrix(), policy, parent);

            default:
                break;
            }
            return handle();
        }

        using Type = phylanx::ir::node_data<T>;
        PYBIND11_TYPE_CASTER(Type, _("node_data[") + get_name<T>() + _("]"));
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class type_caster<phylanx::util::recursive_wrapper<T>>
    {
    private:
        using caster_t = make_caster<T>;
        caster_t subcaster;

        using subcaster_cast_op_type =
            typename caster_t::template cast_op_type<T>;

    public:
        bool load(handle src, bool convert)
        {
            return subcaster.load(src, convert);
        }

        static PHYLANX_PYBIND_DESCR PHYLANX_PYBIND_DESCR_NAME(
            _("recursive_wrapper[") + get_name<T>() + _("]"));

        static handle cast(phylanx::util::recursive_wrapper<T> const& src,
            return_value_policy policy, handle parent)
        {
            // It is definitely wrong to take ownership of this pointer,
            // so mask that rvp
            if (policy == return_value_policy::take_ownership ||
                policy == return_value_policy::automatic)
            {
                policy = return_value_policy::automatic_reference;
            }
            return caster_t::cast(&src.get(), policy, parent);
        }
        static handle cast(phylanx::util::recursive_wrapper<T> && src,
            return_value_policy policy, handle parent)
        {
            // It is definitely wrong to take ownership of this pointer,
            // so mask that rvp
            if (policy == return_value_policy::take_ownership ||
                policy == return_value_policy::automatic)
            {
                policy = return_value_policy::automatic_reference;
            }
            return caster_t::cast(&src.get(), policy, parent);
        }

        template <typename T_>
        using cast_op_type = phylanx::util::recursive_wrapper<T>;

        operator phylanx::util::recursive_wrapper<T>()
        {
            return phylanx::util::recursive_wrapper<T>(
                subcaster.operator subcaster_cast_op_type&());
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Derived, typename Variant>
    struct variant_caster_helper;

    template <typename Derived, template <typename...> class V, typename... Ts>
    struct variant_caster_helper<Derived, V<Ts...>>
    {
        PYBIND11_TYPE_CASTER(Derived,
            _("Union[") + detail::concat(make_caster<Ts>::name...) + _("]"));

        static_assert(sizeof...(Ts) > 0,
            "Variant must consist of at least one alternative.");

        template <typename U, typename... Us>
        bool load_alternative(handle src, bool convert, type_list<U, Us...>)
        {
            // deliberately start from the end of the type-list, this allows
            // to match a list before a matrix
            if (load_alternative(src, convert, type_list<Us...>{}))
                return true;

            auto caster = make_caster<U>();
            if (caster.load(src, convert))
            {
                value = cast_op<U>(caster);
                return true;
            }

            return false;
        }

        template <typename U, typename... Us>
        bool load_alternative(handle src, bool convert,
            type_list<phylanx::util::recursive_wrapper<U>, Us...>)
        {
            // deliberately start from the end of the type-list, this allows
            // to match a list before a matrix
            if (load_alternative(src, convert, type_list<Us...>{}))
                return true;

            auto caster = make_caster<U>();
            if (caster.load(src, convert))
            {
                value = cast_op<U>(caster);
                return true;
            }

            return false;
        }

        bool load_alternative(handle, bool, type_list<>) { return false; }

        bool load(handle src, bool convert)
        {
            // Do a first pass without conversions to improve constructor resolution.
            // E.g. `py::int_(1).cast<variant<double, int>>()` needs to fill the `int`
            // slot of the variant. Without two-pass loading `double` would be filled
            // because it appears first and a conversion is possible.
            if (convert && load_alternative(src, false, type_list<Ts...>{}))
                return true;
            return load_alternative(src, convert, type_list<Ts...>{});
        }

        template <typename Derived_>
        static handle cast(
            Derived_&& src, return_value_policy policy, handle parent)
        {
            return visit_helper<V>::call(variant_caster_visitor{policy, parent},
                                         std::forward<Derived_>(src));
        }
    };

    template <>
    class type_caster<phylanx::execution_tree::primitive_argument_type>
    {
    private:
        using caster_t = variant_caster_helper<
            phylanx::execution_tree::primitive_argument_type,
            phylanx::execution_tree::argument_value_type>;

    public:
        static PHYLANX_PYBIND_DESCR PHYLANX_PYBIND_DESCR_NAME(
            get_name<phylanx::execution_tree::primitive_argument_type>());

        bool load(handle src, bool convert)
        {
            return subcaster.load(src, convert);
        }

        template <typename T>
        static handle cast(T&& src, return_value_policy policy, handle parent)
        {
            return caster_t::cast(std::forward<T>(src), policy, parent);
        }

        template <typename T>
        using cast_op_type = typename caster_t::template cast_op_type<T>;

        operator phylanx::execution_tree::primitive_argument_type*()
        {
            return subcaster.operator
                phylanx::execution_tree::primitive_argument_type*();
        }
        operator phylanx::execution_tree::primitive_argument_type&()
        {
            return subcaster.operator
                phylanx::execution_tree::primitive_argument_type&();
        }
        operator phylanx::execution_tree::primitive_argument_type&&() &&
        {
            return std::move(subcaster).operator
                phylanx::execution_tree::primitive_argument_type &&();
        }

    private:
        caster_t subcaster;
    };
}}

#endif
