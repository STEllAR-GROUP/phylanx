// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGMINMAX_IMPL_2019_01_19_1130AM)
#define PHYLANX_PRIMITIVES_ARGMINMAX_IMPL_2019_01_19_1130AM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/argminmax.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/tensor_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>

#include <algorithm>
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
    argminmax<Op, Derived>::argminmax(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax0d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // `axis` is optional
        if (numargs == 2)
        {
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "argminmax<Op, Derived>::argminmax0d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 0d"));
            }
        }
        return primitive_argument_type(std::int64_t(0));
    }

    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax0d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argminmax0d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argminmax0d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argminmax0d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argminmax0d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "argminmax<Op, Derived>::argminmax0d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax1d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // `axis` is optional
        if (numargs == 2)
        {
            // `axis` can only be -1 or 0
            if (axis < -1 || axis > 0)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "phylanx::execution_tree::primitives::"
                        "argminmax<Op, Derived>::argminmax1d",
                    generate_error_message(
                        "operand axis can only between -1 and 0 for "
                        "an a operand that is 1d"));
            }
        }

        // a should not be empty
        auto a = arg.vector();
        if (a.size() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax1d",
                generate_error_message(
                    "attempt to get argminmax of an empty sequence"));
        }

        // Find the maximum value among the elements
        auto max_it = Op{}(a.begin(), a.end());

        // Return min/max's index
        return primitive_argument_type(
            (std::int64_t)(std::distance(a.begin(), max_it)));
    }

    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax1d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argminmax1d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argminmax1d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argminmax1d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argminmax1d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "argminmax<Op, Derived>::argminmax1d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax2d_flatten(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;

        auto a = arg.matrix();

        const matrix_row_iterator<decltype(a)> a_begin(a);
        const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        T global_minmax = Op::template initial<T>();
        std::size_t global_index = 0ul;
        std::size_t passed_rows = 0ul;
        for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
        {
            auto local_minmax = Op{}(it->begin(), it->end());
            auto local_minmax_val = *local_minmax;

            if (Op::compare(local_minmax_val, global_minmax))
            {
                global_minmax = local_minmax_val;
                global_index = std::distance(it->begin(), local_minmax) +
                    passed_rows * it->size();
            }
        }
        return primitive_argument_type(std::int64_t(global_index));
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax2d_0_axis(
        ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_column_iterator;
        matrix_column_iterator<decltype(a)> a_begin(a);
        matrix_column_iterator<decltype(a)> a_end(a, a.columns());

        blaze::DynamicVector<std::int64_t> result(a.columns());
        auto result_it = result.begin();
        for (auto it = a_begin; it != a_end; ++it, ++result_it)
        {
            auto local_minmax = Op{}(it->begin(), it->end());
            *result_it = std::distance(it->begin(), local_minmax);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax2d_1_axis(
        ir::node_data<T>&& arg) const
    {
        auto a = arg.matrix();

        using phylanx::util::matrix_row_iterator;
        matrix_row_iterator<decltype(a)> a_begin(a);
        matrix_row_iterator<decltype(a)> a_end(a, a.rows());

        blaze::DynamicVector<std::int64_t> result(a.rows());
        auto result_it = result.begin();
        for (auto it = a_begin; it != a_end; ++it, ++result_it)
        {
            auto local_minmax = Op{}(it->begin(), it->end());
            *result_it = std::distance(it->begin(), local_minmax);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax2d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // a should not be empty
        auto m = arg.matrix();
        if (m.rows() == 0 || m.columns() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax2d",
                generate_error_message(
                    "attempt to get argminmax of an empty sequence"));
        }

        // `axis` is optional
        if (numargs == 1)
        {
            // Option 1: Flatten and find min/max
            return argminmax2d_flatten(std::move(arg));
        }

        // `axis` can only be -2, -1, 0, or 1
        if (axis < -2 || axis > 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax2d",
                generate_error_message(
                    "operand axis can only between -2 and 1 for an an "
                    "operand that is 2d"));
        }

        switch (axis)
        {
        // Option 2: Find min/max among rows
        case -2: HPX_FALLTHROUGH;
        case 0:
            return argminmax2d_0_axis(std::move(arg));

        // Option 3: Find min/max among columns
        case -1: HPX_FALLTHROUGH;
        case 1:
            return argminmax2d_1_axis(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax2d",
                generate_error_message(
                    "operand has an invalid value for the axis parameter"));
        }
    }

    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax2d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argminmax2d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argminmax2d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argminmax2d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argminmax2d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "argminmax<Op, Derived>::argminmax2d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d_flatten(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;

        auto a = arg.tensor();

        T global_minmax = Op::template initial<T>();
        std::size_t global_index = 0ul;
        std::size_t passed_pages = 0ul;

        std::size_t rows = blaze::rows(a);
        std::size_t columns = blaze::columns(a);

        for (std::size_t page = 0; page != blaze::pages(a); ++page, ++passed_pages)
        {
            auto ps = blaze::pageslice(a, page);

            matrix_row_iterator<decltype(ps)> const a_begin(ps);
            matrix_row_iterator<decltype(ps)> const a_end(ps, rows);

            std::size_t passed_rows = 0ul;
            for (auto it = a_begin; it != a_end; ++it, ++passed_rows)
            {
                auto local_minmax = Op{}(it->begin(), it->end());
                auto local_minmax_val = *local_minmax;

                if (Op::compare(local_minmax_val, global_minmax))
                {
                    global_minmax = local_minmax_val;
                    global_index = std::distance(it->begin(), local_minmax) +
                        passed_pages * rows + passed_rows * columns;
                }
            }
        }

        return primitive_argument_type(std::int64_t(global_index));
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d_0_axis(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();

        blaze::DynamicMatrix<std::int64_t> result(t.rows(), t.columns());
        for (std::size_t i = 0; i != t.rows(); ++i)
        {
            auto slice = blaze::rowslice(t, i);
            for (std::size_t j = 0; j != t.columns(); ++j)
            {
                auto row = blaze::row(slice, j);
                result(i, j) =
                    std::distance(row.begin(), Op{}(row.begin(), row.end()));
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d_1_axis(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();

        blaze::DynamicMatrix<std::int64_t> result(t.pages(), t.columns());
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            for (std::size_t j = 0; j != t.columns(); ++j)
            {
                auto col = blaze::column(slice, j);
                result(k, j) =
                    std::distance(col.begin(), Op{}(col.begin(), col.end()));
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d_2_axis(
        ir::node_data<T>&& arg) const
    {
        auto t = arg.tensor();

        blaze::DynamicMatrix<std::int64_t> result(t.pages(), t.rows());
        for (std::size_t k = 0; k != t.pages(); ++k)
        {
            auto slice = blaze::pageslice(t, k);
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto row = blaze::row(slice, i);
                result(k, i) =
                    std::distance(row.begin(), Op{}(row.begin(), row.end()));
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d(
        std::size_t numargs, ir::node_data<T>&& arg, std::int64_t axis) const
    {
        // a should not be empty
        auto t = arg.tensor();
        if (t.pages() == 0 || t.rows() == 0 || t.columns() == 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax3d",
                generate_error_message(
                    "attempt to get argminmax of an empty sequence"));
        }

        // `axis` is optional
        if (numargs == 1)
        {
            // Option 1: Flatten and find min/max
            return argminmax3d_flatten(std::move(arg));
        }

        // `axis` can only be -3, -2, -1, 0, 1, or 2
        if (axis < -3 || axis > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax3d",
                generate_error_message(
                    "operand axis can only between -3 and 2 for an an "
                    "operand that is 3d"));
        }

        switch (axis)
        {
        // Option 1: Find min/max among pages
        case -3: HPX_FALLTHROUGH;
        case 0:
            return argminmax3d_0_axis(std::move(arg));

        // Option 2: Find min/max among rows
        case -2: HPX_FALLTHROUGH;
        case 1:
            return argminmax3d_1_axis(std::move(arg));

        // Option 3: Find min/max among columns
        case -1: HPX_FALLTHROUGH;
        case 2:
            return argminmax3d_2_axis(std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::argminmax3d",
                generate_error_message(
                    "operand has an invalid value for the axis parameter"));
        }
    }

    template <typename Op, typename Derived>
    primitive_argument_type argminmax<Op, Derived>::argminmax3d(
        primitive_arguments_type&& args) const
    {
        std::int64_t axis = -1;
        std::size_t numargs = args.size();
        if (numargs == 2)
        {
            axis = extract_scalar_integer_value(args[1], name_, codename_);
        }

        switch (extract_common_type(args[0]))
        {
        case node_data_type_bool:
            return argminmax3d(numargs, extract_boolean_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_int64:
            return argminmax3d(numargs, extract_integer_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_double:
            return argminmax3d(numargs, extract_numeric_value_strict(
                std::move(args[0]), name_, codename_), axis);

        case node_data_type_unknown:
            return argminmax3d(numargs, extract_numeric_value(
                std::move(args[0]), name_, codename_), axis);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "argminmax<Op, Derived>::argminmax3d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                    "be numeric data types"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    hpx::future<primitive_argument_type> argminmax<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argminmax<Op, Derived>::eval",
                generate_error_message(
                    "the argminmax primitive requires exactly one or "
                    "two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "argminmax<Op, Derived>::eval",
                    generate_error_message(
                        "the argminmax primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                std::size_t a_dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);
                switch (a_dims)
                {
                case 0:
                    return this_->argminmax0d(std::move(args));

                case 1:
                    return this_->argminmax1d(std::move(args));

                case 2:
                    return this_->argminmax2d(std::move(args));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->argminmax3d(std::move(args));
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argminmax<Op, Derived>::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

#endif
