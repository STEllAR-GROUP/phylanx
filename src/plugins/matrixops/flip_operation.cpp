// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const flip_operation::match_data =
    {
        hpx::util::make_tuple("flip",
        std::vector<std::string>{"flip(_1,_2)","flip(_1)"},
        &create_flip_operation, &create_primitive<flip_operation>,
        "a, axis\n"
        "Args:\n"
        "\n"
        "    a (array) : a scalar, a vector a matrix or a tensor\n"
        "    axis (optional, integer or tuple of integers): an axis to flip "
        "       over. The default, axis=None, will flip over all of the axes "
        "       of the input array.\n"
        "\n"
        "Returns:\n"
        "\n"
        "Reverses the order of elements in an array along the given axis")
    };

    ///////////////////////////////////////////////////////////////////////////
    flip_operation::flip_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

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

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flip_operation::flipnd",
                generate_error_message(
                    "operand a has an invalid number of dimensions"));
        }
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
            return hpx::dataflow(hpx::launch::sync,
                hpx::util::unwrapping(
                    [this_ = std::move(this_)](primitive_argument_type&& arg,
                        ir::range&& axis)->primitive_argument_type {

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
            }),
                value_operand(operands[0], args, name_, codename_, ctx),
                list_operand(operands[1], args, name_, codename_, ctx));
        }
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](
                    primitive_argument_type&& arg) -> primitive_argument_type {
                    switch (extract_common_type(arg))
                    {
                    case node_data_type_bool:
                        return this_->flipnd(extract_boolean_value(
                            std::move(arg), this_->name_, this_->codename_));
                    case node_data_type_int64:
                        return this_->flipnd(extract_integer_value(
                            std::move(arg), this_->name_, this_->codename_));
                    case node_data_type_double:
                        return this_->flipnd(extract_numeric_value(
                            std::move(arg), this_->name_, this_->codename_));
                    default:
                        break;
                    }

                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "flip::eval",
                        this_->generate_error_message(
                            "the flip primitive requires for all arguments "
                            "to be numeric data types"));
                }),
            value_operand(operands[0], args, name_, codename_, ctx));
    }
}}}
