// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/flatten.hpp>
#include <phylanx/util/matrix_iterators.hpp>

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
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const flatten::match_data = {hpx::util::make_tuple(
        "flatten", std::vector<std::string>{"flatten(_1, _2)", "flatten(_1)"},
        &create_flatten, &create_primitive<flatten>, R"(
            a, axis
            Args:

                ar (array) : a vector or matrix
                order (optional, char): 'C' means row-major(C-style),
                                        'F' means column-major(Fortran-style),
                                        The default is 'C'.

            Returns:

            A copy of the array collapsed into one dimension."
            )")};

    ///////////////////////////////////////////////////////////////////////////
    flatten::flatten(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flatten::flatten0d(ir::node_data<T>&& arg) const
    {
        blaze::DynamicVector<T> result(1UL, arg.scalar());
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    primitive_argument_type flatten::flatten0d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return flatten0d(
                extract_boolean_value_strict(std::move(arg), name_, codename_));

        case node_data_type_int64:
            return flatten0d(
                extract_integer_value_strict(std::move(arg), name_, codename_));

        case node_data_type_double:
            return flatten0d(
                extract_numeric_value_strict(std::move(arg), name_, codename_));

        case node_data_type_unknown:
            return flatten0d(
                extract_numeric_value(std::move(arg), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::flatten::flatten0d",
            generate_error_message(
                "the flatten primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flatten::flatten1d(ir::node_data<T>&& arg) const
    {
        return primitive_argument_type({std::move(arg)});
    }

    primitive_argument_type flatten::flatten1d(
        primitive_argument_type&& arg) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return flatten1d(
                extract_boolean_value_strict(std::move(arg), name_, codename_));

        case node_data_type_int64:
            return flatten1d(
                extract_integer_value_strict(std::move(arg), name_, codename_));

        case node_data_type_double:
            return flatten1d(
                extract_numeric_value_strict(std::move(arg), name_, codename_));

        case node_data_type_unknown:
            return flatten1d(
                extract_numeric_value(std::move(arg), name_, codename_));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::flatten::flatten1d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type flatten::flatten2d(
        ir::node_data<T>&& arg, std::string order) const
    {
        using phylanx::util::matrix_column_iterator;
        using phylanx::util::matrix_row_iterator;

        auto a = arg.matrix();

        blaze::DynamicVector<T> result(a.rows() * a.columns());

        if (order == "C")
        {
            const matrix_row_iterator<decltype(a)> a_begin(a);
            const matrix_row_iterator<decltype(a)> a_end(a, a.rows());

            auto d = result.data();

            for (auto it = a_begin; it != a_end; ++it)
            {
                d = std::copy(it->begin(), it->end(), d);
            }
        }

        if (order == "F")
        {
            const matrix_column_iterator<decltype(a)> a_begin(a);
            const matrix_column_iterator<decltype(a)> a_end(a, a.columns());

            auto d = result.data();

            for (auto it = a_begin; it != a_end; ++it)
            {
                d = std::copy(it->begin(), it->end(), d);
            }
        }

        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type flatten::flatten2d(
        primitive_argument_type&& arg, std::string order) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return flatten2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                order);

        case node_data_type_int64:
            return flatten2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                order);

        case node_data_type_double:
            return flatten2d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                order);

        case node_data_type_unknown:
            return flatten2d(
                extract_numeric_value(std::move(arg), name_, codename_), order);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::flatten::flatten2d",
            generate_error_message(
                "the arange primitive requires for all arguments to "
                "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> flatten::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "flatten::eval",
                generate_error_message("the flatten primitive requires exactly "
                                       "one or two operands"));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "flatten::eval",
                    generate_error_message(
                        "the flatten primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 1)
        {
            return hpx::dataflow(hpx::launch::sync,
                hpx::util::unwrapping(
                    [this_ = std::move(this_)](primitive_argument_type&& arg)
                        -> primitive_argument_type {

                        std::size_t a_dims = extract_numeric_value_dimension(
                            arg, this_->name_, this_->codename_);

                        switch (a_dims)
                        {
                        case 0:
                            return this_->flatten0d(std::move(arg));

                        case 1:
                            return this_->flatten1d(std::move(arg));

                        case 2:
                            return this_->flatten2d(std::move(arg), "C");

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "flatten::eval",
                                this_->generate_error_message(
                                    "operand a has an invalid number of "
                                    "dimensions"));
                        }
                    }),
                value_operand(operands[0], args, name_, codename_, ctx));
        }
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_argument_type&& arg,
                                      std::string order)
                                      -> primitive_argument_type {
                if (order != "C" && order != "F")
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::flatten::"
                        "flatten0d",
                        this_->generate_error_message(
                            "order not understood "
                            "the order parameter could only be 'C' or 'F'"));

                std::size_t a_dims = extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_);

                switch (a_dims)
                {
                case 0:
                    return this_->flatten0d(std::move(arg));

                case 1:
                    return this_->flatten1d(std::move(arg));

                case 2:
                    return this_->flatten2d(std::move(arg), std::move(order));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "flatten::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            value_operand(operands[0], args, name_, codename_, ctx),
            string_operand(operands[1], args, name_, codename_, ctx));
    }
}}}
