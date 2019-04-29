// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/sort.hpp>
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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const sort::match_data = {hpx::util::make_tuple("sort",
        std::vector<std::string>{"sort(_1, __arg(_2_axis, -1), __arg(_3_kind, "
                                 "\"quicksort\"), __arg(_4_order, nil))"},
        &create_sort, &create_primitive<sort>, R"(
            a, axis, kind, order
            Args:

                a (array_like) : input array
                axis (optional, int) : axis along which to sort. If None, array is
                                       flattened before sorting. Default is -1.
                kind (optional, string) : sorting algorithm, default is "quicksort"
                oredr (optional, string or list of string) :  specifies which fields to
                                                              compare first, second, etc.
            Returns:

            The sorted array."
            )")};

    ///////////////////////////////////////////////////////////////////////////
    sort::sort(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type sort::sort_flatten2d(
        ir::node_data<T>&& arg, std::string kind) const
    {
        auto m = arg.matrix();
        auto r = blaze::ravel(m);
        blaze::DynamicVector<T> result(m.rows() * m.columns());

        std::copy(r.begin(), r.end(), result.begin());
        std::sort(result.begin(), result.begin() + result.size());
        return primitive_argument_type{std::move(result)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type sort::sort_flatten3d(ir::node_data<T>&& arg,
        std::string kind) const
    {
        auto t = arg.tensor();
        auto r = blaze::ravel(t);
        blaze::DynamicVector<T> result(t.pages() * t.rows() * t.columns());

        std::copy(r.begin(), r.end(), result.begin());
        std::sort(result.begin(), result.begin() + result.size());
        return primitive_argument_type{std::move(result)};
    }
#endif

    template <typename T>
    primitive_argument_type sort::sort_flatten_helper(
        ir::node_data<T>&& arg, std::string kind) const
    {
        switch (extract_numeric_value_dimension(arg, name_, codename_))
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sort::sort_flatten_helper",
                util::generate_error_message(
                    "axis out of bounds for array of dimension 0", name_,
                    codename_));
        case 1:
            return sort1d(std::move(arg), -1, kind);
        case 2:
            return sort_flatten2d(std::move(arg), kind);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return sort_flatten3d(std::move(arg), kind);
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sort::eval",
            util::generate_error_message("operand a has an invalid "
                                         "number of dimensions",
                name_, codename_));
    }

    primitive_argument_type sort::sort_flatten(
        primitive_argument_type&& arg, std::string kind) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return sort_flatten_helper(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                kind);

        case node_data_type_int64:
            return sort_flatten_helper(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                kind);

        case node_data_type_double:
            return sort_flatten_helper(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                kind);

        case node_data_type_unknown:
            return sort_flatten_helper(
                extract_numeric_value(std::move(arg), name_, codename_), kind);
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sort::sort_flatten_helper",
            generate_error_message("the sort primitive requires for "
                                   "all arguments to be numeric data types"));
    }

    template <typename T>
    primitive_argument_type sort::sort1d(
        ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const
    {
        if (axis == 0 || axis == -1)
        {
            auto v = arg.vector();

            std::sort(v.begin(), v.begin() + v.size());
            return primitive_argument_type{std::move(arg)};
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sort::sort1d",
            generate_error_message(
                "axis is out of bounds for array of dimension 1"));
    }

    template <typename T>
    primitive_argument_type sort::sort2d_axis0(ir::node_data<T>&& arg,
        std::string kind) const
    {
        using phylanx::util::matrix_column_iterator;
        auto m = arg.matrix();

        matrix_column_iterator<decltype(m)> m_begin(m);
        const matrix_column_iterator<decltype(m)> m_end(m, m.columns());

        for (auto it = m_begin; it != m_end; ++it)
        {
            std::sort(it->begin(), it->end());
        }

        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type sort::sort2d_axis1(ir::node_data<T>&& arg,
        std::string kind) const
    {
        auto m = arg.matrix();
        using phylanx::util::matrix_row_iterator;

        matrix_row_iterator<decltype(m)> m_begin(m);
        const matrix_row_iterator<decltype(m)> m_end(m, m.rows());

        for (auto it = m_begin; it != m_end; ++it)
        {
            std::sort(it->begin(), it->end());
        }

        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type sort::sort2d(
        ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const
    {
        switch (axis)
        {
        case -2:
            HPX_FALLTHROUGH;
        case 0:
            return sort2d_axis0(std::move(arg), kind);
        case -1:
            HPX_FALLTHROUGH;
        case 1:
            return sort2d_axis1(std::move(arg), kind);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sort::sort2d",
            generate_error_message(
                "operand has an invalid value for the axis parameter"));
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type sort::sort3d_axis0(ir::node_data<T>&& arg,
        std::string kind) const
    {
        using phylanx::util::matrix_row_iterator;
        auto t = arg.tensor();

        for (std::size_t i = 0; i != t.rows(); ++i)
        {
            auto slice = blaze::rowslice(t, i);
            matrix_row_iterator<decltype(slice)> const a_begin(slice);
            matrix_row_iterator<decltype(slice)> const a_end(
                slice, slice.rows());

            for (auto it = a_begin; it != a_end; ++it)
                std::sort(it->begin(), it->end());
        }
        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type sort::sort3d_axis1(ir::node_data<T>&& arg,
        std::string kind) const
    {
        using phylanx::util::matrix_row_iterator;
        auto t = arg.tensor();

        for (std::size_t i = 0; i != t.columns(); ++i)
        {
            auto slice = blaze::columnslice(t, i);
            matrix_row_iterator<decltype(slice)> const a_begin(slice);
            matrix_row_iterator<decltype(slice)> const a_end(
                slice, slice.rows());

            for (auto it = a_begin; it != a_end; ++it)
                std::sort(it->begin(), it->end());
        }
        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type sort::sort3d_axis2(ir::node_data<T>&& arg,
        std::string kind) const
    {
        using phylanx::util::matrix_column_iterator;
        auto t = arg.tensor();

        for (std::size_t i = 0; i != t.rows(); ++i)
        {
            auto slice = blaze::rowslice(t, i);
            matrix_column_iterator<decltype(slice)> const a_begin(slice);
            matrix_column_iterator<decltype(slice)> const a_end(
                slice, slice.columns());

            for (auto it = a_begin; it != a_end; ++it)
                std::sort(it->begin(), it->end());
        }
        return primitive_argument_type{std::move(arg)};
    }

    template <typename T>
    primitive_argument_type sort::sort3d(
        ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const
    {
        switch (axis)
        {
        case -3:
            HPX_FALLTHROUGH;
        case 0:
            return sort3d_axis0(std::move(arg), kind);
        case -2:
            HPX_FALLTHROUGH;
        case 1:
            return sort3d_axis1(std::move(arg), kind);
        case -1:
            HPX_FALLTHROUGH;
        case 2:
            return sort3d_axis2(std::move(arg), kind);
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sort::sort3d",
            generate_error_message(
                "operand has an invalid value for the axis parameter"));
    }
#endif

    template <typename T>
    primitive_argument_type sort::sort_helper(
        ir::node_data<T>&& arg, std::int64_t axis, std::string kind) const
    {
        switch (extract_numeric_value_dimension(arg, name_, codename_))
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sort::sort_helper",
                util::generate_error_message(
                    "axis out of bounds for array of dimension 0", name_,
                    codename_));
        case 1:
            return sort1d(std::move(arg), axis, kind);
        case 2:
            return sort2d(std::move(arg), axis, kind);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return sort3d(std::move(arg), axis, kind);
#endif
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "sort::sort_helper",
            util::generate_error_message("operand a has an invalid "
                                         "number of dimensions",
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> sort::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "sort::eval",
                generate_error_message("the sort primitive requires at most "
                                       "four operands"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                //TODO: "order" is not implemented, "kind" is ignored
                std::int64_t axis = -1;
                std::string kind = "quicksort";
                if (args.size() > 2)
                {
                    kind = extract_string_value(
                        std::move(args[2]), this_->name_, this_->codename_);
                }

                if (kind != "quicksort" && kind != "mergesort" &&
                    kind != "heapsort" && kind != "stable")
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "sort::eval",
                        this_->generate_error_message(
                            "sort algorithm not supported"));
                }

                if (args.size() > 1)
                {
                    if (!valid(args[1]))
                        return this_->sort_flatten(std::move(args[0]), kind);
                    axis = extract_scalar_integer_value_strict(
                        std::move(args[1]), this_->name_, this_->codename_);
                }

                switch (extract_common_type(args[0]))
                {
                case node_data_type_bool:
                    return this_->sort_helper(
                        extract_boolean_value_strict(
                            std::move(args[0]), this_->name_, this_->codename_),
                        axis, kind);

                case node_data_type_int64:
                    return this_->sort_helper(
                        extract_integer_value_strict(
                            std::move(args[0]), this_->name_, this_->codename_),
                        axis, kind);

                case node_data_type_double:
                    return this_->sort_helper(
                        extract_numeric_value_strict(
                            std::move(args[0]), this_->name_, this_->codename_),
                        axis, kind);

                case node_data_type_unknown:
                    return this_->sort_helper(
                        extract_numeric_value(
                            std::move(args[0]), this_->name_, this_->codename_),
                        axis, kind);

                default:
                    break;
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "sort::eval",
                    this_->generate_error_message(
                        "the sort primitive requires for "
                        "all arguments to be numeric data types"));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
