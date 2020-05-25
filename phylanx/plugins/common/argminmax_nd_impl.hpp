// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_COMMON_ARGMINMAX_IMPL_2020_MAY_21_0326PM)
#define PHYLANX_COMMON_ARGMINMAX_IMPL_2020_MAY_21_0326PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/argminmax_nd.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/tensor_iterators.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace common {

    namespace detail {

        template <typename Op, typename T>
        execution_tree::primitive_argument_type argminmax0d(std::size_t numargs,
            ir::node_data<T>&& arg, std::int64_t axis, std::string const& name,
            std::string const& codename)
        {
            // `axis` is optional
            if (numargs == 2)
            {
                // `axis` can only be -1 or 0
                if (axis < -1 || axis > 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::common::argminmax0d<Operation>",
                        util::generate_error_message(
                            "operand axis can only between -1 and 0 for "
                            "an a operand that is 0d",
                            name, codename));
                }
            }
            return execution_tree::primitive_argument_type(std::int64_t(0));
        }
    }    // namespace detail

    template <typename Operation>
    execution_tree::primitive_argument_type argminmax0d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename)
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = execution_tree::extract_scalar_integer_value(
                std::move(args[1]), name, codename);
        }

        switch (execution_tree::extract_common_type(args[0]))
        {
        case execution_tree::node_data_type_bool:
            return detail::argminmax0d<Operation>(numargs,
                execution_tree::extract_boolean_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_int64:
            return detail::argminmax0d<Operation>(numargs,
                execution_tree::extract_integer_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_double:
            return detail::argminmax0d<Operation>(numargs,
                execution_tree::extract_numeric_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_unknown:
            return detail::argminmax0d<Operation>(numargs,
                execution_tree::extract_numeric_value(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::common::argminmax0d<Operation>",
            util::generate_error_message(
                "the argminmax primitive requires for all arguments to "
                "be numeric data types",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax1d(std::size_t numargs,
            ir::node_data<T>&& arg, std::int64_t axis,
            execution_tree::primitive_argument_type* value,
            std::string const& name, std::string const& codename)
        {
            // `axis` is optional
            if (numargs == 2)
            {
                // `axis` can only be -1 or 0
                if (axis < -1 || axis > 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::common::argminmax1d<Operation>",
                        util::generate_error_message(
                            "operand axis can only between -1 and 0 for "
                            "an a operand that is 1d",
                            name, codename));
                }
            }

            // a should not be empty
            auto a = arg.vector();
            if (a.size() == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argminmax<Op, Derived>::argminmax1d",
                    util::generate_error_message(
                        "attempt to get argminmax of an empty sequence", name,
                        codename));
            }

            // Find the maximum value among the elements
            auto it = Operation{}(a.begin(), a.end());

            // Return min/max's value (if requested)
            if (value != nullptr)
            {
                *value = execution_tree::primitive_argument_type{*it};
            }

            // Return min/max's index
            return execution_tree::primitive_argument_type(
                (std::int64_t)(std::distance(a.begin(), it)));
        }
    }    // namespace detail

    template <typename Operation>
    execution_tree::primitive_argument_type argminmax1d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename,
        execution_tree::primitive_argument_type* value)
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = execution_tree::extract_scalar_integer_value(
                std::move(args[1]), name, codename);
        }

        switch (execution_tree::extract_common_type(args[0]))
        {
        case execution_tree::node_data_type_bool:
            return detail::argminmax1d<Operation>(numargs,
                execution_tree::extract_boolean_value_strict(
                    std::move(args[0]), name, codename),
                axis, value, name, codename);

        case execution_tree::node_data_type_int64:
            return detail::argminmax1d<Operation>(numargs,
                execution_tree::extract_integer_value_strict(
                    std::move(args[0]), name, codename),
                axis, value, name, codename);

        case execution_tree::node_data_type_double:
            return detail::argminmax1d<Operation>(numargs,
                execution_tree::extract_numeric_value_strict(
                    std::move(args[0]), name, codename),
                axis, value, name, codename);

        case execution_tree::node_data_type_unknown:
            return detail::argminmax1d<Operation>(numargs,
                execution_tree::extract_numeric_value(
                    std::move(args[0]), name, codename),
                axis, value, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "argminmax<Op, Derived>::argminmax1d",
            util::generate_error_message(
                "the arange primitive requires for all arguments to "
                "be numeric data types",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax2d_flatten(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            using phylanx::util::matrix_row_iterator;

            auto a = arg.matrix();

            matrix_row_iterator<decltype(a)> const a_begin(a);
            matrix_row_iterator<decltype(a)> const a_end(a, a.rows());

            T global_minmax = Operation::template initial<T>();
            std::size_t global_index = 0ul;
            std::size_t passed_rows = 0ul;
            for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
            {
                auto local_minmax = Operation{}(it->begin(), it->end());
                auto local_minmax_val = *local_minmax;

                if (Operation::compare(local_minmax_val, global_minmax))
                {
                    global_minmax = local_minmax_val;
                    global_index = std::distance(it->begin(), local_minmax) +
                        passed_rows * it->size();
                }
            }

            return execution_tree::primitive_argument_type(
                std::int64_t(global_index));
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax2d_0_axis(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            auto a = arg.matrix();

            using phylanx::util::matrix_column_iterator;
            matrix_column_iterator<decltype(a)> a_begin(a);
            matrix_column_iterator<decltype(a)> a_end(a, a.columns());

            blaze::DynamicVector<std::int64_t> result(a.columns());
            auto result_it = result.begin();
            for (auto it = a_begin; it != a_end; ++it, ++result_it)
            {
                auto local_minmax = Operation{}(it->begin(), it->end());
                *result_it = std::distance(it->begin(), local_minmax);
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax2d_1_axis(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            auto a = arg.matrix();

            using phylanx::util::matrix_row_iterator;
            matrix_row_iterator<decltype(a)> a_begin(a);
            matrix_row_iterator<decltype(a)> a_end(a, a.rows());

            blaze::DynamicVector<std::int64_t> result(a.rows());
            auto result_it = result.begin();
            for (auto it = a_begin; it != a_end; ++it, ++result_it)
            {
                auto local_minmax = Operation{}(it->begin(), it->end());
                *result_it = std::distance(it->begin(), local_minmax);
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax2d(std::size_t numargs,
            ir::node_data<T>&& arg, std::int64_t axis, std::string const& name,
            std::string const& codename)
        {
            // a should not be empty
            auto m = arg.matrix();
            if (m.rows() == 0 || m.columns() == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::common::argminmax2d<Operation>",
                    util::generate_error_message(
                        "attempt to get argminmax of an empty sequence", name,
                        codename));
            }

            // `axis` is optional
            if (numargs == 1)
            {
                // Option 1: Flatten and find min/max
                return argminmax2d_flatten<Operation>(
                    std::move(arg), name, codename);
            }

            // `axis` can only be -2, -1, 0, or 1
            if (axis < -2 || axis > 1)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::common::argminmax2d<Operation>",
                    util::generate_error_message(
                        "operand axis can only between -2 and 1 for an an "
                        "operand that is 2d",
                        name, codename));
            }

            switch (axis)
            {
                // Option 2: Find min/max among rows
            case -2:
                HPX_FALLTHROUGH;
            case 0:
                return argminmax2d_0_axis<Operation>(
                    std::move(arg), name, codename);

                // Option 3: Find min/max among columns
            case -1:
                HPX_FALLTHROUGH;
            case 1:
                return argminmax2d_1_axis<Operation>(
                    std::move(arg), name, codename);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::common::argminmax2d<Operation>",
                    util::generate_error_message(
                        "operand has an invalid value for the axis parameter",
                        name, codename));
            }
        }
    }    // namespace detail

    template <typename Operation>
    execution_tree::primitive_argument_type argminmax2d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename)
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = execution_tree::extract_scalar_integer_value(
                std::move(args[1]), name, codename);
        }

        switch (execution_tree::extract_common_type(args[0]))
        {
        case execution_tree::node_data_type_bool:
            return detail::argminmax2d<Operation>(numargs,
                execution_tree::extract_boolean_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_int64:
            return detail::argminmax2d<Operation>(numargs,
                execution_tree::extract_integer_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_double:
            return detail::argminmax2d<Operation>(numargs,
                execution_tree::extract_numeric_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_unknown:
            return detail::argminmax2d<Operation>(numargs,
                execution_tree::extract_numeric_value(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
            "phylanx::common::argminmax2d<Operation>",
            util::generate_error_message(
                "the arange primitive requires for all arguments to "
                "be numeric data types",
                name, codename));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail {

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax3d_flatten(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            using phylanx::util::matrix_row_iterator;

            auto a = arg.tensor();

            T global_minmax = Operation::template initial<T>();
            std::size_t global_index = 0ul;
            std::size_t passed_pages = 0ul;

            std::size_t rows = blaze::rows(a);
            std::size_t columns = blaze::columns(a);

            for (std::size_t page = 0; page != blaze::pages(a);
                 ++page, ++passed_pages)
            {
                auto ps = blaze::pageslice(a, page);

                matrix_row_iterator<decltype(ps)> const a_begin(ps);
                matrix_row_iterator<decltype(ps)> const a_end(ps, rows);

                std::size_t passed_rows = 0ul;
                for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
                {
                    auto local_minmax = Operation{}(it->begin(), it->end());
                    auto local_minmax_val = *local_minmax;

                    if (Operation::compare(local_minmax_val, global_minmax))
                    {
                        global_minmax = local_minmax_val;
                        global_index =
                            std::distance(it->begin(), local_minmax) +
                            passed_pages * rows + passed_rows * columns;
                    }
                }
            }

            return execution_tree::primitive_argument_type(
                std::int64_t(global_index));
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax3d_0_axis(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            auto t = arg.tensor();

            blaze::DynamicMatrix<std::int64_t> result(t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    auto row = blaze::row(slice, j);
                    result(i, j) = std::distance(
                        row.begin(), Operation{}(row.begin(), row.end()));
                }
            }
            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax3d_1_axis(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            auto t = arg.tensor();

            blaze::DynamicMatrix<std::int64_t> result(t.pages(), t.columns());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t j = 0; j != t.columns(); ++j)
                {
                    auto col = blaze::column(slice, j);
                    result(k, j) = std::distance(
                        col.begin(), Operation{}(col.begin(), col.end()));
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax3d_2_axis(
            ir::node_data<T>&& arg, std::string const& name,
            std::string const& codename)
        {
            auto t = arg.tensor();

            blaze::DynamicMatrix<std::int64_t> result(t.pages(), t.rows());
            for (std::size_t k = 0; k != t.pages(); ++k)
            {
                auto slice = blaze::pageslice(t, k);
                for (std::size_t i = 0; i != t.rows(); ++i)
                {
                    auto row = blaze::row(slice, i);
                    result(k, i) = std::distance(
                        row.begin(), Operation{}(row.begin(), row.end()));
                }
            }

            return execution_tree::primitive_argument_type{std::move(result)};
        }

        template <typename Operation, typename T>
        execution_tree::primitive_argument_type argminmax3d(std::size_t numargs,
            ir::node_data<T>&& arg, std::int64_t axis, std::string const& name,
            std::string const& codename)
        {
            // a should not be empty
            auto t = arg.tensor();
            if (t.pages() == 0 || t.rows() == 0 || t.columns() == 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::common::argminmax3d<Operation>",
                    util::generate_error_message(
                        "attempt to get argminmax of an empty sequence", name,
                        codename));
            }

            // `axis` is optional
            if (numargs == 1)
            {
                // Option 1: Flatten and find min/max
                return argminmax3d_flatten<Operation>(
                    std::move(arg), name, codename);
            }

            // `axis` can only be -3, -2, -1, 0, 1, or 2
            if (axis < -3 || axis > 2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::common::argminmax3d<Operation>",
                    util::generate_error_message(
                        "operand axis can only between -3 and 2 for an an "
                        "operand that is 3d",
                        name, codename));
            }

            switch (axis)
            {
                // Option 1: Find min/max among pages
            case -3:
                HPX_FALLTHROUGH;
            case 0:
                return argminmax3d_0_axis<Operation>(
                    std::move(arg), name, codename);

                // Option 2: Find min/max among rows
            case -2:
                HPX_FALLTHROUGH;
            case 1:
                return argminmax3d_1_axis<Operation>(
                    std::move(arg), name, codename);

                // Option 3: Find min/max among columns
            case -1:
                HPX_FALLTHROUGH;
            case 2:
                return argminmax3d_2_axis<Operation>(
                    std::move(arg), name, codename);

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::common::argminmax3d<Operation>",
                    util::generate_error_message(
                        "operand has an invalid value for the axis parameter",
                        name, codename));
            }
        }
    }    // namespace detail

    template <typename Operation>
    execution_tree::primitive_argument_type argminmax3d(
        execution_tree::primitive_arguments_type&& args,
        std::string const& name, std::string const& codename)
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = execution_tree::extract_scalar_integer_value(
                std::move(args[1]), name, codename);
        }

        switch (execution_tree::extract_common_type(args[0]))
        {
        case execution_tree::node_data_type_bool:
            return detail::argminmax3d<Operation>(numargs,
                execution_tree::extract_boolean_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_int64:
            return detail::argminmax3d<Operation>(numargs,
                execution_tree::extract_integer_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_double:
            return detail::argminmax3d<Operation>(numargs,
                execution_tree::extract_numeric_value_strict(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        case execution_tree::node_data_type_unknown:
            return detail::argminmax3d<Operation>(numargs,
                execution_tree::extract_numeric_value(
                    std::move(args[0]), name, codename),
                axis, name, codename);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::common::argminmax3d<Operation>",
            util::generate_error_message(
                "the arange primitive requires for all arguments to "
                "be numeric data types",
                name, codename));
    }

}}    // namespace phylanx::common

#endif
