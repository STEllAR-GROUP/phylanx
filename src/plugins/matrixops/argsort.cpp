// Copyright (c) 2019 R. Tohid
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/argsort.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const argsort::match_data = {
        hpx::util::make_tuple("argsort",
            std::vector<std::string>{
                "argsort(_1, __arg(_2_axis, -1), __arg(_3_kind, "
                "\"quicksort\"), __arg(_4_order, nil))"},
            &create_argsort, &create_primitive<argsort>,
            R"(
            a, axis, kind, order
            Args:
              a (array_like):
                An array to sort.

              axis (optional, {int, nil}):
                Axis along which to sort.
                The default is -1 (the last axis).
                If nil, the flattend array is used.

              kind (optional, {'quicksort', 'merrgesort', 'heapsort', 'stable'}):
                Sorting algorithm.
                *** ignored ***

              order (optional, {str, list of str}):
                When a is an array with fields defined, this argument specifies which
                fields to compare first, second, etc. A single field can be specified
                as a string, and not all fields need be specified, but unspecified
                fields will still be used, in the order in which they come up in the
                dtype, to break ties.
                *** ignored ***

            Returns:
              index_array ({ndarray, int}):
              When a is an array with fields defined, this argument specifies which
              fields to compare first, second, etc. A single field can be specified as a
              string, and not all fields need be specified, but unspecified fields will
              still be used, in the order in which they come up in the dtype, to break
              ties.

source:
https://docs.scipy.org/doc/numpy/reference/generated/numpy.argsort.html#numpy.argsort.)")};

    ///////////////////////////////////////////////////////////////////////////
    argsort::argsort(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    template <typename T>
    primitive_argument_type argsort::argsort_flatten2d(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        auto mat = in_array.matrix();
        auto flatten = blaze::ravel(mat);
        blaze::DynamicVector<std::int64_t> idx(mat.rows() * mat.columns());
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(),
            [&flatten](size_t a, size_t b) { return flatten[a] < flatten[b]; });
        return primitive_argument_type{std::move(idx)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type argsort::argsort_flatten3d(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        auto tensor = in_array.tensor();
        auto flatten = blaze::ravel(tensor);
        blaze::DynamicVector<std::int64_t> idx(
            tensor.pages() * tensor.rows() * tensor.columns());
        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end(),
            [&flatten](size_t a, size_t b) { return flatten[a] < flatten[b]; });
        return primitive_argument_type{std::move(idx)};
    }
#endif
    template <typename T>
    primitive_argument_type argsort::argsort_flatten_helper(
        ir::node_data<T>&& in_array, std::int64_t axis, std::string kind,
        std::string order) const
    {
        auto dim = extract_numeric_value_dimension(in_array, name_, codename_);
        switch (dim)
        {
        case 1:
            return argsort1d(std::move(in_array), -1, kind, order);
        case 2:
            return argsort_flatten2d(std::move(in_array), kind, order);
        case 3:
            return argsort_flatten3d(std::move(in_array), kind, order);
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argsort::argsort_helper",
                util::generate_error_message(
                    "Invalid dimension. The `in_array` could be 0 to 3 "
                    "dimensional.",
                    name_, codename_));
#else
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argsort::argsort_helper",
                util::generate_error_message(
                    "Invalid dimension. The `in_array` could be 0 to 2 "
                    "dimensional.",
                    name_, codename_));
#endif
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "argsort::argsort_flatten_helper",
            generate_error_message("Invalid axis."));
    }

    primitive_argument_type argsort::argsort_flatten(
        primitive_argument_type&& in_array, std::int64_t axis, std::string kind,
        std::string order) const
    {
        switch (extract_common_type(in_array))
        {
        case node_data_type_bool:
            return argsort_flatten_helper(
                extract_boolean_value_strict(
                    std::move(in_array), name_, codename_),
                axis, kind, order);
        case node_data_type_int64:
            return argsort_flatten_helper(
                extract_integer_value_strict(
                    std::move(in_array), name_, codename_),
                axis, kind, order);
        case node_data_type_double:
            return argsort_flatten_helper(
                extract_numeric_value_strict(
                    std::move(in_array), name_, codename_),
                axis, kind, order);
        case node_data_type_unknown:
            return argsort_flatten_helper(
                extract_numeric_value(std::move(in_array), name_, codename_),
                axis, kind, order);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argsort::argsort_flatten_helper",
                generate_error_message(
                    "`argsort` expects the input to be an "
                    "`array_like`, i.e., numeric data type. "));
        }
    }

    template <typename T>
    primitive_argument_type argsort::argsort1d(ir::node_data<T>&& in_array,
        std::int64_t axis, std::string kind, std::string order) const
    {
        if (0 == axis || -1 == axis)
        {
            auto vec = in_array.vector();
            blaze::DynamicVector<std::int64_t> idx(vec.size());
            std::iota(idx.begin(), idx.end(), 0);
            std::sort(idx.begin(), idx.end(),
                [&vec](size_t a, size_t b) { return vec[a] < vec[b]; });
            return primitive_argument_type{std::move(idx)};
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "argsort::argsort1d",
            generate_error_message("Invalid axis. `argsort` of a 1 "
                                   "dimensional `array_like` "
                                   "could only be 0 or 1."));
    }

    template <typename T>
    primitive_argument_type argsort::argsort2d_axis0(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        using phylanx::util::matrix_column_iterator;
        auto mat = in_array.matrix();
        blaze::DynamicMatrix<std::int64_t> idx(mat.rows(), mat.columns());

        matrix_column_iterator<decltype(mat)> mat_cols_begin(mat);
        matrix_column_iterator<decltype(mat)> mat_cols_end(mat, mat.columns());

        matrix_column_iterator<decltype(idx)> idx_cols_it(idx);

        for (auto mat_col = mat_cols_begin; mat_col != mat_cols_end;
             ++mat_col, ++idx_cols_it)
        {
            std::iota(idx_cols_it->begin(), idx_cols_it->end(), 0);
            std::sort(idx_cols_it->begin(), idx_cols_it->end(),
                [mat_col](size_t a, size_t b) {
                    return *(mat_col->begin() + a) < *(mat_col->begin() + b);
                });
        }

        return primitive_argument_type{std::move(idx)};
    }

    template <typename T>
    primitive_argument_type argsort::argsort2d_axis1(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        using phylanx::util::matrix_row_iterator;
        auto mat = in_array.matrix();
        blaze::DynamicMatrix<std::int64_t> idx(mat.rows(), mat.columns());

        matrix_row_iterator<decltype(mat)> mat_rows_begin(mat);
        matrix_row_iterator<decltype(mat)> mat_rows_end(mat, mat.rows());

        matrix_row_iterator<decltype(idx)> idx_rows_it(idx);

        for (auto mat_row = mat_rows_begin; mat_row != mat_rows_end;
             ++mat_row, ++idx_rows_it)
        {
            std::iota(idx_rows_it->begin(), idx_rows_it->end(), 0);
            std::sort(idx_rows_it->begin(), idx_rows_it->end(),
                [mat_row](size_t a, size_t b) {
                    return *(mat_row->begin() + a) < *(mat_row->begin() + b);
                });
        }

        return primitive_argument_type{std::move(idx)};
    }

    template <typename T>
    primitive_argument_type argsort::argsort2d(ir::node_data<T>&& in_array,
        std::int64_t axis, std::string kind, std::string order) const
    {
        switch (axis)
        {
        case -2:
            HPX_FALLTHROUGH;
        case 0:
            return argsort2d_axis0(std::move(in_array), kind, order);
        case -1:
            HPX_FALLTHROUGH;
        case 1:
            return argsort2d_axis1(std::move(in_array), kind, order);
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "argsort::argsort3d",
                generate_error_message("Invalid axis. `argsort` of a 2 "
                                       "dimensional `array_like` "
                                       "must be in range [-2, 1]"));
        }
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type argsort::argsort3d_axis0(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        using phylanx::util::matrix_row_iterator;
        auto tensor = in_array.tensor();
        blaze::DynamicTensor<std::int64_t> idx(
            tensor.pages(), tensor.rows(), tensor.columns());

        for (size_t row = 0; row < tensor.rows(); ++row)
        {
            auto tensor_row_slice = blaze::rowslice(tensor, row);
            matrix_row_iterator<decltype(tensor_row_slice)> const
                mat_slice_rows_begin(tensor_row_slice);
            matrix_row_iterator<decltype(tensor_row_slice)> const
                mat_slice_rows_end(tensor_row_slice, tensor_row_slice.rows());

            auto idx_row_slice = blaze::rowslice(idx, row);
            matrix_row_iterator<decltype(idx_row_slice)> const
                idx_slice_rows_begin(idx_row_slice);

            auto idx_row = idx_slice_rows_begin;
            for (auto mat_row = mat_slice_rows_begin;
                 mat_row != mat_slice_rows_end; ++mat_row, ++idx_row)
            {
                std::iota(idx_row->begin(), idx_row->end(), 0);
                std::sort(idx_row->begin(), idx_row->end(),
                    [mat_row](size_t a, size_t b) {
                        return *(mat_row->begin() + a) <
                            *(mat_row->begin() + b);
                    });
            }
        }
        return primitive_argument_type{std::move(idx)};
    }

    template <typename T>
    primitive_argument_type argsort::argsort3d_axis1(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        using phylanx::util::matrix_row_iterator;
        auto tensor = in_array.tensor();
        blaze::DynamicTensor<std::int64_t> idx(
            tensor.pages(), tensor.rows(), tensor.columns());

        for (size_t page = 0; page < tensor.columns(); ++page)
        {
            auto tensor_col_slice = blaze::columnslice(tensor, page);
            matrix_row_iterator<decltype(tensor_col_slice)> const
                mat_slice_rows_begin(tensor_col_slice);
            matrix_row_iterator<decltype(tensor_col_slice)> const
                mat_slice_rows_end(
                    tensor_col_slice, tensor_col_slice.columns());

            auto idx_col_slice = blaze::columnslice(idx, page);
            matrix_row_iterator<decltype(idx_col_slice)> const
                idx_slice_rows_begin(idx_col_slice);

            auto idx_row = idx_slice_rows_begin;
            for (auto mat_row = mat_slice_rows_begin;
                 mat_row != mat_slice_rows_end; ++mat_row, ++idx_row)
            {
                std::iota(idx_row->begin(), idx_row->end(), 0);
                std::sort(idx_row->begin(), idx_row->end(),
                    [mat_row](size_t a, size_t b) {
                        return *(mat_row->begin() + a) <
                            *(mat_row->begin() + b);
                    });
            }
        }
        return primitive_argument_type{std::move(idx)};
    }

    template <typename T>
    primitive_argument_type argsort::argsort3d_axis2(
        ir::node_data<T>&& in_array, std::string kind, std::string order) const
    {
        using phylanx::util::matrix_row_iterator;
        auto tensor = in_array.tensor();
        blaze::DynamicTensor<std::int64_t> idx(
            tensor.pages(), tensor.rows(), tensor.columns());

        for (size_t page = 0; page < tensor.pages(); ++page)
        {
            auto tensor_page_slice = blaze::pageslice(tensor, page);

            matrix_row_iterator<decltype(tensor_page_slice)> const
                mat_slice_pages_begin(tensor_page_slice);
            matrix_row_iterator<decltype(tensor_page_slice)> const
                mat_slice_pages_end(
                    tensor_page_slice, tensor_page_slice.rows());

            auto idx_page_slice = blaze::pageslice(idx, page);
            matrix_row_iterator<decltype(idx_page_slice)> const
                idx_slice_pages_begin(idx_page_slice);

            auto idx_page = idx_slice_pages_begin;
            for (auto mat_page = mat_slice_pages_begin;
                 mat_page != mat_slice_pages_end; ++mat_page, ++idx_page)
            {
                std::iota(idx_page->begin(), idx_page->end(), 0);
                std::sort(idx_page->begin(), idx_page->end(),
                    [mat_page](size_t a, size_t b) {
                        return *(mat_page->begin() + a) <
                            *(mat_page->begin() + b);
                    });
            }
        }
        return primitive_argument_type{std::move(idx)};
    }

    template <typename T>
    primitive_argument_type argsort::argsort3d(ir::node_data<T>&& in_array,
        std::int64_t axis, std::string kind, std::string order) const
    {
        switch (axis)
        {
        case -3:
            HPX_FALLTHROUGH;
        case 0:
            return argsort3d_axis0(std::move(in_array), kind, order);
        case -2:
            HPX_FALLTHROUGH;
        case 1:
            return argsort3d_axis1(std::move(in_array), kind, order);
        case -1:
            HPX_FALLTHROUGH;
        case 2:
            return argsort3d_axis2(std::move(in_array), kind, order);
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "argsort::argsort3d",
                generate_error_message("Invalid axis. `argsort` of a 3 "
                                       "dimensional `array_like` "
                                       "must be in range [-3, 2]"));
        }
    }
#endif
    template <typename T>
    primitive_argument_type argsort::argsort_helper(ir::node_data<T>&& in_array,
        std::int64_t axis, std::string kind, std::string order) const
    {
        switch (extract_numeric_value_dimension(in_array, name_, codename_))
        {
        // case 0:
        //     return argsort0d(std::move(in_array), axis, kind, order);
        case 1:
            return argsort1d(std::move(in_array), axis, kind, order);
        case 2:
            return argsort2d(std::move(in_array), axis, kind, order);
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return argsort3d(std::move(in_array), axis, kind, order);
#endif
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "argsort::argsort_helper",
                util::generate_error_message(
                    "Invalid dimension. The `in_array` could be 0 to 3 "
                    "dimensional.",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> argsort::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();

        // if no argument is passed to argsort, operands will hold a single element:
        // 'nil'
        if (operands.empty() || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "argsort::eval",
                this_->generate_error_message(
                    "The argsort primitive requires at least 1 argument- an "
                    "`array_like`."));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                std::string kind = extract_string_value_strict(
                    std::move(args[2]), this_->name_, this_->codename_);

                // TODO: order is ignored!
                std::string order = "";

                if (is_list_operand_strict(args[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argsort::eval",
                        this_->generate_error_message(
                            "TODO: argsoft does not support lists."));
                }
                else
                {
                    if (!valid(args[1]))
                    {
                        return this_->argsort_flatten(
                            std::move(args[0]), -1, kind, order);
                    }

                    std::int64_t axis = extract_scalar_integer_value_strict(
                        std::move(args[1]), this_->name_, this_->codename_);
                    switch (extract_common_type(args[0]))
                    {
                    case node_data_type_bool:
                        return this_->argsort_helper(
                            extract_boolean_value_strict(std::move(args[0]),
                                this_->name_, this_->codename_),
                            axis, kind, order);

                    case node_data_type_int64:
                        return this_->argsort_helper(
                            extract_integer_value_strict(std::move(args[0]),
                                this_->name_, this_->codename_),
                            axis, kind, order);

                    case node_data_type_unknown:
                    case node_data_type_double:
                        return this_->argsort_helper(
                            extract_numeric_value(std::move(args[0]),
                                this_->name_, this_->codename_),
                            axis, kind, order);

                    default:
                        break;
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "argsort::eval",
                        this_->generate_error_message(
                            "`argsort` expects the input to be an "
                            "`array_like`, i.e., numeric data type. "));
                }
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}    // namespace primitives
}    // namespace execution_tree
}    // namespace phylanx
