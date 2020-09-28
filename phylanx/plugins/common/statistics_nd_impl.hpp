// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_STATISTICS_IMPL_DEC_24_2018_0342PM)
#define PHYLANX_COMMON_STATISTICS_IMPL_DEC_24_2018_0342PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/statistics_nd.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/assert.hpp>
#include <hpx/datastructures/optional.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common {

    namespace detail {

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics0d(
            ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (axis)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statistics0d",
                    util::generate_error_message(
                        "the statistics_operation primitive requires that no "
                        "axis is specified for scalar values.",
                        name, codename, ctx.back_trace()));
            }

            Op<T> op{name, codename};

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            return execution_tree::primitive_argument_type{
                op(execution_tree::extract_scalar_data<result_type>(
                       std::move(arg), name, codename),
                    initial_value)};
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics1d_axis(
            ir::node_data<T>&& arg, hpx::util::optional<Init> const& initial,
            bool keepdims, std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            Op<T> op{name, codename};

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            auto v = arg.vector();
            T result = op(v, initial_value);

            if (keepdims)
            {
                using result_type = typename Op<T>::result_type;

                return execution_tree::primitive_argument_type{
                    blaze::DynamicVector<result_type>(
                        1, op.finalize(result, v.size()))};
            }

            return execution_tree::primitive_argument_type{
                op.finalize(result, v.size())};
        }

        ///////////////////////////////////////////////////////////////////////
        template <typename F1, typename F2>
        auto invoke_if(F1&& f1, F2&& f2, std::false_type)
        {
            return f2();
        }

        template <typename F1, typename F2>
        auto invoke_if(F1&& f1, F2&& f2, std::true_type)
        {
            return f1();
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics1d(
            ir::node_data<T>&& arg, hpx::util::optional<Init> const& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            Op<T> op{name, codename};

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            auto v = arg.vector();
            std::size_t size = v.size();

            using result_type = typename Op<T>::result_type;

            auto f_ref = [&]() {
                blaze::DynamicVector<result_type> result(size);
                for (std::size_t i = 0; i != size; ++i)
                {
                    result[i] = op(v[i], initial_value);
                }
                return execution_tree::primitive_argument_type{
                    std::move(result) };
            };

            if (arg.is_ref())
            {
                return f_ref();
            }

            auto f = [&]() {
                for (std::size_t i = 0; i != size; ++i)
                {
                    v[i] = op(v[i], initial_value);
                }
                return execution_tree::primitive_argument_type{std::move(arg)};
            };

            using pred = typename std::is_same<result_type, T>::type;
            return invoke_if(std::move(f), std::move(f_ref), pred{});
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics1d(
            ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (axis)
            {
                if (axis.value() == 0 || axis.value() == -1)
                {
                    return statistics1d_axis<Op>(std::move(arg), initial,
                        keepdims, name, codename, std::move(ctx));
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statistics1d",
                    util::generate_error_message(
                        "the statistics_operation primitive requires "
                        "operand axis to be either 0 or -1 for vectors.",
                        name, codename, ctx.back_trace()));
            }

            Op<T> op{ name, codename };

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            auto v = arg.vector();
            T result = op(v, initial_value);
            if (keepdims)
            {
                using result_type = typename Op<T>::result_type;

                return execution_tree::primitive_argument_type{
                    blaze::DynamicVector<result_type>(
                        1, op.finalize(result, v.size()))};
            }

            return execution_tree::primitive_argument_type{
                op.finalize(result, v.size())};
        }

        ////////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics2d_flat(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto m = arg.matrix();

            Op<T> op{name, codename};
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
                using result_type = typename Op<T>::result_type;

                return execution_tree::primitive_argument_type{
                    blaze::DynamicMatrix<result_type>(
                        1, 1, op.finalize(result, size))};
            }

            return execution_tree::primitive_argument_type{
                op.finalize(result, size)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics2d_axis0(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto m = arg.matrix();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicMatrix<result_type> result(1, m.columns());
                for (std::size_t i = 0; i != m.columns(); ++i)
                {
                    Op<T> op{name, codename};
                    auto col = blaze::column(m, i);
                    result(0, i) =
                        op.finalize(op(col, initial_value), col.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(m.columns());
            for (std::size_t i = 0; i != m.columns(); ++i)
            {
                Op<T> op{name, codename};
                auto col = blaze::column(m, i);
                result[i] = op.finalize(op(col, initial_value), col.size());
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics2d_axis1(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto m = arg.matrix();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicMatrix<result_type> result(m.rows(), 1);
                for (std::size_t i = 0; i != m.rows(); ++i)
                {
                    Op<T> op{name, codename};
                    auto row = blaze::row(m, i);
                    result(i, 0) =
                        op.finalize(op(row, initial_value), row.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(m.rows());
            for (std::size_t i = 0; i != m.rows(); ++i)
            {
                Op<T> op{name, codename};
                auto row = blaze::row(m, i);
                result[i] = op.finalize(op(row, initial_value), row.size());
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics2d(
            ir::node_data<T>&& arg, hpx::util::optional<Init> const& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            Op<T> op{name, codename};

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            auto m = arg.matrix();
            std::size_t rows = m.rows();
            std::size_t columns = m.columns();

            using result_type = typename Op<T>::result_type;

            auto f_ref = [&]() {
                blaze::DynamicMatrix<result_type> result(rows, columns);
                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        result(i, j) = op(m(i, j), initial_value);
                    }
                }
                return execution_tree::primitive_argument_type{
                    std::move(result) };
            };

            if (arg.is_ref())
            {
                return f_ref();
            }

            auto f = [&]() {
                for (std::size_t i = 0; i != rows; ++i)
                {
                    for (std::size_t j = 0; j != columns; ++j)
                    {
                        m(i, j) = op(m(i, j), initial_value);
                    }
                }
                return execution_tree::primitive_argument_type{std::move(arg)};
            };

            using pred = typename std::is_same<result_type, T>::type;
            return invoke_if(std::move(f), std::move(f_ref), pred{});
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics2d(
            ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (axis)
            {
                switch (axis.value())
                {
                case -2:
                    HPX_FALLTHROUGH;
                case 0:
                    return statistics2d_axis0<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                case -1:
                    HPX_FALLTHROUGH;
                case 1:
                    return statistics2d_axis1<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "common::statistics2d",
                        util::generate_error_message(
                            "the statistics_operation primitive requires "
                            "operand axis to be between -2 and 1 for matrices.",
                            name, codename, ctx.back_trace()));
                }
            }
            return statistics2d_flat<Op>(std::move(arg), keepdims, initial,
                name, codename, std::move(ctx));
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_flat(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Op<T> op{name, codename};

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
                using result_type = typename Op<T>::result_type;

                return execution_tree::primitive_argument_type{
                    blaze::DynamicTensor<result_type>(
                        1, 1, 1, op.finalize(result, size))};
            }

            return execution_tree::primitive_argument_type{
                op.finalize(result, size)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_axis0(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicTensor<result_type> result(
                    1, t.rows(), t.columns());
                for (std::size_t i = 0; i != t.rows(); ++i)
                {
                    auto slice = blaze::rowslice(t, i);
                    for (std::size_t j = 0; j != t.columns(); ++j)
                    {
                        Op<T> op{name, codename};
                        auto row = blaze::row(slice, j);
                        result(0, i, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    Op<T> op{name, codename};
                    auto row = blaze::row(slice, j);
                    result(i, j) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_axis1(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicTensor<result_type> result(
                    t.pages(), 1, t.columns());
                for (std::size_t k = 0; k != t.pages(); ++k)
                {
                    auto slice = blaze::pageslice(t, k);
                    for (std::size_t j = 0; j != t.columns(); ++j)
                    {
                        Op<T> op{name, codename};
                        auto col = blaze::column(slice, j);
                        result(k, 0, j) =
                            op.finalize(op(col, initial_value), col.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(t.pages(), t.columns());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    Op<T> op{name, codename};
                    auto col = blaze::column(slice, j);
                    result(k, j) =
                        op.finalize(op(col, initial_value), col.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_axis2(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicTensor<result_type> result(
                    t.pages(), t.rows(), 1);
                for (std::size_t k = 0; k != t.pages(); ++k)
                {
                    auto slice = blaze::pageslice(t, k);
                    for (std::size_t i = 0; i != t.rows(); ++i)
                    {
                        Op<T> op{name, codename};
                        auto row = blaze::row(slice, i);
                        result(k, i, 0) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(t.pages(), t.rows());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t i = 0; i != t.rows(); ++i)
                {
                    Op<T> op{name, codename};
                    auto row = blaze::row(slice, i);
                    result(k, i) =
                        op.finalize(op(row, initial_value), row.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d(
            ir::node_data<T>&& arg, hpx::util::optional<Init> const& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            Op<T> op{name, codename};

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            auto t = arg.tensor();
            std::size_t pages = t.pages();
            std::size_t rows = t.rows();
            std::size_t columns = t.columns();

            using result_type = typename Op<T>::result_type;

            auto f_ref = [&]() {
                blaze::DynamicTensor<result_type> result(pages, rows, columns);
                for (std::size_t k = 0; k != pages; ++k)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            result(k, i, j) = op(t(k, i, j), initial_value);
                        }
                    }
                }
                return execution_tree::primitive_argument_type{
                    std::move(result)};
            };

            if (arg.is_ref())
            {
                return f_ref();
            }

            auto f = [&]() {
                for (std::size_t k = 0; k != pages; ++k)
                {
                    for (std::size_t i = 0; i != rows; ++i)
                    {
                        for (std::size_t j = 0; j != columns; ++j)
                        {
                            t(k, i, j) = op(t(k, i, j), initial_value);
                        }
                    }
                }
                return execution_tree::primitive_argument_type{std::move(arg)};
            };

            using pred = typename std::is_same<result_type, T>::type;
            return invoke_if(std::move(f), std::move(f_ref), pred{});
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d(
            ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (axis)
            {
                switch (axis.value())
                {
                case -3:
                    HPX_FALLTHROUGH;
                case 0:
                    return statistics3d_axis0<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                case -2:
                    HPX_FALLTHROUGH;
                case 1:
                    return statistics3d_axis1<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                case -1:
                    HPX_FALLTHROUGH;
                case 2:
                    return statistics3d_axis2<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "common::statistics3d",
                        util::generate_error_message(
                            "the statistics_operation primitive requires "
                            "operand axis to be between -3 and 2 for tensors.",
                            name, codename, ctx.back_trace()));
                }
            }
            return statistics3d_flat<Op>(std::move(arg), keepdims, initial,
                name, codename, std::move(ctx));
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_columnslice(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicTensor<result_type> result(1, 1, t.columns());
                for (std::size_t k = 0; k != t.columns(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::columnslice(t, k));
                    result(0, 0, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(t.columns());
            for (std::size_t k = 0; k != t.columns(); ++k)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(blaze::columnslice(t, k));
                result[k] = op.finalize(op(slice, initial_value), slice.size());
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_rowslice(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicTensor<result_type> result(1, t.rows(), 1);
                for (std::size_t k = 0; k != t.rows(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::rowslice(t, k));
                    result(0, k, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(t.rows());
            for (std::size_t k = 0; k != t.rows(); ++k)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(blaze::rowslice(t, k));
                result[k] = op.finalize(op(slice, initial_value), slice.size());
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics3d_pageslice(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto t = arg.tensor();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicTensor<result_type> result(t.pages(), 1, 1);
                for (std::size_t k = 0; k != t.pages(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::pageslice(t, k));
                    result(k, 0, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(t.pages());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(blaze::pageslice(t, k));
                result[k] = op.finalize(op(slice, initial_value), slice.size());
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T>
        execution_tree::primitive_argument_type statistics3d_slice(
            ir::node_data<T>&& arg, std::int64_t axis0, std::int64_t axis1,
            bool keepdims, execution_tree::primitive_argument_type&& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            using result_type = typename Op<T>::result_type;

            hpx::util::optional<result_type> initial_value;
            if (execution_tree::valid(initial))
            {
                initial_value =
                    execution_tree::extract_scalar_data<result_type>(
                        std::move(initial), name, codename);
            }

            if (axis0 == 0)
            {
                if (axis1 == 1)
                {
                    return statistics3d_columnslice<Op>(std::move(arg),
                        keepdims, initial_value, name, codename,
                        std::move(ctx));
                }

                // axis0 == 0 && axis1 == 2
                return statistics3d_rowslice<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));
            }

            // axis0 == 1 && axis1 == 2
            return statistics3d_pageslice<Op>(std::move(arg), keepdims,
                initial_value, name, codename, std::move(ctx));
        }

        template <template <class T> class Op>
        execution_tree::primitive_argument_type statistics3d_slice(
            execution_tree::primitive_argument_type&& arg, std::int64_t axis0,
            std::int64_t axis1, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            execution_tree::node_data_type dtype, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (dtype == execution_tree::node_data_type_unknown)
            {
                dtype = execution_tree::extract_common_type(arg);
            }

            if (axis0 > axis1)
            {
                std::swap(axis0, axis1);
            }

            switch (dtype)
            {
            case execution_tree::node_data_type_bool:
                return statistics3d_slice<Op>(
                    extract_boolean_value_strict(
                        std::move(arg), name, codename),
                    axis0, axis1, keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            case execution_tree::node_data_type_int64:
                return statistics3d_slice<Op>(
                    extract_integer_value_strict(
                        std::move(arg), name, codename),
                    axis0, axis1, keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            case execution_tree::node_data_type_unknown:
                HPX_FALLTHROUGH;
            case execution_tree::node_data_type_double:
                return statistics3d_slice<Op>(
                    extract_numeric_value(std::move(arg), name, codename),
                    axis0, axis1, keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "common::statistics3d_slice",
                util::generate_error_message(
                    "the statistics primitive requires for all arguments "
                    "to be numeric data types",
                    name, codename, ctx.back_trace()));
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_slice01(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    1, 1, q.rows(), q.columns());
                for (std::size_t l = 0; l != q.rows(); ++l)
                {
                    auto tensor =
                        blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l);
                    for (std::size_t k = 0; k != q.columns(); ++k)
                    {
                        Op<T> op{name, codename};
                        auto slice =
                            blaze::ravel(blaze::columnslice(tensor, k));
                        result(0, 0, l, k) =
                            op.finalize(op(slice, initial_value), slice.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(q.rows(), q.columns());
            for (std::size_t l = 0; l != q.rows(); ++l)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l);
                for (std::size_t k = 0; k != q.columns(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                    result(l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_slice02(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    1, q.pages(), 1, q.columns());
                for (std::size_t l = 0; l != q.pages(); ++l)
                {
                    auto tensor =
                        blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
                    for (std::size_t k = 0; k != q.columns(); ++k)
                    {
                        Op<T> op{name, codename};
                        auto slice =
                            blaze::ravel(blaze::columnslice(tensor, k));
                        result(0, 0, l, k) =
                            op.finalize(op(slice, initial_value), slice.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(q.pages(), q.columns());
            for (std::size_t l = 0; l != q.pages(); ++l)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
                for (std::size_t k = 0; k != q.columns(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                    result(l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_slice03(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    1, q.pages(), q.rows(), 1);
                for (std::size_t l = 0; l != q.pages(); ++l)
                {
                    auto tensor =
                        blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
                    for (std::size_t k = 0; k != q.rows(); ++k)
                    {
                        Op<T> op{name, codename};
                        auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                        result(0, l, k, 0) =
                            op.finalize(op(slice, initial_value), slice.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(q.pages(), q.rows());
            for (std::size_t l = 0; l != q.pages(); ++l)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l);
                for (std::size_t k = 0; k != q.rows(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                    result(l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_slice12(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), 1, 1, q.columns());
                for (std::size_t l = 0; l != q.quats(); ++l)
                {
                    auto tensor = blaze::quatslice(q, l);
                    for (std::size_t k = 0; k != q.columns(); ++k)
                    {
                        Op<T> op{name, codename};
                        auto slice =
                            blaze::ravel(blaze::columnslice(tensor, k));
                        result(l, 0, 0, k) =
                            op.finalize(op(slice, initial_value), slice.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(q.quats(), q.columns());
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                auto tensor = blaze::quatslice(q, l);
                for (std::size_t k = 0; k != q.columns(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::columnslice(tensor, k));
                    result(l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_slice13(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), 1, q.rows(), 1);
                for (std::size_t l = 0; l != q.quats(); ++l)
                {
                    auto tensor = blaze::quatslice(q, l);
                    for (std::size_t k = 0; k != q.rows(); ++k)
                    {
                        Op<T> op{name, codename};
                        auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                        result(l, 0, k, 0) =
                            op.finalize(op(slice, initial_value), slice.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(q.quats(), q.rows());
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                auto tensor = blaze::quatslice(q, l);
                for (std::size_t k = 0; k != q.rows(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::rowslice(tensor, k));
                    result(l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_slice23(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), q.pages(), 1, 1);
                for (std::size_t l = 0; l != q.quats(); ++l)
                {
                    auto tensor = blaze::quatslice(q, l);
                    for (std::size_t k = 0; k != q.pages(); ++k)
                    {
                        Op<T> op{name, codename};
                        auto slice = blaze::ravel(blaze::pageslice(tensor, k));
                        result(l, k, 0, 0) =
                            op.finalize(op(slice, initial_value), slice.size());
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicMatrix<result_type> result(q.quats(), q.pages());
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                auto tensor = blaze::quatslice(q, l);
                for (std::size_t k = 0; k != q.pages(); ++k)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::pageslice(tensor, k));
                    result(l, k) =
                        op.finalize(op(slice, initial_value), slice.size());
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T>
        execution_tree::primitive_argument_type statistics4d_slice(
            ir::node_data<T>&& arg, std::int64_t axis0, std::int64_t axis1,
            bool keepdims, execution_tree::primitive_argument_type&& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            using result_type = typename Op<T>::result_type;

            hpx::util::optional<result_type> initial_value;
            if (execution_tree::valid(initial))
            {
                initial_value =
                    execution_tree::extract_scalar_data<result_type>(
                        std::move(initial), name, codename);
            }

            if (axis0 == 0)
            {
                if (axis1 == 1)
                {
                    return statistics4d_slice01<Op>(std::move(arg), keepdims,
                        initial_value, name, codename, std::move(ctx));
                }

                else if (axis1 == 2)
                {
                    return statistics4d_slice02<Op>(std::move(arg), keepdims,
                        initial_value, name, codename, std::move(ctx));
                }
                // axis0 == 0 && axis1 == 3
                return statistics4d_slice03<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));
            }

            else if (axis0 == 1)
            {
                if (axis1 == 2)
                {
                    return statistics4d_slice12<Op>(std::move(arg), keepdims,
                        initial_value, name, codename, std::move(ctx));
                }
                // axis0 == 1 && axis1 == 3
                return statistics4d_slice13<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));
            }

            // axis0 == 2 && axis1 == 3
            return statistics4d_slice23<Op>(std::move(arg), keepdims,
                initial_value, name, codename, std::move(ctx));
        }

        template <template <class T> class Op>
        execution_tree::primitive_argument_type statistics4d_slice(
            execution_tree::primitive_argument_type&& arg, std::int64_t axis0,
            std::int64_t axis1, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            execution_tree::node_data_type dtype, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (dtype == execution_tree::node_data_type_unknown)
            {
                dtype = execution_tree::extract_common_type(arg);
            }

            if (axis0 > axis1)
            {
                std::swap(axis0, axis1);
            }

            switch (dtype)
            {
            case execution_tree::node_data_type_bool:
                return statistics4d_slice<Op>(
                    extract_boolean_value_strict(
                        std::move(arg), name, codename),
                    axis0, axis1, keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            case execution_tree::node_data_type_int64:
                return statistics4d_slice<Op>(
                    extract_integer_value_strict(
                        std::move(arg), name, codename),
                    axis0, axis1, keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            case execution_tree::node_data_type_unknown:
                HPX_FALLTHROUGH;
            case execution_tree::node_data_type_double:
                return statistics4d_slice<Op>(
                    extract_numeric_value(std::move(arg), name, codename),
                    axis0, axis1, keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "common::statistics4d_slice",
                util::generate_error_message(
                    "the statistics primitive requires for all arguments "
                    "to be numeric data types",
                    name, codename, ctx.back_trace()));
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_tensor012(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    1, 1, 1, q.columns());
                for (std::size_t l = 0; l != q.columns(); ++l)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(
                        blaze::quatslice(blaze::trans(q, {3, 0, 1, 2}), l));
                    result(0, 0, 0, l) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(q.columns());
            for (std::size_t l = 0; l != q.columns(); ++l)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(
                    blaze::quatslice(blaze::trans(q, {3, 0, 1, 2}), l));
                result[l] = op.finalize(op(slice, initial_value), slice.size());
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_tensor013(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(1, 1, q.rows(), 1);
                for (std::size_t l = 0; l != q.rows(); ++l)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(
                        blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l));
                    result(0, 0, l, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(q.rows());
            for (std::size_t l = 0; l != q.rows(); ++l)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(
                    blaze::quatslice(blaze::trans(q, {2, 0, 1, 3}), l));
                result[l] = op.finalize(op(slice, initial_value), slice.size());
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_tensor023(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    1, q.pages(), 1, 1);
                for (std::size_t l = 0; l != q.pages(); ++l)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(
                        blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l));
                    result(0, l, 0, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(q.pages());
            for (std::size_t l = 0; l != q.pages(); ++l)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), l));
                result[l] = op.finalize(op(slice, initial_value), slice.size());
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_tensor123(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), 1, 1, 1);
                for (std::size_t l = 0; l != q.quats(); ++l)
                {
                    Op<T> op{name, codename};
                    auto slice = blaze::ravel(blaze::quatslice(q, l));
                    result(l, 0, 0, 0) =
                        op.finalize(op(slice, initial_value), slice.size());
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicVector<result_type> result(q.quats());
            for (std::size_t l = 0; l != q.quats(); ++l)
            {
                Op<T> op{name, codename};
                auto slice = blaze::ravel(blaze::quatslice(q, l));
                result[l] = op.finalize(op(slice, initial_value), slice.size());
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T>
        execution_tree::primitive_argument_type statistics4d_tensor(
            ir::node_data<T>&& arg, std::int64_t axis0, std::int64_t axis1,
            std::int64_t axis2, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            using result_type = typename Op<T>::result_type;

            hpx::util::optional<result_type> initial_value;
            if (execution_tree::valid(initial))
            {
                initial_value =
                    execution_tree::extract_scalar_data<result_type>(
                        std::move(initial), name, codename);
            }

            // axes are unique, so each of 4 combinations has a different sum
            switch (axis0 + axis1 + axis2)
            {
            case 3:
                return statistics4d_tensor012<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 4:
                return statistics4d_tensor013<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 5:
                return statistics4d_tensor023<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 6:
                return statistics4d_tensor123<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "common::statistics4d_tensor",
                util::generate_error_message("invalid combination of axes",
                    name, codename, ctx.back_trace()));
        }

        template <template <class T> class Op>
        execution_tree::primitive_argument_type statistics4d_tensor(
            execution_tree::primitive_argument_type&& arg, std::int64_t axis0,
            std::int64_t axis1, std::int64_t axis2, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            execution_tree::node_data_type dtype, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (dtype == execution_tree::node_data_type_unknown)
            {
                dtype = execution_tree::extract_common_type(arg);
            }

            switch (dtype)
            {
            case execution_tree::node_data_type_bool:
                return statistics4d_tensor<Op>(
                    execution_tree::extract_boolean_value_strict(
                        std::move(arg), name, codename),
                    axis0, axis1, axis2, keepdims, std::move(initial), name,
                    codename, std::move(ctx));

            case execution_tree::node_data_type_int64:
                return statistics4d_tensor<Op>(
                    execution_tree::extract_integer_value_strict(
                        std::move(arg), name, codename),
                    axis0, axis1, axis2, keepdims, std::move(initial), name,
                    codename, std::move(ctx));

            case execution_tree::node_data_type_unknown:
                HPX_FALLTHROUGH;
            case execution_tree::node_data_type_double:
                return statistics4d_tensor<Op>(
                    execution_tree::extract_numeric_value(
                        std::move(arg), name, codename),
                    axis0, axis1, axis2, keepdims, std::move(initial), name,
                    codename, std::move(ctx));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "common::statistics4d_tensor",
                util::generate_error_message(
                    "the statistics primitive requires for all arguments "
                    "to be numeric data types",
                    name, codename, ctx.back_trace()));
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_flat(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Op<T> op{name, codename};

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
                using result_type = typename Op<T>::result_type;

                return execution_tree::primitive_argument_type{
                    blaze::DynamicArray<4UL, result_type>(
                        blaze::init_from_value, op.finalize(result, size), 1, 1,
                        1, 1)};
            }

            return execution_tree::primitive_argument_type{
                op.finalize(result, size)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_axis0(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
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
                            Op<T> op{name, codename};
                            auto row = blaze::row(slice, j);
                            result(0, k, i, j) =
                                op.finalize(op(row, initial_value), row.size());
                        }
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicTensor<result_type> result(
                q.pages(), q.rows(), q.columns());
            for (std::size_t k = 0; k != q.pages(); ++k)
            {
                auto tensor =
                    blaze::quatslice(blaze::trans(q, {1, 0, 2, 3}), k);
                for (std::size_t i = 0; i != q.rows(); ++i)
                {
                    auto slice = blaze::rowslice(tensor, i);
                    for (std::size_t j = 0; j != q.columns(); ++j)
                    {
                        Op<T> op{name, codename};
                        auto row = blaze::row(slice, j);
                        result(k, i, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_axis1(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), 1, q.rows(), q.columns());
                for (std::size_t k = 0; k != q.quats(); ++k)
                {
                    auto tensor = blaze::quatslice(q, k);
                    for (std::size_t i = 0; i != q.rows(); ++i)
                    {
                        auto slice = blaze::rowslice(tensor, i);
                        for (std::size_t j = 0; j != q.columns(); ++j)
                        {
                            Op<T> op{name, codename};
                            auto row = blaze::row(slice, j);
                            result(k, 0, i, j) =
                                op.finalize(op(row, initial_value), row.size());
                        }
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicTensor<result_type> result(
                q.quats(), q.rows(), q.columns());
            for (std::size_t k = 0; k != q.quats(); ++k)
            {
                auto tensor = blaze::quatslice(q, k);
                for (std::size_t i = 0; i != q.rows(); ++i)
                {
                    auto slice = blaze::rowslice(tensor, i);
                    for (std::size_t j = 0; j != q.columns(); ++j)
                    {
                        Op<T> op{name, codename};
                        auto row = blaze::row(slice, j);
                        result(k, i, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_axis2(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), q.pages(), 1, q.columns());
                for (std::size_t k = 0; k != q.quats(); ++k)
                {
                    auto tensor = blaze::quatslice(q, k);
                    for (std::size_t i = 0; i != q.pages(); ++i)
                    {
                        auto slice = blaze::pageslice(tensor, i);
                        for (std::size_t j = 0; j != q.columns(); ++j)
                        {
                            Op<T> op{name, codename};
                            auto col = blaze::column(slice, j);
                            result(k, i, 0, j) =
                                op.finalize(op(col, initial_value), col.size());
                        }
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicTensor<result_type> result(
                q.quats(), q.pages(), q.columns());
            for (std::size_t k = 0; k != q.quats(); ++k)
            {
                auto tensor = blaze::quatslice(q, k);
                for (std::size_t i = 0; i != q.pages(); ++i)
                {
                    auto slice = blaze::pageslice(tensor, i);
                    for (std::size_t j = 0; j != q.columns(); ++j)
                    {
                        Op<T> op{name, codename};
                        auto col = blaze::column(slice, j);
                        result(k, i, j) =
                            op.finalize(op(col, initial_value), col.size());
                    }
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d_axis3(
            ir::node_data<T>&& arg, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            auto q = arg.quatern();

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            using result_type = typename Op<T>::result_type;

            if (keepdims)
            {
                blaze::DynamicArray<4UL, result_type> result(
                    q.quats(), q.pages(), q.rows(), 1);
                for (std::size_t k = 0; k != q.quats(); ++k)
                {
                    auto tensor = blaze::quatslice(q, k);
                    for (std::size_t i = 0; i != q.pages(); ++i)
                    {
                        auto slice = blaze::pageslice(tensor, i);
                        for (std::size_t j = 0; j != q.rows(); ++j)
                        {
                            Op<T> op{name, codename};
                            auto row = blaze::row(slice, j);
                            result(k, i, j, 0) =
                                op.finalize(op(row, initial_value), row.size());
                        }
                    }
                }

                return execution_tree::primitive_argument_type{
                    std::move(result)};
            }

            blaze::DynamicTensor<result_type> result(
                q.quats(), q.pages(), q.rows());
            for (std::size_t k = 0; k != q.quats(); ++k)
            {
                auto tensor = blaze::quatslice(q, k);
                for (std::size_t i = 0; i != q.pages(); ++i)
                {
                    auto slice = blaze::pageslice(tensor, i);
                    for (std::size_t j = 0; j != q.rows(); ++j)
                    {
                        Op<T> op{name, codename};
                        auto row = blaze::row(slice, j);
                        result(k, i, j) =
                            op.finalize(op(row, initial_value), row.size());
                    }
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d(
            ir::node_data<T>&& arg, hpx::util::optional<Init> const& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            Op<T> op{name, codename};

            Init initial_value = Op<T>::initial();
            if (initial)
            {
                initial_value = *initial;
            }

            auto q = arg.quatern();
            std::size_t quats = q.quats();
            std::size_t pages = q.pages();
            std::size_t rows = q.rows();
            std::size_t columns = q.columns();

            using result_type = typename Op<T>::result_type;

            auto f_ref = [&]() {
                blaze::DynamicArray<4UL, result_type> result(
                    quats, pages, rows, columns);
                for (std::size_t l = 0; l != quats; ++l)
                {
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                result(l, k, i, j) =
                                    op(q(l, k, i, j), initial_value);
                            }
                        }
                    }
                }
                return execution_tree::primitive_argument_type{
                    std::move(result)};
            };

            if (arg.is_ref())
            {
                return f_ref();
            }

            auto f = [&]() {
                for (std::size_t l = 0; l != quats; ++l)
                {
                    for (std::size_t k = 0; k != pages; ++k)
                    {
                        for (std::size_t i = 0; i != rows; ++i)
                        {
                            for (std::size_t j = 0; j != columns; ++j)
                            {
                                q(l, k, i, j) =
                                    op(q(l, k, i, j), initial_value);
                            }
                        }
                    }
                }
                return execution_tree::primitive_argument_type{std::move(arg)};
            };

            using pred = typename std::is_same<result_type, T>::type;
            return invoke_if(std::move(f), std::move(f_ref), pred{});
        }

        template <template <class T> class Op, typename T, typename Init>
        execution_tree::primitive_argument_type statistics4d(
            ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            hpx::util::optional<Init> const& initial, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (axis)
            {
                switch (axis.value())
                {
                case -4:
                    HPX_FALLTHROUGH;
                case 0:
                    return statistics4d_axis0<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                case -3:
                    HPX_FALLTHROUGH;
                case 1:
                    return statistics4d_axis1<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                case -2:
                    HPX_FALLTHROUGH;
                case 2:
                    return statistics4d_axis2<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                case -1:
                    HPX_FALLTHROUGH;
                case 3:
                    return statistics4d_axis3<Op>(std::move(arg), keepdims,
                        initial, name, codename, std::move(ctx));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "common::statistics4d",
                        util::generate_error_message(
                            "the statistics_operation primitive requires "
                            "operand axis to be between -4 and 3 for 4d "
                            "arrays.",
                            name, codename, ctx.back_trace()));
                }
            }

            return statistics4d_flat<Op>(std::move(arg), keepdims, initial,
                name, codename, std::move(ctx));
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T>
        execution_tree::primitive_argument_type statisticsnd(
            ir::node_data<T>&& arg,
            hpx::util::optional<std::int64_t> const& axis, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            using initial_type = typename Op<T>::result_type;

            hpx::util::optional<initial_type> initial_value;
            if (execution_tree::valid(initial))
            {
                initial_value =
                    execution_tree::extract_scalar_data<initial_type>(
                        std::move(initial), name, codename);
            }

            switch (arg.num_dimensions())
            {
            case 0:
                return statistics0d<Op>(std::move(arg), axis, initial_value,
                    name, codename, std::move(ctx));

            case 1:
                return statistics1d<Op>(std::move(arg), axis, keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 2:
                return statistics2d<Op>(std::move(arg), axis, keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 3:
                return statistics3d<Op>(std::move(arg), axis, keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 4:
                return statistics4d<Op>(std::move(arg), axis, keepdims,
                    initial_value, name, codename, std::move(ctx));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd",
                    util::generate_error_message(
                        "operand a has an invalid number of dimensions", name,
                        codename, ctx.back_trace()));
            }
        }
    }    // namespace detail

    ///////////////////////////////////////////////////////////////////////
    template <template <class T> class Op>
    execution_tree::primitive_argument_type statisticsnd(
        execution_tree::primitive_argument_type&& arg,
        hpx::util::optional<std::int64_t> const& axis, bool keepdims,
        execution_tree::primitive_argument_type&& initial,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        if (dtype == execution_tree::node_data_type_unknown)
        {
            dtype = execution_tree::extract_common_type(arg);
        }

        switch (dtype)
        {
        case execution_tree::node_data_type_bool:
            return detail::statisticsnd<Op>(
                extract_boolean_value_strict(std::move(arg), name, codename),
                axis, keepdims, std::move(initial), name, codename,
                std::move(ctx));

        case execution_tree::node_data_type_int64:
            return detail::statisticsnd<Op>(
                extract_integer_value_strict(std::move(arg), name, codename),
                axis, keepdims, std::move(initial), name, codename,
                std::move(ctx));

        case execution_tree::node_data_type_unknown:
            HPX_FALLTHROUGH;
        case execution_tree::node_data_type_double:
            return detail::statisticsnd<Op>(
                extract_numeric_value(std::move(arg), name, codename), axis,
                keepdims, std::move(initial), name, codename, std::move(ctx));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd",
            util::generate_error_message(
                "the statistics primitive requires for all arguments "
                "to be numeric data types",
                name, codename, ctx.back_trace()));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <template <class T> class Op, typename T>
        execution_tree::primitive_argument_type statisticsnd(
            ir::node_data<T>&& arg,
            execution_tree::primitive_argument_type&& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            using initial_type = typename Op<T>::result_type;

            hpx::util::optional<initial_type> initial_value;
            if (execution_tree::valid(initial))
            {
                initial_value =
                    execution_tree::extract_scalar_data<initial_type>(
                        std::move(initial), name, codename);
            }

            switch (arg.num_dimensions())
            {
            case 0:
                return statistics0d<Op>(std::move(arg),
                    hpx::util::optional<std::int64_t>(), initial_value, name,
                    codename, std::move(ctx));

            case 1:
                return statistics1d<Op>(std::move(arg), initial_value, name,
                    codename, std::move(ctx));

            case 2:
                return statistics2d<Op>(std::move(arg), initial_value, name,
                    codename, std::move(ctx));

            case 3:
                return statistics3d<Op>(std::move(arg), initial_value, name,
                    codename, std::move(ctx));

            case 4:
                return statistics4d<Op>(std::move(arg), initial_value, name,
                    codename, std::move(ctx));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd",
                    util::generate_error_message("operand a has an unsupported "
                                                 "number of dimensions",
                        name, codename, ctx.back_trace()));
            }
        }

        ///////////////////////////////////////////////////////////////////////
        template <template <class T> class Op>
        execution_tree::primitive_argument_type statisticsnd(
            execution_tree::primitive_argument_type&& arg,
            execution_tree::primitive_argument_type&& initial,
            execution_tree::node_data_type dtype, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (dtype == execution_tree::node_data_type_unknown)
            {
                dtype = execution_tree::extract_common_type(arg);
            }

            switch (dtype)
            {
            case execution_tree::node_data_type_bool:
                return detail::statisticsnd<Op>(
                    execution_tree::extract_boolean_value_strict(
                        std::move(arg), name, codename),
                    std::move(initial), name, codename, std::move(ctx));

            case execution_tree::node_data_type_int64:
                return detail::statisticsnd<Op>(
                    execution_tree::extract_integer_value_strict(
                        std::move(arg), name, codename),
                    std::move(initial), name, codename, std::move(ctx));

            case execution_tree::node_data_type_unknown:
                HPX_FALLTHROUGH;
            case execution_tree::node_data_type_double:
                return detail::statisticsnd<Op>(
                    execution_tree::extract_numeric_value(
                        std::move(arg), name, codename),
                    std::move(initial), name, codename, std::move(ctx));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd",
                util::generate_error_message(
                    "the statistics primitive requires for all arguments "
                    "to be numeric data types",
                    name, codename, ctx.back_trace()));
        }

        ///////////////////////////////////////////////////////////////////////////
        template <template <class T> class Op, typename T>
        execution_tree::primitive_argument_type statisticsnd_flat(
            ir::node_data<T>&& arg, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            std::string const& name, std::string const& codename,
            execution_tree::eval_context ctx)
        {
            using result_type = typename Op<T>::result_type;

            hpx::util::optional<result_type> initial_value;
            if (execution_tree::valid(initial))
            {
                initial_value =
                    execution_tree::extract_scalar_data<result_type>(
                        std::move(initial), name, codename);
            }

            switch (arg.num_dimensions())
            {
            case 0:
                return statistics0d<Op>(std::move(arg),
                    hpx::util::optional<std::int64_t>(), initial_value, name,
                    codename, std::move(ctx));

            case 1:
                return statistics1d<Op>(std::move(arg), initial_value, name,
                    codename, std::move(ctx));

            case 2:
                return statistics2d_flat<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 3:
                return statistics3d_flat<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            case 4:
                return statistics4d_flat<Op>(std::move(arg), keepdims,
                    initial_value, name, codename, std::move(ctx));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "common::statisticsnd_flat",
                    util::generate_error_message(
                        "operand a has an invalid number of dimensions", name,
                        codename, ctx.back_trace()));
            }
        }

        ///////////////////////////////////////////////////////////////////////////
        template <template <class T> class Op>
        execution_tree::primitive_argument_type statisticsnd_flat(
            execution_tree::primitive_argument_type&& arg, bool keepdims,
            execution_tree::primitive_argument_type&& initial,
            execution_tree::node_data_type dtype, std::string const& name,
            std::string const& codename, execution_tree::eval_context ctx)
        {
            if (dtype == execution_tree::node_data_type_unknown)
            {
                dtype = execution_tree::extract_common_type(arg);
            }

            switch (dtype)
            {
            case execution_tree::node_data_type_bool:
                return statisticsnd_flat<Op>(
                    execution_tree::extract_boolean_value_strict(
                        std::move(arg), name, codename),
                    keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            case execution_tree::node_data_type_int64:
                return statisticsnd_flat<Op>(
                    execution_tree::extract_integer_value_strict(
                        std::move(arg), name, codename),
                    keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            case execution_tree::node_data_type_unknown:
                HPX_FALLTHROUGH;
            case execution_tree::node_data_type_double:
                return statisticsnd_flat<Op>(
                    execution_tree::extract_numeric_value(
                        std::move(arg), name, codename),
                    keepdims, std::move(initial), name, codename,
                    std::move(ctx));

            default:
                break;
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd_flat",
                util::generate_error_message(
                    "the statistics primitive requires for all arguments "
                    "to be numeric data types",
                    name, codename, ctx.back_trace()));
        }

        ///////////////////////////////////////////////////////////////////////
        inline void verify_axis(std::int64_t axis, std::int64_t min_axis,
            std::int64_t max_axis, char const* type, std::string const& name,
            std::string const& codename, execution_tree::eval_context const& ctx)
        {
            if (axis < min_axis || axis > max_axis)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::verify_axis",
                    util::generate_error_message(
                        hpx::util::format(
                            "the statistics_operation primitive requires "
                            "operand "
                            "axis to be between {} and {} for {}.",
                            min_axis, max_axis, type),
                        name, codename, ctx.back_trace()));
            }
        }
    }    // namespace detail

    template <template <class T> class Op>
    execution_tree::primitive_argument_type statisticsnd(
        execution_tree::primitive_argument_type&& arg, ir::range&& axes,
        bool keepdims, execution_tree::primitive_argument_type&& initial,
        execution_tree::node_data_type dtype, std::string const& name,
        std::string const& codename, execution_tree::eval_context ctx)
    {
        std::size_t dims = execution_tree::extract_numeric_value_dimension(
            arg, name, codename);

        switch (axes.size())
        {
        case 0:
            // empty list given, we have element-wise operation
            return detail::statisticsnd<Op>(std::move(arg), std::move(initial),
                dtype, name, codename, std::move(ctx));

        case 1:
            // one axis specified, requires at least 1D data to work with
            if (dims >= 1)
            {
                std::int64_t axis =
                    execution_tree::extract_scalar_integer_value_strict(
                        *axes.begin(), name, codename);
                return statisticsnd<Op>(std::move(arg),
                    hpx::util::optional<std::int64_t>(axis), keepdims,
                    std::move(initial), dtype, name, codename, std::move(ctx));
            }
            break;

        case 2:
            // two axes specified, requires at least 2D data to work with
            {
                auto it = axes.begin();
                std::int64_t axis0 =
                    execution_tree::extract_scalar_integer_value_strict(
                        *it, name, codename);
                std::int64_t axis1 =
                    execution_tree::extract_scalar_integer_value_strict(
                        *++it, name, codename);
                axis0 = axis0 < 0 ? axis0 + dims : axis0;
                axis1 = axis1 < 0 ? axis1 + dims : axis1;
                if (axis0 == axis1)
                {
                    // axes must be unique
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "common::statisticsnd",
                        util::generate_error_message(
                            "the statistics primitive requires for all axis "
                            "arguments to be unique",
                            name, codename, ctx.back_trace()));
                }

                if (dims == 2)
                {
                    detail::verify_axis(
                        axis0, 0, 1, "matrices", name, codename, ctx);
                    detail::verify_axis(
                        axis1, 0, 1, "matrices", name, codename, ctx);

                    return detail::statisticsnd_flat<Op>(std::move(arg),
                        keepdims, std::move(initial), dtype, name, codename,
                        std::move(ctx));
                }
                else if (dims == 3)
                {
                    detail::verify_axis(
                        axis0, 0, 2, "tensors", name, codename, ctx);
                    detail::verify_axis(
                        axis1, 0, 2, "tensors", name, codename, ctx);

                    return detail::statistics3d_slice<Op>(std::move(arg), axis0,
                        axis1, keepdims, std::move(initial), dtype, name,
                        codename, std::move(ctx));
                }
                else if (dims == 4)
                {
                    detail::verify_axis(
                        axis0, 0, 3, "4d arrays", name, codename, ctx);
                    detail::verify_axis(
                        axis1, 0, 3, "4d arrays", name, codename, ctx);

                    return detail::statistics4d_slice<Op>(std::move(arg), axis0,
                        axis1, keepdims, std::move(initial), dtype, name,
                        codename, std::move(ctx));
                }
            }
            break;

        case 3:
            // three axes specified, requires at least 3D data to work with
            {
                auto it = axes.begin();
                std::int64_t axis0 =
                    execution_tree::extract_scalar_integer_value_strict(
                        *it, name, codename);
                std::int64_t axis1 =
                    execution_tree::extract_scalar_integer_value_strict(
                        *++it, name, codename);
                std::int64_t axis2 =
                    execution_tree::extract_scalar_integer_value_strict(
                        *++it, name, codename);
                axis0 = axis0 < 0 ? axis0 + dims : axis0;
                axis1 = axis1 < 0 ? axis1 + dims : axis1;
                axis2 = axis2 < 0 ? axis2 + dims : axis2;
                if (axis0 == axis1 || axis0 == axis2 || axis1 == axis2)
                {
                    // axes must be unique
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "common::statisticsnd",
                        util::generate_error_message(
                            "the statistics primitive requires for all axis "
                            "arguments to be unique",
                            name, codename, ctx.back_trace()));
                }

                if (dims == 3)
                {
                    detail::verify_axis(
                        axis0, 0, 2, "tensors", name, codename, ctx);
                    detail::verify_axis(
                        axis1, 0, 2, "tensors", name, codename, ctx);
                    detail::verify_axis(
                        axis2, 0, 2, "tensors", name, codename, ctx);

                    return detail::statisticsnd_flat<Op>(std::move(arg),
                        keepdims, std::move(initial), dtype, name, codename,
                        std::move(ctx));
                }

                else if (dims == 4)
                {
                    detail::verify_axis(
                        axis0, 0, 3, "4d arrays", name, codename, ctx);
                    detail::verify_axis(
                        axis1, 0, 3, "4d arrays", name, codename, ctx);
                    detail::verify_axis(
                        axis2, 0, 3, "4d arrays", name, codename, ctx);

                    return detail::statistics4d_tensor<Op>(std::move(arg),
                        axis0, axis1, axis2, keepdims, std::move(initial),
                        dtype, name, codename, std::move(ctx));
                }
            }

        case 4: {
            auto it = axes.begin();
            std::int64_t axis0 =
                execution_tree::extract_scalar_integer_value_strict(
                    *it, name, codename);
            std::int64_t axis1 =
                execution_tree::extract_scalar_integer_value_strict(
                    *++it, name, codename);
            std::int64_t axis2 =
                execution_tree::extract_scalar_integer_value_strict(
                    *++it, name, codename);
            std::int64_t axis3 =
                execution_tree::extract_scalar_integer_value_strict(
                    *++it, name, codename);
            axis0 = axis0 < 0 ? axis0 + dims : axis0;
            axis1 = axis1 < 0 ? axis1 + dims : axis1;
            axis2 = axis2 < 0 ? axis2 + dims : axis2;
            axis3 = axis3 < 0 ? axis3 + dims : axis3;
            if (axis0 == axis1 || axis0 == axis2 || axis1 == axis2 ||
                axis0 == axis3 || axis1 == axis3 || axis2 == axis3)
            {
                // axes must be unique
                HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd",
                    util::generate_error_message(
                        "the statistics primitive requires for all axis "
                        "arguments to be unique",
                        name, codename, ctx.back_trace()));
            }

            detail::verify_axis(axis0, 0, 3, "4d arrays", name, codename, ctx);
            detail::verify_axis(axis1, 0, 3, "4d arrays", name, codename, ctx);
            detail::verify_axis(axis2, 0, 3, "4d arrays", name, codename, ctx);
            detail::verify_axis(axis3, 0, 3, "4d arrays", name, codename, ctx);

            return detail::statisticsnd_flat<Op>(std::move(arg), keepdims,
                std::move(initial), dtype, name, codename, std::move(ctx));
        }
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "common::statisticsnd",
            util::generate_error_message(
                hpx::util::format(
                    "invalid number of axis specified ({}), "
                    "should be not larger than the data's dimension ({})",
                    axes.size(), dims),
                name, codename, ctx.back_trace()));
    }

}}    // namespace phylanx::common

#endif
