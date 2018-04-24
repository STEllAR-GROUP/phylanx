//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PYBIND11_TYPE_CASTERS_HPP)
#define PYBIND11_TYPE_CASTERS_HPP

#include <phylanx/phylanx.hpp>

#include <hpx/util/assert.hpp>
#include <hpx/util/tuple.hpp>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
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
    // Takes an input array and determines whether we can make it fit into the
    // Blaze type.
    using array_index_type =
        hpx::util::tuple<std::size_t, std::size_t, std::size_t, std::size_t>;

    template <typename Scalar>
    inline bool conformable(
        array const& a, std::size_t expected_dims, array_index_type& t)
    {
        const auto dims = a.ndim();
        if (dims != expected_dims)
            return false;

        if (dims == 2)
        {
            // Matrix type: require exact match (or dynamic)
            std::size_t np_rows = a.shape(0);
            std::size_t np_cols = a.shape(1);
            std::size_t np_rstride = a.strides(0) / sizeof(Scalar);
            std::size_t np_cstride = a.strides(1) / sizeof(Scalar);

            t = array_index_type{np_rows, np_cols, np_rstride, np_cstride};
            return true;
        }

        // Otherwise we're storing an n-vector.  Only one of the strides will
        // be used, but whichever is used, we want the (single) numpy stride
        // value.
        std::size_t const n = a.shape(0);
        std::size_t stride = a.strides(0) / sizeof(Scalar);

        t = array_index_type{n, 1, stride, 0};
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Casts a Blaze type to numpy array.  If given a base, the numpy array
    // references the src data, otherwise it'll make a copy.
    // writeable lets you turn off the writeable flag for the array.
    template <typename T, bool TF>
    handle blaze_array_cast(blaze::DynamicVector<T, TF> const& src,
        handle base = handle(), bool writeable = true)
    {
        array a{{src.size()}, {sizeof(T)}, src.data(), base};

        if (!writeable)
        {
            array_proxy(a.ptr())->flags &=
                ~detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return a.release();
    }

    template <typename T, bool AF, bool PF, bool TF>
    handle blaze_array_cast(blaze::CustomVector<T, AF, PF, TF> const& src,
        handle base = handle(), bool writeable = true)
    {
        array a{{src.size()}, {sizeof(T)}, src.data(), base};

        if (!writeable)
        {
            array_proxy(a.ptr())->flags &=
                ~detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return a.release();
    }

    template <typename T>
    handle blaze_array_cast(blaze::DynamicMatrix<T, false> const& src,
        handle base = handle(), bool writeable = true)
    {
        array a{
            {src.rows(), src.columns()},            // sizes
            {sizeof(T) * src.spacing(), sizeof(T)}, // strides
            src.data(), base};

        if (!writeable)
        {
            array_proxy(a.ptr())->flags &=
                ~detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return a.release();
    }

    template <typename T>
    handle blaze_array_cast(blaze::DynamicMatrix<T, true> const& src,
        handle base = handle(), bool writeable = true)
    {
        array a{
            {src.rows(), src.columns()},            // sizes
            {sizeof(T), sizeof(T) * src.spacing()}, // strides
            src.data(), base};

        if (!writeable)
        {
            array_proxy(a.ptr())->flags &=
                ~detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return a.release();
    }

    template <typename T, bool AF, bool PF>
    handle blaze_array_cast(blaze::CustomMatrix<T, AF, PF, false> const& src,
        handle base = handle(), bool writeable = true)
    {
        array a{
            {src.rows(), src.columns()},            // sizes
            {sizeof(T) * src.spacing(), sizeof(T)}, // strides
            src.data(), base};

        if (!writeable)
        {
            array_proxy(a.ptr())->flags &=
                ~detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return a.release();
    }

    template <typename T, bool AF, bool PF>
    handle blaze_array_cast(blaze::CustomMatrix<T, AF, PF, true> const& src,
        handle base = handle(), bool writeable = true)
    {
        array a{
            {src.rows(), src.columns()},            // sizes
            {sizeof(T), sizeof(T) * src.spacing()}, // strides
            src.data(), base};

        if (!writeable)
        {
            array_proxy(a.ptr())->flags &=
                ~detail::npy_api::NPY_ARRAY_WRITEABLE_;
        }
        return a.release();
    }

    // Takes an lvalue ref to some Blaze type and a (python) base object,
    // creating a numpy array that references the Blaze object's data with
    // `base` as the python-registered base class (if omitted, the base will
    // be set to None, and lifetime management is up to the caller). The numpy
    // array is non-writeable if the given type is const.
    template <typename Type>
    handle blaze_ref_array(Type& src, handle parent = none())
    {
        // none here is to get past array's should-we-copy detection, which
        // currently always copies when there is no base.  Setting the base to
        // None should be harmless.
        return blaze_array_cast(src, parent, !std::is_const<Type>::value);
    }

    // Takes a pointer to some dense, plain Blaze type, builds a capsule
    // around it, then returns a numpy array that references the encapsulated
    // data with a python-side reference to the capsule to tie its destruction
    // to that of any dependent python objects. Const-ness is determined by
    // whether or not the Type of the pointer given is const.
    template <typename Type>
    handle blaze_encapsulate(Type *src)
    {
        capsule base(src, [](void* o) { delete static_cast<Type*>(o); });
        return blaze_ref_array(*src, base);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    struct casted_type
    {
        using type = T;
    };

    template <>
    struct casted_type<std::uint8_t>
    {
        using type = bool;
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    class type_caster<phylanx::ir::node_data<T>>
    {
        bool load0d(handle src, bool convert)
        {
            auto caster = make_caster<typename casted_type<T>::type>();
            if (caster.load(src, convert))
            {
                value = cast_op<typename casted_type<T>::type>(caster);
                return true;
            }
            return false;
        }

        bool load1d(handle src, bool convert)
        {
            if (!convert &&
                !isinstance<array_t<typename casted_type<T>::type>>(src))
            {
                return false;
            }

            // Coerce into an array, but don't do type conversion yet; the copy
            // below handles it.
            auto buf = array::ensure(src);
            if (!buf) return false;

            auto dims = buf.ndim();
            if (dims != 1) return false;

            array_index_type t;
            bool fits = conformable<typename casted_type<T>::type>(buf, 1, t);
            if (!fits) return false;

            // Allocate the new type, then build a numpy reference into it
            value = blaze::DynamicVector<T>(hpx::util::get<0>(t));

            auto v = value.vector();
            auto ref = reinterpret_steal<array>(blaze_ref_array(v));

            if (dims == 1) ref = ref.squeeze();
            else if (ref.ndim() == 1) buf = buf.squeeze();

            int result =
                detail::npy_api::get().PyArray_CopyInto_(ref.ptr(), buf.ptr());
            if (result < 0)
            {
                // Copy failed!
                PyErr_Clear();
                return false;
            }
            return true;
        }

        bool load2d(handle src, bool convert)
        {
            if (!convert &&
                !isinstance<array_t<typename casted_type<T>::type>>(src))
            {
                return false;
            }

            // Coerce into an array, but don't do type conversion yet; the copy
            // below handles it.
            auto buf = array::ensure(src);
            if (!buf) return false;

            auto dims = buf.ndim();
            if (dims != 2) return false;

            array_index_type t;
            bool fits = conformable<typename casted_type<T>::type>(buf, 2, t);
            if (!fits) return false;

            // Allocate the new type, then build a numpy reference into it
            value = blaze::DynamicMatrix<T>(hpx::util::get<0>(t),
                hpx::util::get<1>(t), hpx::util::get<3>(t));

            auto m = value.matrix();
            auto ref = reinterpret_steal<array>(blaze_ref_array(m));

            if (dims == 1) ref = ref.squeeze();
            else if (ref.ndim() == 1) buf = buf.squeeze();

            int result =
                detail::npy_api::get().PyArray_CopyInto_(ref.ptr(), buf.ptr());
            if (result < 0)
            {
                // Copy failed!
                PyErr_Clear();
                return false;
            }
            return true;
        }

        template <typename Type>
        static handle cast_impl_automatic(Type* src)
        {
            switch (src->index())
            {
            case 1:     // blaze::DynamicVector<T>
                return blaze_encapsulate(&(src->vector_non_ref()));

            case 2:     // blaze::DynamicMatrix<T>
                return blaze_encapsulate(&(src->matrix_non_ref()));

            // custom types require a copy (done by vector_copy/matrix_copy)
            case 3:     // blaze::CustomVector<T>
                return blaze_encapsulate(new blaze::DynamicVector<T>(
                    src->vector_copy()));

            case 4:     // blaze::CustomMatrix<T>
                return blaze_encapsulate(new blaze::DynamicMatrix<T>(
                    src->matrix_copy()));

            default:
                throw cast_error("cast_impl_automatic: "
                    "unexpected node_data type: should not happen!");
            }
            return handle();
        }

        template <typename Type>
        static handle cast_impl_move(Type* src)
        {
            switch (src->index())
            {
            case 1:     // blaze::DynamicVector<T>
                return blaze_encapsulate(new blaze::DynamicVector<T>(
                    std::move(src->vector_non_ref())));

            case 2:     // blaze::DynamicMatrix<T>
                return blaze_encapsulate(new blaze::DynamicMatrix<T>(
                    std::move(src->matrix_non_ref())));

            // custom types require a copy (done by vector_copy/matrix_copy)
            case 3:     // blaze::CustomVector<T>
                return blaze_encapsulate(new blaze::DynamicVector<T>(
                    src->vector_copy()));

            case 4:     // blaze::CustomMatrix<T>
                return blaze_encapsulate(new blaze::DynamicMatrix<T>(
                    src->matrix_copy()));

            default:
                throw cast_error("cast_impl_move: "
                    "unexpected node_data type: should not happen!");
            }
            return handle();
        }

        template <typename Type>
        static handle cast_impl_copy(Type* src)
        {
            switch (src->index())
            {
            case 1:     // blaze::DynamicVector<T>
                return blaze_array_cast(src->vector_non_ref());

            case 2:     // blaze::DynamicMatrix<T>
                return blaze_array_cast(src->matrix_non_ref());

            case 3:     // blaze::CustomVector<T>
                return blaze_encapsulate(new blaze::DynamicVector<T>(
                    src->vector_copy()));

            case 4:     // blaze::CustomMatrix<T>
                return blaze_encapsulate(new blaze::DynamicMatrix<T>(
                    src->matrix_copy()));

            default:
                throw cast_error("cast_impl_copy: "
                    "unexpected node_data type: should not happen!");
            }
            return handle();
        }

        template <typename Type>
        static handle cast_impl_automatic_reference(Type* src)
        {
            switch (src->index())
            {
            case 1:     // blaze::DynamicVector<T>
                return blaze_ref_array(src->vector_non_ref());

            case 2:     // blaze::DynamicMatrix<T>
                return blaze_ref_array(src->matrix_non_ref());

            // custom types require a copy (done by vector_copy/matrix_copy)
            case 3:     // blaze::CustomVector<T>
                return blaze_encapsulate(new blaze::DynamicVector<T>(
                    src->vector_copy()));

            case 4:     // blaze::CustomMatrix<T>
                return blaze_encapsulate(new blaze::DynamicMatrix<T>(
                    src->matrix_copy()));

            default:
                throw cast_error("cast_impl_automatic_reference: "
                    "unexpected node_data type: should not happen!");
            }
            return handle();
        }

        template <typename Type>
        static handle cast_impl_reference_internal(Type* src, handle parent)
        {
            switch (src->index())
            {
            case 1:     // blaze::DynamicVector<T>
                return blaze_ref_array(src->vector_non_ref(), parent);

            case 2:     // blaze::DynamicMatrix<T>
                return blaze_ref_array(src->matrix_non_ref(), parent);

            case 3: HPX_FALLTHROUGH;    // blaze::CustomVector<T>
            case 4: HPX_FALLTHROUGH;    // blaze::CustomMatrix<T>
            default:
                throw cast_error("cast_impl_reference_internal: "
                    "unexpected node_data type: should not happen!");
            }
            return handle();
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Type>
        static handle cast_impl(
            Type* src, return_value_policy policy, handle parent)
        {
            if (0 == src->index())      // T
            {
                return make_caster<typename casted_type<T>::type>::cast(
                    src->scalar(), policy, parent);
            }

            switch (policy)
            {
            case return_value_policy::take_ownership:
                HPX_FALLTHROUGH;
            case return_value_policy::automatic:
                return cast_impl_automatic(src);

            case return_value_policy::move:
                return cast_impl_move(src);

            case return_value_policy::copy:
                return cast_impl_copy(src);

            case return_value_policy::reference:
                HPX_FALLTHROUGH;
            case return_value_policy::automatic_reference:
                return cast_impl_automatic_reference(src);

            case return_value_policy::reference_internal:
                return cast_impl_reference_internal(src, parent);

            default:
                throw cast_error(
                    "type_caster<phylanx::ir::node_data<T>>::cast_impl: "
                    "unhandled return_value_policy: should not happen!");
            };
        }

    public:
        bool load(handle src, bool convert)
        {
            return load0d(src, convert) || load1d(src, convert) ||
                load2d(src, convert);
        }

        // Normal returned non-reference, non-const value:
        static handle cast(phylanx::ir::node_data<T>&& src,
            return_value_policy /* policy */, handle parent)
        {
            return cast_impl(&src, return_value_policy::move, parent);
        }

        // If you return a non-reference const, we mark the numpy array as
        // read only
        static handle cast(phylanx::ir::node_data<T> const&& src,
            return_value_policy /* policy */, handle parent)
        {
            return cast_impl(&src, return_value_policy::move, parent);
        }

        // lvalue reference return; default (automatic) becomes copy
        static handle cast(phylanx::ir::node_data<T>& src,
            return_value_policy policy, handle parent)
        {
            if (policy == return_value_policy::automatic ||
                policy == return_value_policy::automatic_reference)
            {
                policy = return_value_policy::copy;
            }
            return cast_impl(&src, policy, parent);
        }

        // const lvalue reference return; default (automatic) becomes copy
        static handle cast(phylanx::ir::node_data<T> const& src,
            return_value_policy policy, handle parent)
        {
            if (policy == return_value_policy::automatic ||
                policy == return_value_policy::automatic_reference)
            {
                policy = return_value_policy::copy;
            }
            return cast(&src, policy, parent);
        }

        // non-const pointer return
        static handle cast(phylanx::ir::node_data<T>* src,
            return_value_policy policy, handle parent)
        {
            return cast_impl(src, policy, parent);
        }

        // const pointer return
        static handle cast(phylanx::ir::node_data<T> const* src,
            return_value_policy policy, handle parent)
        {
            return cast_impl(src, policy, parent);
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
            _("phylanx::execution_tree::primitive_argument_type::variant"));

        static_assert(sizeof...(Ts) > 0,
            "Variant must consist of at least one alternative.");

        template <typename U, typename... Us>
        inline bool load_alternative(
            handle src, bool convert, type_list<U, Us...>);

        template <typename U, typename... Us>
        inline bool load_alternative(handle src, bool convert,
            type_list<phylanx::util::recursive_wrapper<U>, Us...>);

        bool load_alternative(handle, bool, type_list<>) { return false; }

        inline bool load(handle src, bool convert);

        template <typename Derived_>
        inline static handle cast(
            Derived_&& src, return_value_policy policy, handle parent);
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

    ///////////////////////////////////////////////////////////////////////////
    template <>
    class type_caster<phylanx::ir::range>
    {
    private:
        using list_type =
            std::vector<phylanx::execution_tree::primitive_argument_type>;

        using list_caster_type = make_caster<list_type>;
        list_caster_type subcaster;

        ///////////////////////////////////////////////////////////////////////
        bool load_list(handle src, bool convert)
        {
            if (subcaster.load(src, convert))
            {
                value = std::move(std::move(subcaster).operator list_type &&());
                return true;
            }
            return false;
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename Type>
        static handle cast_impl(
            Type* src, return_value_policy policy, handle parent)
        {
            switch(src->index())
            {
            case 1:                     // wrapped_args_type
                return list_caster_type::cast(src->args(), policy, parent);

            case 2:                     // arg_pair_type
                return list_caster_type::cast(src->copy(), policy, parent);

            case 0: HPX_FALLTHROUGH;    // int_range_type
            default:
                throw cast_error(
                    "cast_impl: unexpected range type: should not happen!");
            }
            return handle();
        }

    public:
        bool load(handle src, bool convert)
        {
            return load_list(src, convert);
        }

        // Normal returned non-reference, non-const value:
        static handle cast(phylanx::ir::range&& src,
            return_value_policy /* policy */, handle parent)
        {
            return cast_impl(&src, return_value_policy::move, parent);
        }

        // If you return a non-reference const, we mark the list as read only
        static handle cast(phylanx::ir::range const&& src,
            return_value_policy /* policy */, handle parent)
        {
            return cast_impl(&src, return_value_policy::move, parent);
        }

        // lvalue reference return; default (automatic) becomes copy
        static handle cast(phylanx::ir::range& src,
            return_value_policy policy, handle parent)
        {
            if (policy == return_value_policy::automatic ||
                policy == return_value_policy::automatic_reference)
            {
                policy = return_value_policy::copy;
            }
            return cast_impl(&src, policy, parent);
        }

        // const lvalue reference return; default (automatic) becomes copy
        static handle cast(phylanx::ir::range const& src,
            return_value_policy policy, handle parent)
        {
            if (policy == return_value_policy::automatic ||
                policy == return_value_policy::automatic_reference)
            {
                policy = return_value_policy::copy;
            }
            return cast(&src, policy, parent);
        }

        // non-const pointer return
        static handle cast(phylanx::ir::range* src,
            return_value_policy policy, handle parent)
        {
            return cast_impl(src, policy, parent);
        }

        // const pointer return
        static handle cast(phylanx::ir::range const* src,
            return_value_policy policy, handle parent)
        {
            return cast_impl(src, policy, parent);
        }

        using Type = phylanx::ir::range;
        PYBIND11_TYPE_CASTER(Type, _("range"));
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Derived, template <typename...> class V, typename... Ts>
    template <typename U, typename... Us>
    bool variant_caster_helper<Derived, V<Ts...>>::load_alternative(
        handle src, bool convert, type_list<U, Us...>)
    {
        auto caster = make_caster<U>();
        if (caster.load(src, convert))
        {
            value = cast_op<U>(caster);
            return true;
        }
        return load_alternative(src, convert, type_list<Us...>{});
    }

    template <typename Derived, template <typename...> class V, typename... Ts>
    template <typename U, typename... Us>
    bool variant_caster_helper<Derived, V<Ts...>>::load_alternative(handle src,
        bool convert, type_list<phylanx::util::recursive_wrapper<U>, Us...>)
    {
        auto caster = make_caster<U>();
        if (caster.load(src, convert))
        {
            value = cast_op<U>(caster);
            return true;
        }
        return load_alternative(src, convert, type_list<Us...>{});
    }

    template <typename Derived, template <typename...> class V, typename... Ts>
    template <typename Derived_>
    handle variant_caster_helper<Derived, V<Ts...>>::cast(
        Derived_&& src, return_value_policy policy, handle parent)
    {
        return visit_helper<V>::call(variant_caster_visitor{policy, parent},
                                        std::forward<Derived_>(src));
    }

    template <typename Derived, template <typename...> class V, typename... Ts>
    bool variant_caster_helper<Derived, V<Ts...>>::load(
        handle src, bool convert)
    {
        // Do a first pass without conversions to improve constructor resolution.
        // E.g. `py::int_(1).cast<variant<double, int>>()` needs to fill the `int`
        // slot of the variant. Without two-pass loading `double` would be filled
        // because it appears first and a conversion is possible.
        if (convert && load_alternative(src, false, type_list<Ts...>{}))
            return true;
        return load_alternative(src, convert, type_list<Ts...>{});
    }
}}

#endif
