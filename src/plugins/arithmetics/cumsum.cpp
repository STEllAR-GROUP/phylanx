//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/arithmetics/cumsum.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/parallel_scan.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/format.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cumsum::match_data =
    {
        match_pattern_type{
            "cumsum",
            std::vector<std::string>{
                "cumsum(_1)", "cumsum(_1, _2)"
            },
            &create_cumsum, &create_primitive<cumsum>, R"(
            a, axis
            Args:

                a (array_like) : input array
                axis (int, optional) : Axis along which the cumulative sum is
                    computed. The default (None) is to compute the cumsum over
                    the flattened array.

            Returns:

            Return the cumulative sum of the elements along a given axis.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    cumsum::cumsum(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type cumsum::cumsum0d(primitive_arguments_type&& ops) const
    {
        if (ops.size() == 2)
        {
            // make sure that axis, if given is equal to one
            std::int64_t axis =
                extract_scalar_integer_value(ops[1], name_, codename_);
            if (axis != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumsum::cumsum_helper<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for scalar", axis)));
            }
        }

        ir::node_data<T> value =
            extract_value_scalar<T>(ops[0], name_, codename_);

        blaze::DynamicVector<T> result(1, value.scalar());
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum::cumsum1d(primitive_arguments_type&& ops) const
    {
        if (ops.size() == 2)
        {
            // make sure that axis, if given is equal to one
            std::int64_t axis =
                extract_scalar_integer_value(ops[1], name_, codename_);
            if (axis != 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumsum::cumsum_helper<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for vector", axis)));
            }
        }

        std::array<std::size_t, 2> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_vector<T>(
            std::move(ops[0]), dims[1], name_, codename_);

        auto v = value.vector();
        blaze::DynamicVector<T> result(v.size());

        hpx::parallel::inclusive_scan(
            hpx::parallel::execution::par,
            v.begin(), v.end(), result.begin());

        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type cumsum::cumsum2d_noaxis(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, 2> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_matrix<T>(
            std::move(ops[0]), dims[0], dims[1], name_, codename_);

        auto m = value.matrix();
        blaze::DynamicVector<T> result(m.rows() * m.columns());

        T init = T(0);
        auto last = result.begin();

        for (std::size_t row = 0; row != m.rows(); ++row)
        {
            last = hpx::parallel::inclusive_scan(
                hpx::parallel::execution::par,
                m.begin(row), m.end(row), last,
                std::plus<T>{}, init);

            init = *(last - 1);
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum::cumsum2d_columns(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, 2> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_matrix<T>(
            std::move(ops[0]), dims[0], dims[1], name_, codename_);

        auto m = value.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns());

        for (std::size_t col = 0; col != m.columns(); ++col)
        {
            auto column = blaze::column(m, col);
            auto result_column = blaze::column(result, col);

            hpx::parallel::inclusive_scan(
                hpx::parallel::execution::par,
                column.begin(), column.end(), result_column.begin());
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum::cumsum2d_rows(
        primitive_arguments_type&& ops) const
    {
        std::array<std::size_t, 2> dims =
            extract_numeric_value_dimensions(ops[0], name_, codename_);

        ir::node_data<T> value = extract_value_matrix<T>(
            std::move(ops[0]), dims[0], dims[1], name_, codename_);

        auto m = value.matrix();
        blaze::DynamicMatrix<T> result(m.rows(), m.columns());

        for (std::size_t row = 0; row != m.rows(); ++row)
        {
            hpx::parallel::inclusive_scan(
                hpx::parallel::execution::par,
                m.begin(row), m.end(row), result.begin(row));
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type cumsum::cumsum2d(primitive_arguments_type&& ops) const
    {
        std::int64_t axis = -1;
        if (ops.size() == 2)
        {
            // make sure that axis, if given is equal to one
            axis = extract_scalar_integer_value(ops[1], name_, codename_);
            if (axis != 0 && axis != 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cumsum::cumsum_helper<T>",
                    generate_error_message(hpx::util::format(
                        "axis {:d} is out of bounds for matrix", axis)));
            }
        }

        switch (axis)
        {
        default: HPX_FALLTHROUGH;
        case -1:        // no axis specified
            return cumsum2d_noaxis<T>(std::move(ops));

        case 0:         // cumulative sum over columns
            return cumsum2d_columns<T>(std::move(ops));

        case 1:         // cumulative sum over rows
            return cumsum2d_rows<T>(std::move(ops));
        }
    }

    template <typename T>
    primitive_argument_type cumsum::cumsum_helper(
        primitive_arguments_type&& ops) const
    {
        std::size_t dims =
            extract_numeric_value_dimension(ops[0], name_, codename_);
        switch (dims)
        {
        case 0:
            return cumsum0d<T>(std::move(ops));

        case 1:
            return cumsum1d<T>(std::move(ops));

        case 2:
            return cumsum2d<T>(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "cumsum::cumsum_helper<T>",
                generate_error_message(
                    "target operand has unsupported number of dimensions"));
        }
    }

    hpx::future<primitive_argument_type> cumsum::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "cumsum::eval",
                generate_error_message(
                    "the cumsum primitive requires one or two operands"));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "cumsum::eval",
                generate_error_message(
                    "the cumsum primitive requires that the "
                        "argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& ops)
            ->  primitive_argument_type
            {
                node_data_type t = this_->dtype_;
                if (t == node_data_type_unknown)
                {
                    t = extract_common_type(ops[0]);
                }

                switch (t)
                {
                case node_data_type_bool:
                    return this_->cumsum_helper<std::uint8_t>(std::move(ops));

                case node_data_type_int64:
                    return this_->cumsum_helper<std::int64_t>(std::move(ops));

                case node_data_type_unknown: HPX_FALLTHROUGH;
                case node_data_type_double:
                    return this_->cumsum_helper<double>(std::move(ops));

                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter, "cumsum::eval",
                    this_->generate_error_message(
                        "target operand has unsupported type"));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
