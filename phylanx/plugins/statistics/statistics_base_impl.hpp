// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVE_STATISTICS_IMPL_DEC_24_2018_0342PM)
#define PHYLANX_PRIMITIVE_STATISTICS_IMPL_DEC_24_2018_0342PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/assertion.hpp>
#include <hpx/datastructures/optional.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    statistics<Op, Derived>::statistics(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics0d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<Init> const& initial) const
    {
        if (axis)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statistics0d",
                generate_error_message("the statistics_operation primitive "
                    "requires that no axis is specified for scalar values."));
        }

        Op<T> op{name_, codename_};

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        return primitive_argument_type{
            op(extract_scalar_data<T>(std::move(arg), name_, codename_),
                initial_value)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics1d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<Init> const& initial) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statistics1d",
                generate_error_message(
                    "the statistics_operation primitive requires operand axis "
                    "to be either 0 or -1 for vectors."));
        }


        Op<T> op{name_, codename_};

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        auto v = arg.vector();
        T result = op(v, initial_value);

        if (keepdims)
        {
            return primitive_argument_type{blaze::DynamicVector<T>(
                1, op.finalize(result, v.size()))};
        }

        return primitive_argument_type{op.finalize(result, v.size())};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics2d_flat(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto m = arg.matrix();

        Op<T> op{name_, codename_};
        std::size_t size = 0;

        Init result = Op<T>::initial();
        if (initial)
        {
            result = *initial;
        }

        for (std::size_t i = 0; i != m.rows(); ++i)
        {
            auto row = blaze::row(m, i);
            result = op(row, result);
            size += row.size();
        }

        if (keepdims)
        {
            return primitive_argument_type{blaze::DynamicMatrix<T>(
                1, 1, op.finalize(result, size))};
        }

        return primitive_argument_type{op.finalize(result, size)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics2d_axis0(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto m = arg.matrix();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicMatrix<T> result(1, m.columns());
            for (std::size_t i = 0; i != m.columns(); ++i)
            {
                Op<T> op{name_, codename_};
                auto col = blaze::column(m, i);
                result(0, i) = op.finalize(op(col, initial_value), col.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(m.columns());
        for (std::size_t i = 0; i != m.columns(); ++i)
        {
            Op<T> op{name_, codename_};
            auto col = blaze::column(m, i);
            result[i] = op.finalize(op(col, initial_value), col.size());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics2d_axis1(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto m = arg.matrix();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicMatrix<T> result(m.rows(), 1);
            for (std::size_t i = 0; i != m.rows(); ++i)
            {
                Op<T> op{name_, codename_};
                auto row = blaze::row(m, i);
                result(i, 0) = op.finalize(op(row, initial_value), row.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(m.rows());
        for (std::size_t i = 0; i != m.rows(); ++i)
        {
            Op<T> op{name_, codename_};
            auto row = blaze::row(m, i);
            result[i] = op.finalize(op(row, initial_value), row.size());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics2d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<Init> const& initial) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -2: HPX_FALLTHROUGH;
            case 0:
                return statistics2d_axis0(std::move(arg), keepdims, initial);

            case -1: HPX_FALLTHROUGH;
            case 1:
                return statistics2d_axis1(std::move(arg), keepdims, initial);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::statistics2d",
                    generate_error_message(
                        "the statistics_operation primitive requires operand "
                        "axis to be between -2 and 1 for matrices."));
            }
        }
        return statistics2d_flat(std::move(arg), keepdims, initial);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_flat(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Op<T> op{name_, codename_};

        std::size_t size = 0;

        Init result = Op<T>::initial();
        if (initial)
        {
            result = *initial;
        }

        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto page = blaze::pageslice(t, k);
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto row = blaze::row(page, i);
                result = op(row, result);
                size += row.size();
            }
        }

        if (keepdims)
        {
            return primitive_argument_type{blaze::DynamicTensor<T>(
                1, 1, 1, op.finalize(result, size))};
        }

        return primitive_argument_type{op.finalize(result, size)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis0(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicTensor<T> result(1, t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    Op<T> op{name_, codename_};
                    auto row = blaze::row(slice, j);
                    result(0, i, j) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(t.rows(), t.columns());
        for (std::size_t i = 0; i != t.rows(); ++i)
        {
            auto slice = blaze::rowslice(t, i);
            for (std::size_t j = 0; j != t.columns(); ++j)
            {
                Op<T> op{name_, codename_};
                auto row = blaze::row(slice, j);
                result(i, j) = op.finalize(op(row, initial_value), row.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis1(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicTensor<T> result(t.pages(), 1, t.columns());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    Op<T> op{name_, codename_};
                    auto col = blaze::column(slice, j);
                    result(k, 0, j) =
                        op.finalize(op(col, initial_value), col.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(t.pages(), t.columns());
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            for (std::size_t j = 0; j != t.columns(); ++j)
            {
                Op<T> op{name_, codename_};
                auto col = blaze::column(slice, j);
                result(k, j) = op.finalize(op(col, initial_value), col.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis2(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), 1);
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t i = 0; i != t.rows(); ++i)
                {
                    Op<T> op{name_, codename_};
                    auto row = blaze::row(slice, i);
                    result(k, i, 0) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(t.pages(), t.rows());
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                Op<T> op{name_, codename_};
                auto row = blaze::row(slice, i);
                result(k, i) = op.finalize(op(row, initial_value), row.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<Init> const& initial) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -3: HPX_FALLTHROUGH;
            case 0:
                return statistics3d_axis0(std::move(arg), keepdims, initial);

            case -2: HPX_FALLTHROUGH;
            case 1:
                return statistics3d_axis1(std::move(arg), keepdims, initial);

            case -1: HPX_FALLTHROUGH;
            case 2:
                return statistics3d_axis2(std::move(arg), keepdims, initial);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::statistics3d",
                    generate_error_message(
                        "the statistics_operation primitive requires operand "
                        "axis to be between -3 and 2 for tensors."));
            }
        }
        return statistics3d_flat(std::move(arg), keepdims, initial);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_columnslice(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicTensor<T> result(1, 1, t.columns());
            for (std::size_t k = 0; k != t.columns(); ++k)
            {
                Op<T> op{name_, codename_};
                auto slice = blaze::ravel(blaze::columnslice(t, k));
                result(0, 0, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(t.columns());
        for (std::size_t k = 0; k != t.columns(); ++k)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(blaze::columnslice(t, k));
            result[k] = op.finalize(op(slice, initial_value), slice.size());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_rowslice(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicTensor<T> result(1, t.rows(), 1);
            for (std::size_t k = 0; k != t.rows(); ++k)
            {
                Op<T> op{name_, codename_};
                auto slice = blaze::ravel(blaze::rowslice(t, k));
                result(0, k, 0) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(t.rows());
        for (std::size_t k = 0; k != t.rows(); ++k)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(blaze::rowslice(t, k));
            result[k] = op.finalize(op(slice, initial_value), slice.size());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics3d_pageslice(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto t = arg.tensor();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicTensor<T> result(t.pages(), 1, 1);
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                Op<T> op{name_, codename_};
                auto slice = blaze::ravel(blaze::pageslice(t, k));
                result(k, 0, 0) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(t.pages());
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(blaze::pageslice(t, k));
            result[k] = op.finalize(op(slice, initial_value), slice.size());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_slice(
        arg_type<T>&& arg, std::int64_t axis0, std::int64_t axis1,
        bool keepdims, primitive_argument_type&& initial) const
    {
        using initial_type = typename Op<T>::result_type;

        hpx::util::optional<initial_type> initial_value;
        if (valid(initial))
        {
            initial_value = extract_scalar_data<initial_type>(
                std::move(initial), name_, codename_);
        }

        if (axis0 == 0)
        {
            if (axis1 == 1)
            {
                return statistics3d_columnslice(
                    std::move(arg), keepdims, initial_value);
            }

            // axis0 == 0 && axis1 == 2
            return statistics3d_rowslice(
                std::move(arg), keepdims, initial_value);
        }

        // axis0 == 1 && axis1 == 2
        return statistics3d_pageslice(std::move(arg), keepdims, initial_value);
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statistics3d_slice(
        primitive_argument_type&& arg,
        std::int64_t axis0, std::int64_t axis1, bool keepdims,
        primitive_argument_type&& initial) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        if (axis0 > axis1)
        {
            std::swap(axis0, axis1);
        }

        switch (t)
        {
        case node_data_type_bool:
            return statistics3d_slice(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                axis0, axis1, keepdims, std::move(initial));

        case node_data_type_int64:
            return statistics3d_slice(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                axis0, axis1, keepdims, std::move(initial));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return statistics3d_slice(
                extract_numeric_value(std::move(arg), name_, codename_), axis0,
                axis1, keepdims, std::move(initial));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statistics3d_slice",
            generate_error_message(
                "the statistics primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice01(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(1, 1, q.rows(), q.columns());
            for (std::size_t l = 0; l != q.rows(); ++l)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l);
                for (std::size_t k = 0; k != q.columns(); ++k)
                {
                    Op<T> op{name_, codename_};
                    auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                    result(0, 0, l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(q.rows(), q.columns());
        for (std::size_t l = 0; l != q.rows(); ++l)
        {
            auto tensor = blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l);
            for (std::size_t k = 0; k != q.columns(); ++k)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                result(l, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice02(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(1, q.pages(), 1, q.columns());
            for (std::size_t l = 0; l != q.pages(); ++l)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
                for (std::size_t k = 0; k != q.columns(); ++k)
                {
                    Op<T> op{name_, codename_};
                    auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                    result(0, 0, l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(q.pages(), q.columns());
        for (std::size_t l = 0; l != q.pages(); ++l)
        {
            auto tensor = blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
            for (std::size_t k = 0; k != q.columns(); ++k)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                result(l, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice03(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(1, q.pages(), q.rows(), 1);
            for (std::size_t l = 0; l != q.pages(); ++l)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
                for (std::size_t k = 0; k != q.rows(); ++k)
                {
                    Op<T> op{name_, codename_};
                    auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                    result(0, l, k, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(q.pages(), q.rows());
        for (std::size_t l = 0; l != q.pages(); ++l)
        {
            auto tensor = blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
            for (std::size_t k = 0; k != q.rows(); ++k)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                result(l, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice12(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(q.quats(), 1, 1, q.columns());
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                auto tensor = blaze::quatslice(q, l);
                for (std::size_t k = 0; k != q.columns(); ++k)
                {
                    Op<T> op{name_, codename_};
                    auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                    result(l, 0, 0, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(q.quats(), q.columns());
        for (std::size_t l = 0; l != q.quats(); ++l)
        {
            auto tensor = blaze::quatslice(q, l);
            for (std::size_t k = 0; k != q.columns(); ++k)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                result(l, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice13(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(q.quats(), 1, q.rows(), 1);
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                auto tensor = blaze::quatslice(q, l);
                for (std::size_t k = 0; k != q.rows(); ++k)
                {
                    Op<T> op{name_, codename_};
                    auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                    result(l, 0, k, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(q.quats(), q.rows());
        for (std::size_t l = 0; l != q.quats(); ++l)
        {
            auto tensor = blaze::quatslice(q, l);
            for (std::size_t k = 0; k != q.rows(); ++k)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                result(l, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice23(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(q.quats(), q.pages(), 1, 1);
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                auto tensor = blaze::quatslice(q, l);
                for (std::size_t k = 0; k != q.pages(); ++k)
                {
                    Op<T> op{name_, codename_};
                    auto slice = blaze::ravel(blaze::pageslice(tensor, k));
                    result(l, k, 0, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(q.quats(), q.pages());
        for (std::size_t l = 0; l != q.quats(); ++l)
        {
            auto tensor = blaze::quatslice(q, l);
            for (std::size_t k = 0; k != q.pages(); ++k)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::pageslice(tensor, k));
                result(l, k) =
                    op.finalize(op(slice, initial_value), slice.size());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice(
        arg_type<T>&& arg, std::int64_t axis0, std::int64_t axis1,
        bool keepdims, primitive_argument_type&& initial) const
    {
        using initial_type = typename Op<T>::result_type;

        hpx::util::optional<initial_type> initial_value;
        if (valid(initial))
        {
            initial_value = extract_scalar_data<initial_type>(
                std::move(initial), name_, codename_);
        }

        if (axis0 == 0)
        {
            if (axis1 == 1)
            {
                return statistics4d_slice01(
                    std::move(arg), keepdims, initial_value);
            }

            else if (axis1 == 2)
            {
                return statistics4d_slice02(
                    std::move(arg), keepdims, initial_value);
            }
            // axis0 == 0 && axis1 == 3
            return statistics4d_slice03(
                std::move(arg), keepdims, initial_value);
        }

        else if (axis0 == 1)
        {
            if (axis1 == 2)
            {
                return statistics4d_slice12(
                    std::move(arg), keepdims, initial_value);
            }
            // axis0 == 1 && axis1 == 3
            return statistics4d_slice13(
                std::move(arg), keepdims, initial_value);
        }

        // axis0 == 2 && axis1 == 3
        return statistics4d_slice23(std::move(arg), keepdims, initial_value);
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statistics4d_slice(
        primitive_argument_type&& arg,
        std::int64_t axis0, std::int64_t axis1, bool keepdims,
        primitive_argument_type&& initial) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        if (axis0 > axis1)
        {
            std::swap(axis0, axis1);
        }

        switch (t)
        {
        case node_data_type_bool:
            return statistics4d_slice(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                axis0, axis1, keepdims, std::move(initial));

        case node_data_type_int64:
            return statistics4d_slice(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                axis0, axis1, keepdims, std::move(initial));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return statistics4d_slice(
                extract_numeric_value(std::move(arg), name_, codename_), axis0,
                axis1, keepdims, std::move(initial));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statistics4d_slice",
            generate_error_message(
                "the statistics primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_tensor012(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(1, 1, 1, q.columns());
            for (std::size_t l = 0; l != q.columns(); ++l)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(
                    blaze::quatslice(blaze::trans(q, {3, 0, 1, 2}), l));
                result(0, 0, 0, l) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(q.columns());
        for (std::size_t l = 0; l != q.columns(); ++l)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(
                blaze::quatslice(blaze::trans(q, {3, 0, 1, 2}), l));
            result[l] = op.finalize(op(slice, initial_value), slice.size());
        }
        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_tensor013(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(1, 1, q.rows(), 1);
            for (std::size_t l = 0; l != q.rows(); ++l)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(
                    blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l));
                result(0, 0, l, 0) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(q.rows());
        for (std::size_t l = 0; l != q.rows(); ++l)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(
                blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l));
            result[l] = op.finalize(op(slice, initial_value), slice.size());
        }
        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_tensor023(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(1, q.pages(), 1, 1);
            for (std::size_t l = 0; l != q.pages(); ++l)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l));
                result(0, l, 0, 0) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(q.pages());
        for (std::size_t l = 0; l != q.pages(); ++l)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(
                blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l));
            result[l] = op.finalize(op(slice, initial_value), slice.size());
        }
        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_tensor123(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(q.quats(), 1, 1, 1);
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                Op<T> op{ name_, codename_ };
                auto slice = blaze::ravel(blaze::quatslice(q, l));
                result(l, 0, 0, 0) =
                    op.finalize(op(slice, initial_value), slice.size());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(q.quats());
        for (std::size_t l = 0; l != q.quats(); ++l)
        {
            Op<T> op{name_, codename_};
            auto slice = blaze::ravel(blaze::quatslice(q, l));
            result[l] = op.finalize(op(slice, initial_value), slice.size());
        }
        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics4d_tensor(
        arg_type<T>&& arg, std::int64_t axis0, std::int64_t axis1,
        std::int64_t axis2, bool keepdims,
        primitive_argument_type&& initial) const
    {
        using initial_type = typename Op<T>::result_type;

        hpx::util::optional<initial_type> initial_value;
        if (valid(initial))
        {
            initial_value = extract_scalar_data<initial_type>(
                std::move(initial), name_, codename_);
        }

        // axes are unique, so each of 4 combinations has a different sum
        switch (axis0 + axis1 + axis2)
        {
        case 3:
            return statistics4d_tensor012(
                std::move(arg), keepdims, initial_value);

        case 4:
            return statistics4d_tensor013(
                std::move(arg), keepdims, initial_value);

        case 5:
            return statistics4d_tensor023(
                std::move(arg), keepdims, initial_value);

        case 6:
            return statistics4d_tensor123(
                std::move(arg), keepdims, initial_value);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statistics4d_tensor",
            generate_error_message("invalid combination of axes"));

    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statistics4d_tensor(
        primitive_argument_type&& arg,
        std::int64_t axis0, std::int64_t axis1,std::int64_t axis2, bool keepdims,
        primitive_argument_type&& initial) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return statistics4d_tensor(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                axis0, axis1, axis2, keepdims, std::move(initial));

        case node_data_type_int64:
            return statistics4d_tensor(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                axis0, axis1, axis2, keepdims, std::move(initial));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return statistics4d_tensor(
                extract_numeric_value(std::move(arg), name_, codename_), axis0,
                axis1, axis2, keepdims, std::move(initial));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statistics4d_tensor",
            generate_error_message(
                "the statistics primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_flat(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Op<T> op{name_, codename_};

        std::size_t size = 0;

        Init result = Op<T>::initial();
        if (initial)
        {
            result = *initial;
        }

        for (std::size_t l = 0; l != q.quats(); ++l)
        {
            auto quat = blaze::quatslice(q, l);
            for (std::size_t k = 0; k != q.pages(); ++k)
            {
                auto page = blaze::pageslice(quat, k);
                for (std::size_t i = 0; i != q.rows(); ++i)
                {
                    auto row = blaze::row(page, i);
                    result = op(row, result);
                    size += row.size();
                }
            }
        }

        if (keepdims)
        {
            return primitive_argument_type{
                blaze::DynamicArray<4UL, T>(blaze::init_from_value,
                    op.finalize(result, size), 1, 1, 1, 1)};
        }

        return primitive_argument_type{op.finalize(result, size)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_axis0(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(
                1, q.pages(), q.rows(), q.columns());
            for (std::size_t k = 0; k != q.pages(); ++k)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), k);
                for (std::size_t i = 0; i != q.rows(); ++i)
                {
                    auto slice = blaze::rowslice(tensor, i);
                    for (std::size_t j = 0; j != q.columns(); ++j)
                    {
                        Op<T> op{ name_, codename_ };
                        auto row = blaze::row(slice, j);
                        result(0, k, i, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicTensor<T> result(q.pages(), q.rows(), q.columns());
        for (std::size_t k = 0; k != q.pages(); ++k)
        {
            auto tensor = blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), k);
            for (std::size_t i = 0; i != q.rows(); ++i)
            {
                auto slice = blaze::rowslice(tensor, i);
                for (std::size_t j = 0; j != q.columns(); ++j)
                {
                    Op<T> op{ name_, codename_ };
                    auto row = blaze::row(slice, j);
                    result(k, i, j) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_axis1(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(
                q.quats(), 1, q.rows(), q.columns());
            for (std::size_t k = 0; k != q.quats(); ++k)
            {
                auto tensor = blaze::quatslice(q, k);
                for (std::size_t i = 0; i != q.rows(); ++i)
                {
                    auto slice = blaze::rowslice(tensor, i);
                    for (std::size_t j = 0; j != q.columns(); ++j)
                    {
                        Op<T> op{ name_, codename_ };
                        auto row = blaze::row(slice, j);
                        result(k, 0, i, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicTensor<T> result(q.quats(), q.rows(), q.columns());
        for (std::size_t k = 0; k != q.quats(); ++k)
        {
            auto tensor = blaze::quatslice(q, k);
            for (std::size_t i = 0; i != q.rows(); ++i)
            {
                auto slice = blaze::rowslice(tensor, i);
                for (std::size_t j = 0; j != q.columns(); ++j)
                {
                    Op<T> op{ name_, codename_ };
                    auto row = blaze::row(slice, j);
                    result(k, i, j) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_axis2(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(
                q.quats(), q.pages(), 1, q.columns());
            for (std::size_t k = 0; k != q.quats(); ++k)
            {
                auto tensor = blaze::quatslice(q, k);
                for (std::size_t i = 0; i != q.pages(); ++i)
                {
                    auto slice = blaze::pageslice(tensor, i);
                    for (std::size_t j = 0; j != q.columns(); ++j)
                    {
                        Op<T> op{ name_, codename_ };
                        auto col = blaze::column(slice, j);
                        result(k, i, 0, j) =
                            op.finalize(op(col, initial_value), col.size());
                    }
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicTensor<T> result(q.quats(), q.pages(), q.columns());
        for (std::size_t k = 0; k != q.quats(); ++k)
        {
            auto tensor = blaze::quatslice(q, k);
            for (std::size_t i = 0; i != q.pages(); ++i)
            {
                auto slice = blaze::pageslice(tensor, i);
                for (std::size_t j = 0; j != q.columns(); ++j)
                {
                    Op<T> op{ name_, codename_ };
                    auto col = blaze::column(slice, j);
                    result(k, i, j) =
                        op.finalize(op(col, initial_value), col.size());
                }
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d_axis3(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<Init> const& initial) const
    {
        auto q = arg.quatern();

        Init initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        if (keepdims)
        {
            blaze::DynamicArray<4UL, T> result(
                q.quats(), q.pages(), q.rows(), 1);
            for (std::size_t k = 0; k != q.quats(); ++k)
            {
                auto tensor = blaze::quatslice(q, k);
                for (std::size_t i = 0; i != q.pages(); ++i)
                {
                    auto slice = blaze::pageslice(tensor, i);
                    for (std::size_t j = 0; j != q.rows(); ++j)
                    {
                        Op<T> op{ name_, codename_ };
                        auto row = blaze::row(slice, j);
                        result(k, i, 0, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicTensor<T> result(q.quats(), q.pages(), q.rows());
        for (std::size_t k = 0; k != q.quats(); ++k)
        {
            auto tensor = blaze::quatslice(q, k);
            for (std::size_t i = 0; i != q.pages(); ++i)
            {
                auto slice = blaze::pageslice(tensor, i);
                for (std::size_t j = 0; j != q.rows(); ++j)
                {
                    Op<T> op{ name_, codename_ };
                    auto row = blaze::row(slice, j);
                    result(k, i, j) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T, typename Init>
    primitive_argument_type statistics<Op, Derived>::statistics4d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<Init> const& initial) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -4: HPX_FALLTHROUGH;
            case 0:
                return statistics4d_axis0(std::move(arg), keepdims, initial);

            case -3: HPX_FALLTHROUGH;
            case 1:
                return statistics4d_axis1(std::move(arg), keepdims, initial);

            case -2: HPX_FALLTHROUGH;
            case 2:
                return statistics4d_axis2(std::move(arg), keepdims, initial);

            case -1: HPX_FALLTHROUGH;
            case 3:
                return statistics4d_axis3(std::move(arg), keepdims, initial);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::statistics4d",
                    generate_error_message(
                        "the statistics_operation primitive requires operand "
                        "axis to be between -4 and 3 for 4d arrays."));
            }
        }
        return statistics4d_flat(std::move(arg), keepdims, initial);
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statisticsnd(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, primitive_argument_type&& initial) const
    {
        using initial_type = typename Op<T>::result_type;

        hpx::util::optional<initial_type> initial_value;
        if (valid(initial))
        {
            initial_value = extract_scalar_data<initial_type>(
                std::move(initial), name_, codename_);
        }

        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return statistics0d(std::move(arg), axis, keepdims, initial_value);

        case 1:
            return statistics1d(std::move(arg), axis, keepdims, initial_value);

        case 2:
            return statistics2d(std::move(arg), axis, keepdims, initial_value);

        case 3:
            return statistics3d(std::move(arg), axis, keepdims, initial_value);

        case 4:
            return statistics4d(std::move(arg), axis, keepdims, initial_value);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statisticsnd",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statisticsnd(
        primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        primitive_argument_type&& initial) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return statisticsnd(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                axis, keepdims, std::move(initial));

        case node_data_type_int64:
            return statisticsnd(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                axis, keepdims, std::move(initial));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return statisticsnd(
                extract_numeric_value(std::move(arg), name_, codename_), axis,
                keepdims, std::move(initial));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statisticsnd",
            generate_error_message(
                "the statistics primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statisticsnd_flat(
        arg_type<T>&& arg, bool keepdims,
        primitive_argument_type&& initial) const
    {
        using initial_type = typename Op<T>::result_type;

        hpx::util::optional<initial_type> initial_value;
        if (valid(initial))
        {
            initial_value = extract_scalar_data<initial_type>(
                std::move(initial), name_, codename_);
        }

        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return statistics0d(std::move(arg),
                hpx::util::optional<std::int64_t>(), keepdims, initial_value);

        case 1:
            return statistics1d(std::move(arg),
                hpx::util::optional<std::int64_t>(), keepdims, initial_value);

        case 2:
            return statistics2d_flat(
                std::move(arg), keepdims, initial_value);

        case 3:
            return statistics3d_flat(
                std::move(arg), keepdims, initial_value);

        case 4:
            return statistics4d_flat(
                std::move(arg), keepdims, initial_value);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statisticsnd_flat",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statisticsnd_flat(
        primitive_argument_type&& arg, bool keepdims,
        primitive_argument_type&& initial) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(arg);
        }

        switch (t)
        {
        case node_data_type_bool:
            return statisticsnd_flat(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                keepdims, std::move(initial));

        case node_data_type_int64:
            return statisticsnd_flat(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                keepdims, std::move(initial));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return statisticsnd_flat(
                extract_numeric_value(std::move(arg), name_, codename_),
                keepdims, std::move(initial));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statisticsnd_flat",
            generate_error_message(
                "the statistics primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        inline void verify_axis(std::int64_t axis, std::int64_t min_axis,
            std::int64_t max_axis, char const* type,
            std::string const& name, std::string const& codename)
        {
            if (axis < min_axis || axis > max_axis)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::verify_axis",
                    util::generate_error_message(hpx::util::format(
                        "the statistics_operation primitive requires operand "
                        "axis to be between {} and {} for {}.",
                        min_axis, max_axis, type), name, codename));
            }
        }
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statisticsnd(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial) const
    {
        std::size_t dims =
            extract_numeric_value_dimension(arg, name_, codename_);

        switch (axes.size())
        {
        case 0:
            // empty list given, assume no axis being specified
            return statisticsnd(std::move(arg),
                hpx::util::optional<std::int64_t>(), keepdims,
                std::move(initial));

        case 1:
            // one axis specified, requires at least 1D data to work with
            if (dims >= 1)
            {
                std::int64_t axis = extract_scalar_integer_value(
                    *axes.begin(), name_, codename_);
                return statisticsnd(std::move(arg),
                    hpx::util::optional<std::int64_t>(axis), keepdims,
                    std::move(initial));
            }
            break;

        case 2:
            // two axes specified, requires at least 2D data to work with
            {
                auto it = axes.begin();
                std::int64_t axis0 =
                    extract_scalar_integer_value(*it, name_, codename_);
                std::int64_t axis1 =
                    extract_scalar_integer_value(*++it, name_, codename_);
                axis0 = axis0 < 0 ? axis0 + dims : axis0;
                axis1 = axis1 < 0 ? axis1 + dims : axis1;
                if (axis0 == axis1)
                {
                    // axes must be unique
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "statistics::statisticsnd",
                        generate_error_message(
                            "the statistics primitive requires for all axis "
                            "arguments to be unique"));
                }

                if (dims == 2)
                {
                    detail::verify_axis(
                        axis0, 0, 1, "matrices", name_, codename_);
                    detail::verify_axis(
                        axis1, 0, 1, "matrices", name_, codename_);

                    return statisticsnd_flat(
                        std::move(arg), keepdims, std::move(initial));
                }
                else if (dims == 3)
                {
                    detail::verify_axis(
                        axis0, 0, 2, "tensors", name_, codename_);
                    detail::verify_axis(
                        axis1, 0, 2, "tensors", name_, codename_);

                    return statistics3d_slice(std::move(arg), axis0, axis1,
                        keepdims, std::move(initial));
                }
                else if (dims == 4)
                {
                    detail::verify_axis(
                        axis0, 0, 3, "4d arrays", name_, codename_);
                    detail::verify_axis(
                        axis1, 0, 3, "4d arrays", name_, codename_);

                    return statistics4d_slice(std::move(arg), axis0, axis1,
                        keepdims, std::move(initial));
                }
            }
            break;

        case 3:
            // three axes specified, requires at least 3D data to work with
            {
                auto it = axes.begin();
                std::int64_t axis0 =
                    extract_scalar_integer_value(*it, name_, codename_);
                std::int64_t axis1 =
                    extract_scalar_integer_value(*++it, name_, codename_);
                std::int64_t axis2 =
                    extract_scalar_integer_value(*++it, name_, codename_);
                axis0 = axis0 < 0 ? axis0 + dims : axis0;
                axis1 = axis1 < 0 ? axis1 + dims : axis1;
                axis2 = axis2 < 0 ? axis2 + dims : axis2;
                if (axis0 == axis1 || axis0 == axis2 || axis1 == axis2)
                {
                    // axes must be unique
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "statistics::statisticsnd",
                        generate_error_message(
                            "the statistics primitive requires for all axis "
                            "arguments to be unique"));
                }

                if (dims == 3)
                {
                    detail::verify_axis(axis0, 0, 2, "tensors", name_, codename_);
                    detail::verify_axis(axis1, 0, 2, "tensors", name_, codename_);
                    detail::verify_axis(axis2, 0, 2, "tensors", name_, codename_);

                    return statisticsnd_flat(
                        std::move(arg), keepdims, std::move(initial));
                }

                else if (dims == 4)
                {
                    detail::verify_axis(
                        axis0, 0, 3, "4d arrays", name_, codename_);
                    detail::verify_axis(
                        axis1, 0, 3, "4d arrays", name_, codename_);
                    detail::verify_axis(
                        axis2, 0, 3, "4d arrays", name_, codename_);

                    return statistics4d_tensor(std::move(arg), axis0, axis1,
                        axis2, keepdims, std::move(initial));
                }
            }

        case 4:
            {
                auto it = axes.begin();
                std::int64_t axis0 =
                    extract_scalar_integer_value(*it, name_, codename_);
                std::int64_t axis1 =
                    extract_scalar_integer_value(*++it, name_, codename_);
                std::int64_t axis2 =
                    extract_scalar_integer_value(*++it, name_, codename_);
                std::int64_t axis3 =
                    extract_scalar_integer_value(*++it, name_, codename_);
                axis0 = axis0 < 0 ? axis0 + dims : axis0;
                axis1 = axis1 < 0 ? axis1 + dims : axis1;
                axis2 = axis2 < 0 ? axis2 + dims : axis2;
                axis3 = axis3 < 0 ? axis3 + dims : axis3;
                if (axis0 == axis1 || axis0 == axis2 || axis1 == axis2 ||
                    axis0 == axis3 || axis1 == axis3 || axis2 == axis3)
                {
                    // axes must be unique
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "statistics::statisticsnd",
                        generate_error_message(
                            "the statistics primitive requires for all axis "
                            "arguments to be unique"));
                }

                detail::verify_axis(axis0, 0, 3, "4d arrays", name_, codename_);
                detail::verify_axis(axis1, 0, 3, "4d arrays", name_, codename_);
                detail::verify_axis(axis2, 0, 3, "4d arrays", name_, codename_);
                detail::verify_axis(axis3, 0, 3, "4d arrays", name_, codename_);

                return statisticsnd_flat(
                    std::move(arg), keepdims, std::move(initial));
            }
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "statistics::statisticsnd",
            generate_error_message(hpx::util::format(
                "invalid number of axis specified ({}), "
                "should be not larger than the data's dimension ({})",
                axes.size(), dims)));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    hpx::future<primitive_argument_type> statistics<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() ||
            operands.size() > derived().match_data.patterns_.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::eval",
                generate_error_message(
                    "the statistics primitive requires exactly one, two, or "
                        "three operands"));
        }

        std::size_t count = 0;
        for (auto const& i : operands)
        {
            // axis (arg1) and keepdims (arg2) are allowed to be nil
            if (count != 1 && count != 2 && !valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::eval",
                    generate_error_message(
                        "the statistics_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
            ++count;
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                // Extract axis and keepdims
                // Presence of axis changes behavior for >1d cases
                hpx::util::optional<std::int64_t> axis;
                bool keepdims = false;
                primitive_argument_type initial;

                if (args.size() > 1)
                {
                    // keepdims is (optional) argument #3
                    if (args.size() > 2 && valid(args[2]))
                    {
                        keepdims = extract_scalar_boolean_value(
                            args[2], this_->name_, this_->codename_);
                    }

                    // initial is (optional) argument #4
                    if (args.size() > 3)
                    {
                        initial = std::move(args[3]);
                    }

                    // axis is (optional) argument #2
                    if (valid(args[1]) && !is_explicit_nil(args[1]))
                    {
                        // the second argument is either a list of integers...
                        if (is_list_operand_strict(args[1]))
                        {
                            return this_->statisticsnd(std::move(args[0]),
                                extract_list_value_strict(std::move(args[1]),
                                    this_->name_, this_->codename_),
                                keepdims, std::move(initial));
                        }

                        // ... or a single integer
                        axis = extract_scalar_integer_value(
                            args[1], this_->name_, this_->codename_);
                    }
                }

                return this_->statisticsnd(
                    std::move(args[0]), axis, keepdims, std::move(initial));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

#endif
