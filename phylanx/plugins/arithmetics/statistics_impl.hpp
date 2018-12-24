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
#include <phylanx/plugins/arithmetics/statistics.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
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
    template <typename Op, typename Derived>
    statistics<Op, Derived>::statistics(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics0d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> axis,
        bool keep_dims) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statistics0d",
                generate_error_message(
                    "the statistics_operation primitive requires operand axis "
                    "to be either 0 or -1 for scalar values."));
        }

        return primitive_argument_type{arg.scalar()};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics1d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> axis,
        bool keep_dims) const
    {
        if (axis && axis.value() != 0 && axis.value() != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::statistics1d",
                generate_error_message(
                    "the statistics_operation primitive requires operand axis "
                    "to be either 0 or -1 for vectors."));
        }

        auto v = arg.vector();
        T result = Op{}(v.begin(), v.end(), Op::template initial<T>());

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicVector<T>(1, result)};
        }

        return primitive_argument_type{result};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d_flat(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();

        T result = Op::template initial<T>();
        for (std::size_t i = 0; i != m.rows(); ++i)
        {
            auto row = blaze::row(m, i);
            result = Op{}(row.begin(), row.end(), result);
        }

        if (keep_dims)
        {
            return primitive_argument_type{blaze::DynamicMatrix<T>(1, 1, result)};
        }

        return primitive_argument_type{result};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d_axis0(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();

        if (keep_dims)
        {
            blaze::DynamicMatrix<T> result(1, m.columns());
            for (std::size_t i = 0; i != m.columns(); ++i)
            {
                auto col = blaze::column(m, i);
                result(1, i) =
                    Op{}(col.begin(), col.end(), Op::template initial<T>());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(m.columns());
        for (std::size_t i = 0; i != m.columns(); ++i)
        {
            auto col = blaze::column(m, i);
            result[i] = Op{}(col.begin(), col.end(), Op::template initial<T>());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d_axis1(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto m = arg.matrix();

        if (keep_dims)
        {
            blaze::DynamicMatrix<T> result(m.rows(), 1);
            for (std::size_t i = 0; i != m.rows(); ++i)
            {
                auto row = blaze::row(m, i);
                result(i, 0) =
                    Op{}(row.begin(), row.end(), Op::template initial<T>());
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicVector<T> result(m.rows());
        for (std::size_t i = 0; i != m.rows(); ++i)
        {
            auto row = blaze::row(m, i);
            result[i] = Op{}(row.begin(), row.end(), Op::template initial<T>());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics2d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> axis,
        bool keep_dims) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -2: HPX_FALLTHROUGH;
            case 0:
                return statistics2d_axis0(std::move(arg), keep_dims);

            case -1: HPX_FALLTHROUGH;
            case 1:
                return statistics2d_axis1(std::move(arg), keep_dims);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::statistics2d",
                    generate_error_message(
                        "the statistics_operation primitive requires operand "
                        "axis to be between -2 and 1 for matrices."));
            }
        }
        return statistics2d_flat(std::move(arg), keep_dims);
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_flat(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto t = arg.tensor();

        if (keep_dims)
        {
            return primitive_argument_type{
                blaze::DynamicTensor<T>(1, 1, 1, result)};
        }

        T result = Op::template initial<T>();
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto page = blaze::pageslice(t, k);
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto row = blaze::row(page, i);
                result = Op{}(row.begin(), row.end(), result);
            }
        }

        return primitive_argument_type{result};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis0(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto t = arg.tensor();

        if (keep_dims)
        {
            blaze::DynamicTensor<T> result(1, t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    auto row = blaze::row(slice, j);
                    result(0, i, j) =
                        Op{}(row.begin(), row.end(), Op::template initial<T>());
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
                auto row = blaze::row(slice, j);
                result(i, j) =
                    Op{}(row.begin(), row.end(), Op::template initial<T>());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis1(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto t = arg.tensor();

        if (keep_dims)
        {
            blaze::DynamicTensor<T> result(t.columns(), 1, t.pages());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    auto col = blaze::column(slice, j);
                    result(j, 0, k) =
                        Op{}(col.begin(), col.end(), Op::template initial<T>());
                }
            }

            return primitive_argument_type{std::move(result)};
        }

        blaze::DynamicMatrix<T> result(t.columns(), t.pages());
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            for (std::size_t j = 0; j != t.columns(); ++j)
            {
                auto col = blaze::column(slice, j);
                result(j, k) =
                    Op{}(col.begin(), col.end(), Op::template initial<T>());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d_axis2(
        arg_type<T>&& arg, bool keep_dims) const
    {
        auto t = arg.tensor();

        if (keep_dims)
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), 1);
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t i = 0; i != t.rows(); ++i)
                {
                    auto row = blaze::row(slice, k);
                    result(k, i, 0) =
                        Op{}(row.begin(), row.end(), Op::template initial<T>());
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
                auto row = blaze::row(slice, k);
                result(k, i) =
                    Op{}(row.begin(), row.end(), Op::template initial<T>());
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statistics3d(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> axis,
        bool keep_dims) const
    {
        if (axis)
        {
            switch (axis.value())
            {
            case -3: HPX_FALLTHROUGH;
            case 0:
                return statistics3d_axis0(std::move(arg), keep_dims);

            case -2: HPX_FALLTHROUGH;
            case 1:
                return statistics3d_axis1(std::move(arg), keep_dims);

            case -1: HPX_FALLTHROUGH;
            case 2:
                return statistics3d_axis2(std::move(arg), keep_dims);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::statistics3d",
                    generate_error_message(
                        "the statistics_operation primitive requires operand "
                        "axis to be between -3 and 2 for tensors."));
            }
        }
        return statistics2d_flat(std::move(arg), keep_dims);
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type statistics<Op, Derived>::statisticsnd(
        arg_type<T>&& arg, hpx::util::optional<std::int64_t> axis,
        bool keep_dims) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            return statistics0d(std::move(arg), axis, keep_dims);

        case 1:
            return statistics1d(std::move(arg), axis, keep_dims);

        case 2:
            return statistics2d(std::move(arg), axis, keep_dims);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return statistics3d(std::move(arg), axis, keep_dims);
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::eval",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    hpx::future<primitive_argument_type> statistics<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "statistics::eval",
                generate_error_message(
                    "the statistics primitive requires exactly one, two, or "
                        "three operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::eval",
                    generate_error_message(
                        "the statistics_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                // Extract axis and keep_dims
                // Presence of axis changes behavior for >1d cases
                hpx::util::optional<std::int64_t> axis;
                bool keep_dims = false;

                // axis is argument #2
                if (args.size() > 1)
                {
                    if (valid(args[1]))
                    {
                        axis = extract_scalar_integer_value(
                            args[1], this_->name_, this_->codename_);
                    }

                    // keep_dims is argument #3
                    if (args.size() == 3)
                    {
                        keep_dims = extract_scalar_boolean_value(
                            args[2], this_->name_, this_->codename_);
                    }
                }

                node_data_type t = this_->dtype_;
                if (t == node_data_type_unknown)
                {
                    t = extract_common_type(args[0]);
                }

                switch (t)
                {
                case node_data_type_bool:
                    return this_->statisticsnd(
                        extract_boolean_value_strict(std::move(args[0]),
                            this_->name_, this_->codename_),
                        axis, keep_dims);

                case node_data_type_int64:
                    return this_->statisticsnd(
                        extract_integer_value_strict(std::move(args[0]),
                            this_->name_, this_->codename_),
                        axis, keep_dims);

                case node_data_type_unknown: HPX_FALLTHROUGH;
                case node_data_type_double:
                    return this_->statisticsnd(
                        extract_numeric_value(std::move(args[0]),
                            this_->name_, this_->codename_),
                        axis, keep_dims);

                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "statistics::eval",
                    this_->generate_error_message(
                        "the statistics primitive requires for all arguments "
                        "to be numeric data types"));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

#endif
