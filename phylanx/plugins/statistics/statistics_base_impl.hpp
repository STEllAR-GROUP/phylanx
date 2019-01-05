// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVE_STATISTICS_IMPL_DEC_24_2018_0342PM)
#define PHYLANX_PRIMITIVE_STATISTICS_IMPL_DEC_24_2018_0342PM

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics0d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<T> const& initial) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statistics0d",
                generate_error_message(
                    "the statistics_operation primitive requires operand axis "
                    "to be either 0 or -1 for scalar values."));
        }

        Op<T> op{name_, codename_};

        T initial_value = Op<T>::initial();
        if (initial)
        {
            initial_value = *initial;
        }

        return primitive_argument_type{
            op(extract_scalar_data<T>(std::move(arg), name_, codename_),
                initial_value)};
    }

    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics1d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<T> const& initial) const
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

        T initial_value = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d_flat(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto m = arg.matrix();

        Op<T> op{name_, codename_};
        std::size_t size = 0;

        T result = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d_axis0(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto m = arg.matrix();

        T initial_value = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d_axis1(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto m = arg.matrix();

        T initial_value = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<T> const& initial) const
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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_flat(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto t = arg.tensor();

        Op<T> op{name_, codename_};

        std::size_t size = 0;

        T result = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis0(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto t = arg.tensor();

        T initial_value = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis1(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto t = arg.tensor();

        T initial_value = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis2(
        arg_type<T>&& arg, bool keepdims,
        hpx::util::optional<T> const& initial) const
    {
        auto t = arg.tensor();

        T initial_value = Op<T>::initial();
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
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, hpx::util::optional<T> const& initial) const
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
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <template <class T> class Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statisticsnd(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> const& axis,
        bool keepdims, primitive_argument_type&& initial) const
    {
        hpx::util::optional<T> initial_value;
        if (valid(initial))
        {
            initial_value =
                extract_scalar_data<T>(std::move(arg), name_, codename_);
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

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return statistics3d(std::move(arg), axis, keepdims, initial_value);
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::eval",
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

    namespace detail
    {
        // This function is a hack helping to map the 3d axes into 2d/1d axes
        // while iterating over the given list of axes (which reduces the
        // dimensionality of the data we work on).
        inline std::int64_t adapt_axis(std::int64_t dim, std::size_t dims,
            std::int64_t prev_axis, std::int64_t axis)
        {
            if (dim == 1)
            {
                if (dims == 2)
                {
                    axis = 0;       // collapse remaining vector
                }
                else
                {
                    HPX_ASSERT(dims == 3);
                    HPX_ASSERT(prev_axis != -1);
                    switch (prev_axis)
                    {
                    case 0:
                        axis -= 3;
                        break;

                    case 1:
                        if (axis == 2)
                        {
                            axis = 1;
                        }
                        else
                        {
                            HPX_ASSERT(axis == 0);
                        }
                        break;

                    case 2:
                        axis -= 2;
                        break;
                    }

                }
            }
            else if (dim == 2)
            {
                axis = 0;       // collapse remaining vector
            }

            return axis;
        }
    }

    template <template <class T> class Op, typename Derived>
    primitive_argument_type statistics<Op, Derived>::statisticsnd(
        primitive_argument_type&& arg, ir::range&& axes, bool keepdims,
        primitive_argument_type&& initial) const
    {
        std::size_t dims =
            extract_numeric_value_dimension(arg, name_, codename_);

        // for a list of axes simply repeat the invocation of statisticsnd
        std::int64_t dim = 0;
        std::int64_t prev_axis = -1;
        std::set<std::int64_t> seen_axes;
        for (auto && elem : axes)
        {
            std::int64_t axis =
                extract_scalar_integer_value(std::move(elem), name_, codename_);
            if (seen_axes.find(axis) != seen_axes.end())
            {
                // axes must be unique
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::statisticsnd",
                    generate_error_message(
                        "the statistics primitive requires for all axis "
                        "arguments to be unique"));
            }
            seen_axes.insert(axis);

            if (keepdims)
            {
                arg = statisticsnd(std::move(arg),
                    hpx::util::optional<std::int64_t>(axis), true,
                    std::move(initial));
            }
            else
            {
                axis = detail::adapt_axis(dim, dims, prev_axis, axis);

                arg = statisticsnd(std::move(arg),
                    hpx::util::optional<std::int64_t>(axis),
                    false, std::move(initial));

                prev_axis = axis;
            }
            ++dim;

            // the initial value has to be used for the first axis only
            initial = primitive_argument_type{};
        }
        return arg;
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
                    if (valid(args[1]))
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
