// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/reshape_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

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
    match_pattern_type const reshape_operation::match_data =
    {
        hpx::util::make_tuple("reshape",
        std::vector<std::string>{"reshape(_1,_2)"},
        &create_reshape_operation, &create_primitive<reshape_operation>,
         R"(
            a, newshape
            Args:

                a (array_like) : input array
                newshape (integer or tuple of integers): The new shape should be
                     compatible with the original shape (number of elements in
                     both arrays are the same). If an integer, then the result
                     will be a 1-D array of that length. The last parameter of
                     the newshape can be -1. In this case, the value is inferred
                     from the length of the array and remaining dimensions.

            Returns:

            Returns a new shape to an array without changing its data."
            )") };

    ///////////////////////////////////////////////////////////////////////////
    reshape_operation::reshape_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base(std::move(operands), name, codename)
    {}

    bool reshape_operation::validate_shape(
        std::int64_t n, ir::range const& arg) const
    {
        if (arg.size() == 1)
        {
            auto first = extract_scalar_integer_value_strict(*arg.begin());
            if (first == -1)
                return true;

            return first == n;
        }
        else if (arg.size() == 2)
        {
            auto it = arg.begin();
            auto first = extract_scalar_integer_value_strict(*it);
            auto second = extract_scalar_integer_value_strict(*++it);
            if (second == -1 && first > 0)
            {
                if (n % first == 0)
                    return true;
            }
            else if (first > 0 && second > 0)
            {
                return first * second == n;
            }
            return false;
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::validate_shape",
                util::generate_error_message("The given shape has an invalid "
                    "number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape0d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch(arg.size())
        {
        case 1:
            return primitive_argument_type{
                blaze::DynamicVector<T>{arr.scalar()}};

        case 2:
            return primitive_argument_type{
                blaze::DynamicMatrix<T>{1, 1, arr.scalar()}};

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message(
                    "reshaping to >2d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape0d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape0d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape0d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape0d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape0d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape0d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape1d_2d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        using phylanx::util::matrix_row_iterator;
        auto a = arr.vector();

        auto it = arg.begin();
        auto first = extract_scalar_integer_value(*it);
        auto second = extract_scalar_integer_value(*++it);

        blaze::DynamicMatrix<T> result(first, second);

        const matrix_row_iterator<decltype(result)> r_begin(result);
        const matrix_row_iterator<decltype(result)> r_end(result, first);

        auto b = a.begin();
        auto e = b;
        std::advance(e, second);

        for (auto i = r_begin; i != r_end; i++)
        {
            std::copy(b, e, i->begin());
            b = e;
            std::advance(e, second);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape1d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return primitive_argument_type{std::move(arr)};

        case 2:
            return reshape1d_2d(std::move(arr),std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::reshape1d",
                util::generate_error_message(
                    "reshaping to >2d is not supported",
                    name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape1d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape1d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape1d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape1d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape1d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape1d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type reshape_operation::reshape2d_1d(
        ir::node_data<T>&& arr) const
    {
        using phylanx::util::matrix_row_iterator;
        auto a = arr.matrix();

        blaze::DynamicVector<T> result(a.rows() * a.columns(), T(0));

        matrix_row_iterator<decltype(a)> const a_begin(a);
        matrix_row_iterator<decltype(a)> const a_end(a, a.rows());

        auto d = result.data();
        for (auto it = a_begin; it != a_end; ++it)
        {
            d = std::copy(it->begin(), it->end(), d);
        }
        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape2d_2d(
        ir::node_data<T>&& arr, ir::range&& arg) const
    {
        auto a = arr.matrix();

        auto it = arg.begin();
        auto rows = extract_scalar_integer_value(*it++);
        auto columns = extract_scalar_integer_value(*it);

        blaze::DynamicMatrix<T> result(rows, columns);

        using phylanx::util::matrix_iterator;
        matrix_iterator<decltype(a)> begin(a, 0);
        matrix_iterator<decltype(a)> end(a, a.rows());
        matrix_iterator<blaze::DynamicMatrix<T>> dest(result);

        std::copy(begin, end, dest);

        return primitive_argument_type{std::move(result)};
    }

    template <typename T>
    primitive_argument_type reshape_operation::reshape2d(ir::node_data<T>&& arr,
        ir::range&& arg) const
    {
        switch (arg.size())
        {
        case 1:
            return reshape2d_1d(std::move(arr));

        case 2:
            return reshape2d_2d(std::move(arr), std::move(arg));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::reshape2d",
                util::generate_error_message(
                    "reshaping to >2d is not supported", name_, codename_));
        }
    }

    primitive_argument_type reshape_operation::reshape2d(
        primitive_argument_type&& arr, ir::range&& arg) const
    {
        switch (extract_common_type(arr))
        {
        case node_data_type_bool:
            return reshape2d(
                extract_boolean_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_int64:
            return reshape2d(
                extract_integer_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_double:
            return reshape2d(
                extract_numeric_value_strict(std::move(arr), name_, codename_),
                std::move(arg));

        case node_data_type_unknown:
            return reshape2d(
                extract_numeric_value(std::move(arr), name_, codename_),
                std::move(arg));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::reshape_operation::reshape2d",
            generate_error_message(
                "the reshape primitive requires for all arguments to "
                "be numeric data types"));
    }
    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> reshape_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "reshape_operation::eval",
                util::generate_error_message(
                    "the reshape_operation primitive requires exactly two "
                    "operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "reshape_operation::eval",
                    util::generate_error_message(
                        "the reshape_operation primitive requires that the "
                        "arguments given by the operands array are valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](
                    primitive_argument_type&& arr, ir::range&& arg)
            ->  primitive_argument_type
            {
                auto arr_dims_num = extract_numeric_value_dimension(
                    arr, this_->name_, this_->codename_);
                auto size = extract_numeric_value_size(
                    arr, this_->name_, this_->codename_);

                if (arr_dims_num > 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "reshape_operation::eval",
                        util::generate_error_message("operand a has an invalid "
                            "number of dimensions",
                            this_->name_, this_->codename_));
                }

                if (this_->validate_shape(size, arg))
                {
                    switch (arr_dims_num)
                    {
                    case 0:
                        return this_->reshape0d(std::move(arr), std::move(arg));

                    case 1:
                        return this_->reshape1d(std::move(arr), std::move(arg));

                    case 2:
                        return this_->reshape2d(std::move(arr), std::move(arg));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "reshape_operation::eval",
                            util::generate_error_message("operand a has an invalid "
                                "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }
                else
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "reshape_operation::eval",
                        util::generate_error_message("The given shape is not "
                            "compatible with the shape of the original array",
                            this_->name_, this_->codename_));
                }
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            list_operand(operands[1], args, name_, codename_, ctx));
    }
}}}
