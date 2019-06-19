// Copyright (c) 2018-2019 Bita Hasheminezhad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/flip_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>
#include <phylanx/util/detail/bad_swap.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/optional.hpp>

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
    std::vector<match_pattern_type> const flip_operation::match_data =
    {
        match_pattern_type{"flipud",
        std::vector<std::string>{"flipud(_1)"},
        &create_flip_operation, &create_primitive<flip_operation>,
        R"(a
        Args:

            a (array) : a vector a matrix or a tensor

        Returns:

        Flip array in the up/down direction)"},

        match_pattern_type{"fliplr",
        std::vector<std::string>{"fliplr(_1)"},
        &create_flip_operation, &create_primitive<flip_operation>,
        R"(a
        Args:

            a (array) : a matrix or a tensor

        Returns:

        Flip array in the left/right direction)"},

        match_pattern_type{"flip",
        std::vector<std::string>{"flip(_1,_2)","flip(_1)"},
        &create_flip_operation, &create_primitive<flip_operation>,
        R"(a, axes
        Args:

            a (array) : a scalar, a vector a matrix or a tensor
            axes (optional, integer or tuple of integers): an axis to flip
               over. The default, axis=None, will flip over all of the axes
               of the input array.

        Returns:

        Reverses the order of elements in an array along the given axes)"},
    };

    ///////////////////////////////////////////////////////////////////////////
    flip_operation::flip_mode extract_flip_mode(std::string const& name)
    {
        flip_operation::flip_mode result = flip_operation::flip_mode_axes;

        if (name.find("flipud") != std::string::npos)
        {
            result = flip_operation::flip_mode_up_down;
        }
        else if (name.find("fliplr") != std::string::npos)
        {
            result = flip_operation::flip_mode_left_right;
        }
        return result;
    }

    flip_operation::flip_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
        , mode_(extract_flip_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flip_operation::flip1d(ir::node_data<T>&& arg) const
    {
        if (!arg.is_ref())
        {
            auto v = arg.vector();
            std::reverse(std::begin(v), std::end(v));
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            auto v = arg.vector();
            blaze::DynamicVector<T> result(v.size());
            std::reverse_copy(std::begin(v), std::end(v), std::begin(result));
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip1d(ir::node_data<T>&& arg,
        ir::range&& axes) const
    {
        if (axes.size() != 1)
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flip1d",
                generate_error_message(
                    "the flip_operation primitive requires operand axis to be "
                    "of size 1 for vectors."));

        auto axis = extract_scalar_integer_value_strict(*axes.begin());
        if (axis != 0 && axis != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flip1d",
                generate_error_message(
                    "the flip_operation primitive requires operand axis to be "
                    "either 0 or -1 for vectors."));
        }

        if (!arg.is_ref())
        {
            auto v = arg.vector();
            std::reverse(std::begin(v), std::end(v));
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            auto v = arg.vector();
            blaze::DynamicVector<T> result(v.size());
            std::reverse_copy(std::begin(v), std::end(v), std::begin(result));
            return primitive_argument_type{std::move(result)};
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flip_operation::flip2d_axis0(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;

        auto m = arg.matrix();
        matrix_row_iterator<decltype(m)> m_begin(m);
        matrix_row_iterator<decltype(m)> m_end(m, m.rows());

        if (!arg.is_ref())
        {
            std::reverse(m_begin, m_end);
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicMatrix<T> result(m.rows(), m.columns());
            matrix_row_iterator<decltype(result)> r_begin(result);
            std::reverse_copy(m_begin, m_end, r_begin);
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip2d_axis1(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;

        auto m = arg.matrix();
        matrix_column_iterator<decltype(m)> m_begin(m);
        matrix_column_iterator<decltype(m)> m_end(m, m.columns());

        if (!arg.is_ref())
        {
            std::reverse(m_begin, m_end);
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicMatrix<T> result(m.rows(), m.columns());
            matrix_column_iterator<decltype(result)> r_begin(result);
            std::reverse_copy(m_begin, m_end, r_begin);
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip2d_both_axes(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_row_iterator;
        using phylanx::util::matrix_column_iterator;

        auto m = arg.matrix();
        matrix_row_iterator<decltype(m)> r_begin(m);
        matrix_row_iterator<decltype(m)> r_end(m, m.rows());

        if (!arg.is_ref())
        {
            matrix_column_iterator<decltype(m)> c_begin(m);
            matrix_column_iterator<decltype(m)> c_end(m, m.columns());
            std::reverse(r_begin, r_end);
            std::reverse(c_begin, c_end);
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicMatrix<T> result(m.rows(), m.columns());
            matrix_row_iterator<decltype(result)> result_begin(result);
            std::reverse_copy(r_begin, r_end, result_begin);
            matrix_column_iterator<decltype(result)> c_begin(result);
            matrix_column_iterator<decltype(result)> c_end(
                result, result.columns());
            std::reverse(c_begin, c_end);
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip2d(ir::node_data<T>&& arg,
        ir::range&& axes) const
    {
        if (axes.size() == 2)
        {
            auto it = axes.begin();
            auto first = extract_scalar_integer_value_strict(*it);
            auto second = extract_scalar_integer_value_strict(*++it);

            if (first < 0)
                first += 2;
            if (second < 0)
                second += 2;

            if ((first == 0 && second == 1) || (first == 1 && second == 0))
                return flip2d_both_axes(std::move(arg));

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flip2d",
                generate_error_message(
                    "the flip_operation primitive requires each axis "
                    "to be between -2 and 1 for matrices and an axis should "
                    "not be repeated when both are given"));
        }
        else if (axes.size() == 1)
        {
            switch (extract_scalar_integer_value_strict(*axes.begin()))
            {
            case -2:
                HPX_FALLTHROUGH;
            case 0:
                return flip2d_axis0(std::move(arg));

            case -1:
                HPX_FALLTHROUGH;
            case 1:
                return flip2d_axis1(std::move(arg));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "flip_operation::flip2d",
                    generate_error_message(
                        "the flip_operation primitive requires operand axis "
                        "to be between -2 and 1 for matrices."));
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "flip_operation::flip2d",
            generate_error_message(
                "the flip_operation primitive requires operand axis "
                "to be of size 1 or 2 for matrices."));
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    template <typename T>
    primitive_argument_type flip_operation::flip3d_axis0(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;

        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::rowslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d_axis1(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;

        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::columnslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d_axis2(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;

        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.pages(); ++i)
            {
                auto slice = blaze::pageslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.pages(); ++i)
            {
                auto slice = blaze::pageslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::pageslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d_axes_0_1(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;
        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::rowslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            for (std::size_t i = 0; i != result.columns(); ++i)
            {
                auto slice = blaze::columnslice(result, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d_axes_0_2(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;
        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            for (std::size_t i = 0; i != t.pages(); ++i)
            {
                auto slice = blaze::pageslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::rowslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            for (std::size_t i = 0; i != result.pages(); ++i)
            {
                auto slice = blaze::pageslice(result, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d_axes_1_2(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;
        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            for (std::size_t i = 0; i != t.pages(); ++i)
            {
                auto slice = blaze::pageslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::columnslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            for (std::size_t i = 0; i != result.pages(); ++i)
            {
                auto slice = blaze::pageslice(result, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d_all_axes(
        ir::node_data<T>&& arg) const
    {
        using phylanx::util::matrix_column_iterator;
        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            for (std::size_t i = 0; i != t.columns(); ++i)
            {
                auto slice = blaze::columnslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            for (std::size_t i = 0; i != t.pages(); ++i)
            {
                auto slice = blaze::pageslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(arg)};
        }
        else
        {
            blaze::DynamicTensor<T> result(t.pages(), t.rows(), t.columns());
            for (std::size_t i = 0; i != t.rows(); ++i)
            {
                auto slice = blaze::rowslice(t, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                auto result_slice = blaze::rowslice(result, i);
                matrix_column_iterator<decltype(result_slice)> r_begin(
                    result_slice);
                std::reverse_copy(c_begin, c_end, r_begin);
            }
            for (std::size_t i = 0; i != result.columns(); ++i)
            {
                auto slice = blaze::columnslice(result, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            for (std::size_t i = 0; i != result.pages(); ++i)
            {
                auto slice = blaze::pageslice(result, i);
                matrix_column_iterator<decltype(slice)> c_begin(slice);
                matrix_column_iterator<decltype(slice)> c_end(
                    slice, slice.columns());
                std::reverse(c_begin, c_end);
            }
            return primitive_argument_type{std::move(result)};
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flip3d(ir::node_data<T>&& arg,
        ir::range&& axes) const
    {
        if (axes.size() == 3)
        {
            auto it = axes.begin();
            auto first = extract_scalar_integer_value_strict(*it);
            auto second = extract_scalar_integer_value_strict(*++it);
            auto third = extract_scalar_integer_value_strict(*++it);

            if (first < 0)
                first += 3;
            if (second < 0)
                second += 3;
            if (third < 0)
                third += 3;

            if ((first == 0 && second == 1 && third == 2) ||
                (first == 0 && second == 2 && third == 1) ||
                (first == 1 && second == 0 && third == 2) ||
                (first == 1 && second == 2 && third == 0) ||
                (first == 2 && second == 1 && third == 0) ||
                (first == 2 && second == 0 && third == 1))
                return flip3d_all_axes(std::move(arg));

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flip3d",
                generate_error_message(
                    "the flip_operation primitive requires each axis "
                    "to be between -3 and 2 for tensors and there should "
                    "not be any repetition in axes"));
        }
        else if (axes.size() == 2)
        {
            auto it = axes.begin();
            auto first = extract_scalar_integer_value_strict(*it);
            auto second = extract_scalar_integer_value_strict(*++it);

            if (first < 0)
                first += 3;
            if (second < 0)
                second += 3;

            if ((first == 0 && second == 1) || (first == 1 && second == 0))
                return flip3d_axes_0_1(std::move(arg));

            else if ((first == 0 && second == 2) || (first == 2 && second == 0))
                return flip3d_axes_0_2(std::move(arg));

            else if ((first == 1 && second == 2) || (first == 2 && second == 1))
                return flip3d_axes_1_2(std::move(arg));

            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flip3d",
                generate_error_message(
                    "the flip_operation primitive requires each axis "
                    "to be between -3 and 2 for tensors and there should "
                    "not be any repetition in axes"));
        }
        else if (axes.size() == 1)
        {
            switch (extract_scalar_integer_value_strict(*axes.begin()))
            {
            case -3:
                HPX_FALLTHROUGH;
            case 0:
                return flip3d_axis0(std::move(arg));

            case -2:
                HPX_FALLTHROUGH;
            case 1:
                return flip3d_axis1(std::move(arg));

            case -1:
                HPX_FALLTHROUGH;
            case 2:
                return flip3d_axis2(std::move(arg));

            default:
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "flip_operation::flip3d",
                    generate_error_message(
                        "the flip_operation primitive requires operand axis "
                        "to be between -3 and 2 for tensors."));
            }
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "flip_operation::flip3d",
            generate_error_message(
                "the flip_operation primitive requires operand axis "
                "to be of size 1, 2 or 3 for tensors."));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flip_operation::flipnd(ir::node_data<T>&& arg) const
    {
        switch (arg.num_dimensions())
        {
        case 0:
            return primitive_argument_type{arg.scalar()};

        case 1:
            return flip1d(std::move(arg));

        case 2:
            return flip2d_both_axes(std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return flip3d_all_axes(std::move(arg));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flipnd",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    template <typename T>
    primitive_argument_type flip_operation::flipnd(ir::node_data<T>&& arg,
        ir::range&& axes) const
    {
        std::size_t a_dims = arg.num_dimensions();
        switch (a_dims)
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flipnd",
                generate_error_message("axis should be None for a scalar"));

        case 1:
            return flip1d(std::move(arg), std::move(axes));

        case 2:
            return flip2d(std::move(arg), std::move(axes));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return flip3d(std::move(arg), std::move(axes));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flipnd",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    primitive_argument_type flip_operation::flipnd_helper(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return flipnd(
                extract_boolean_value(std::move(arg), name_, codename_));
        case node_data_type_int64:
            return flipnd(
                extract_integer_value(std::move(arg), name_, codename_));
        case node_data_type_double:
            return flipnd(
                extract_numeric_value(std::move(arg), name_, codename_));
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "flip::flipnd_helper",
            generate_error_message(
                "the flip primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flip_operation::flipud(ir::node_data<T>&& arg) const
    {
        switch (arg.num_dimensions())
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flipud",
                generate_error_message("input array should be >= 1d"));

        case 1:
            return flip1d(std::move(arg));

        case 2:
            return flip2d_axis0(std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return flip3d_axis0(std::move(arg));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flipud",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    primitive_argument_type flip_operation::flipud_helper(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return flipud(
                extract_boolean_value(std::move(arg), name_, codename_));
        case node_data_type_int64:
            return flipud(
                extract_integer_value(std::move(arg), name_, codename_));
        case node_data_type_double:
            return flipud(
                extract_numeric_value(std::move(arg), name_, codename_));
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "flip::flipud_helper",
            generate_error_message(
                "the flip primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flip_operation::fliplr(ir::node_data<T>&& arg) const
    {
        switch (arg.num_dimensions())
        {
        case 0:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::fliplr",
                generate_error_message("input array should be >= 2d"));

        case 1:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::fliplr",
                generate_error_message("input array should be >= 2d"));

        case 2:
            return flip2d_axis1(std::move(arg));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            return flip3d_axis1(std::move(arg));
#endif

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::fliplr",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
    }

    primitive_argument_type flip_operation::fliplr_helper(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return fliplr(
                extract_boolean_value(std::move(arg), name_, codename_));
        case node_data_type_int64:
            return fliplr(
                extract_integer_value(std::move(arg), name_, codename_));
        case node_data_type_double:
            return fliplr(
                extract_numeric_value(std::move(arg), name_, codename_));
        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "flip::fliplr_helper",
            generate_error_message(
                "the flip primitive requires for all arguments "
                "to be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> flip_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2 && operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::eval",
                generate_error_message("the flip_operation primitive requires "
                                       "exactly one or two, operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::eval",
                generate_error_message(
                    "the flip_operation primitive requires that the "
                    "arguments given by the operands array are valid"));
        }


        auto this_ = this->shared_from_this();
        if (operands.size() == 2 && valid(operands[1]))
        {
            if (this_->mode_ == flip_mode_up_down ||
                this_->mode_ == flip_mode_left_right)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "flip::eval",
                    this_->generate_error_message(
                        "the flip operation in up/down and left/right mode "
                        "requires exactly one operand"));
            }

            auto&& op0 =
                value_operand(operands[0], args, name_, codename_, ctx);

            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f1,
                        hpx::future<ir::range>&& f2)
                -> primitive_argument_type
                {
                    auto&& arg = f1.get();
                    auto&& axis = f2.get();

                    switch (extract_common_type(arg))
                    {
                    case node_data_type_bool:
                        return this_->flipnd(
                            extract_boolean_value(
                                std::move(arg), this_->name_, this_->codename_),
                            std::move(axis));

                    case node_data_type_int64:
                        return this_->flipnd(
                            extract_integer_value(
                                std::move(arg), this_->name_, this_->codename_),
                            std::move(axis));

                    case node_data_type_double:
                        return this_->flipnd(
                            extract_numeric_value(
                                std::move(arg), this_->name_, this_->codename_),
                            std::move(axis));

                    default:
                        break;
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "flip::eval",
                        this_->generate_error_message(
                            "the flip primitive requires for all arguments "
                            "to be numeric data types"));
                },
                std::move(op0),
                list_operand(operands[1], args, name_, codename_, std::move(ctx)));
        }

        if (operands.size() == 1 || mode_ == flip_mode_axes)
        {
            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& arg)
                -> primitive_argument_type
                {
                    switch (this_->mode_)
                    {
                    case flip_mode_up_down:
                        return this_->flipud_helper(arg.get());

                    case flip_mode_left_right:
                        return this_->fliplr_helper(arg.get());

                    case flip_mode_axes:
                        return this_->flipnd_helper(arg.get());

                    default:
                        break;
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "flip_operation::eval",
                        this_->generate_error_message(
                            "unsupported flip mode requested"));
                },
                value_operand(operands[0], args, name_, codename_, std::move(ctx)));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "flip::eval",
            this_->generate_error_message(
                "the flip operation in up/down and left/right mode "
                "requires exactly one operand"));
    }
}}}
