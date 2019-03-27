//   Copyright (c) 2017-2019 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_CUMULATIVE_IMPL_JAN_22_2019_0345PM)
#define PHYLANX_PRIMITIVES_CUMULATIVE_IMPL_JAN_22_2019_0345PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/arithmetics/cumulative.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/parallel_scan.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/format.hpp>
#include <hpx/util/optional.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
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
    cumulative<Op, Derived>::cumulative(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative0d(
        primitive_arguments_type&& ops,
        hpx::util::optional<std::int64_t>&& axis) const
    {
        if (axis)
        {
            // make sure that axis, if given is equal to zero
            if (*axis != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumulative<Op, Derived>::cumulative0d<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for scalar", *axis)));
            }
        }

        ir::node_data<T> value =
            extract_value_scalar<T>(ops[0], name_, codename_);

        blaze::DynamicVector<T> result(1, value.scalar());
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative1d(
        primitive_arguments_type&& ops,
        hpx::util::optional<std::int64_t>&& axis) const
    {
        if (axis)
        {
            // make sure that axis, if given is equal to zero
            if (*axis != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumulative<Op, Derived>::cumulative1d<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for vector", *axis)));
            }
        }

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_vector<T>(
            std::move(ops[0]), dims[0], name_, codename_);

        auto v = value.vector();
        blaze::DynamicVector<T> result(v.size());

        Op{}(v.begin(), v.end(), result.begin(), Op::template initial<T>());

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative2d_noaxis(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_matrix<T>(
            std::move(ops[0]), dims[0], dims[1], name_, codename_);

        auto m = value.matrix();
        blaze::DynamicVector<T> result(m.rows() * m.columns());

        T init = Op::template initial<T>();
        auto last = result.begin();

        for (std::size_t row = 0; row != m.rows(); ++row)
        {
            last = Op{}(m.begin(row), m.end(row), last, init);
            init = *(last - 1);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative2d_columns(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_matrix<T>(
            std::move(ops[0]), dims[0], dims[1], name_, codename_);

        auto m = value.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns());

        T init = Op::template initial<T>();
        for (std::size_t col = 0; col != m.columns(); ++col)
        {
            auto column = blaze::column(m, col);
            auto result_column = blaze::column(result, col);

            Op{}(column.begin(), column.end(), result_column.begin(), init);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative2d_rows(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_matrix<T>(
            std::move(ops[0]), dims[0], dims[1], name_, codename_);

        auto m = value.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns());

        T init = Op::template initial<T>();
        for (std::size_t row = 0; row != m.rows(); ++row)
        {
            Op{}(m.begin(row), m.end(row), result.begin(row), init);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative2d(
        primitive_arguments_type&& ops,
        hpx::util::optional<std::int64_t>&& axis) const
    {
        if (axis)
        {
            // make sure that axis, if given, is in valid range
            if (*axis < -2 || *axis > 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumulative<Op, Derived>::cumulative2d<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for matrix", *axis)));
            }

            switch (*axis)
            {
            case -2:
            case 0:         // cumulative sum over columns
                return cumulative2d_columns<T>(std::move(ops));

            case -1:
            case 1:         // cumulative sum over rows
                return cumulative2d_rows<T>(std::move(ops));
            }
        }

        // no axis specified
        return cumulative2d_noaxis<T>(std::move(ops));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative3d_noaxis(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_tensor<T>(
            std::move(ops[0]), dims[0], dims[1], dims[2], name_, codename_);

        auto t = value.tensor();
        blaze::DynamicVector<T> result(t.pages() * t.rows() * t.columns());

        T init = Op::template initial<T>();
        auto last = result.begin();

        for (std::size_t page = 0; page != t.pages(); ++page)
        {
            auto ps = blaze::pageslice(t, page);
            for (std::size_t row = 0; row != t.rows(); ++row)
            {
                last = Op{}(ps.begin(row), ps.end(row), last, init);
                init = *(last - 1);
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative3d_pages(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_tensor<T>(
            std::move(ops[0]), dims[0], dims[1], dims[2], name_, codename_);

        auto t = value.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
        T init = Op::template initial<T>();
        for (std::size_t i = 0; i != t.rows(); ++i)
        {
            auto slice = blaze::rowslice(t, i);
            auto result_slice = blaze::rowslice(result, i);
            for (std::size_t j = 0; j != blaze::rows(slice); ++j)
            {
                auto row = blaze::row(slice, j);
                auto result_row = blaze::row(result_slice, j);

                Op{}(row.begin(), row.end(), result_row.begin(), init);
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative3d_columns(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_tensor<T>(
            std::move(ops[0]), dims[0], dims[1], dims[2], name_, codename_);

        auto t = value.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
        T init = Op::template initial<T>();
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            auto result_slice = blaze::pageslice(result, k);
            for (std::size_t j = 0; j != blaze::columns(slice); ++j)
            {
                auto col = blaze::column(slice, j);
                auto result_col = blaze::column(result_slice, j);

                Op{}(col.begin(), col.end(), result_col.begin(), init);
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative3d_rows(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_tensor<T>(
            std::move(ops[0]), dims[0], dims[1], dims[2], name_, codename_);

        auto t = value.tensor();

        blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
        T init = Op::template initial<T>();
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            auto result_slice = blaze::pageslice(result, k);
            for (std::size_t j = 0; j != blaze::rows(slice); ++j)
            {
                auto row = blaze::row(slice, j);
                auto result_row = blaze::row(result_slice, j);

                Op{}(row.begin(), row.end(), result_row.begin(), init);
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative3d(
        primitive_arguments_type&& ops,
        hpx::util::optional<std::int64_t>&& axis) const
    {
        if (axis)
        {
            // make sure that axis, if given, is in valid range
            if (*axis < -3 || *axis > 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumulative<Op, Derived>::cumulative3d<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for tensor", *axis)));
            }

            switch (*axis)
            {
            case -3:
            case 0:         // cumulative sum over pages
                return cumulative3d_pages<T>(std::move(ops));

            case -2:
            case 1:         // cumulative sum over columns
                return cumulative3d_columns<T>(std::move(ops));

            case -1:
            case 2:         // cumulative sum over rows
                return cumulative3d_rows<T>(std::move(ops));
            }
        }

        // no axis specified
        return cumulative3d_noaxis<T>(std::move(ops));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type cumulative<Op, Derived>::cumulative_helper(
        primitive_arguments_type&& ops,
        hpx::util::optional<std::int64_t>&& axis) const
    {
        std::size_t dims =
            extract_numeric_value_dimension(ops[0], name_, codename_);

        switch (dims)
        {
        case 0:
            return cumulative0d<T>(std::move(ops), std::move(axis));

        case 1:
            return cumulative1d<T>(std::move(ops), std::move(axis));

        case 2:
            return cumulative2d<T>(std::move(ops), std::move(axis));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return cumulative3d<T>(std::move(ops), std::move(axis));
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cumulative<Op, Derived>::cumulative_helper<T>",
                generate_error_message(
                    "target operand has unsupported number of dimensions"));
        }
    }

    template <typename Op, typename Derived>
    hpx::future<primitive_argument_type> cumulative<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cumulative<Op, Derived>::eval",
                generate_error_message(
                    "the cumulative primitive requires one, two, or three "
                    "operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cumulative<Op, Derived>::eval",
                generate_error_message(
                    "the cumulative primitive requires that the "
                        "argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& ops)
            ->  primitive_argument_type
            {
                hpx::util::optional<std::int64_t> axis;
                if (ops.size() >= 2 && valid(ops[1]))
                {
                    axis = extract_scalar_integer_value(
                        std::move(ops[1]), this_->name_, this_->codename_);
                }

                node_data_type t = node_data_type_unknown;
                if (ops.size() >= 3 && valid(ops[2]))
                {
                    t = map_dtype(extract_string_value(
                        std::move(ops[2]), this_->name_, this_->codename_));
                }

                if (t == node_data_type_unknown)
                {
                    t = extract_common_type(ops[0]);
                }

                switch (t)
                {
                case node_data_type_bool:
                    return this_->template cumulative_helper<std::uint8_t>(
                        std::move(ops), std::move(axis));

                case node_data_type_int64:
                    return this_->template cumulative_helper<std::int64_t>(
                        std::move(ops), std::move(axis));

                case node_data_type_unknown: HPX_FALLTHROUGH;
                case node_data_type_double:
                    return this_->template cumulative_helper<double>(
                        std::move(ops), std::move(axis));

                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumulative<Op, Derived>::eval",
                    this_->generate_error_message(
                        "target operand has unsupported type"));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

#endif
