// Copyright (c) 2018 Bibek Wagle
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/diag_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const diag_operation::match_data =
    {
        hpx::util::make_tuple("diag",
            std::vector<std::string>{"diag(_1)", "diag(_1, _2)"},
            &create_diag_operation, &create_primitive<diag_operation>, R"(
            m
            Args:

                m (matrix) : a square matrix

            Returns:

            A vector created from the diagonal elements of `m`.)"
        )
    };

    ///////////////////////////////////////////////////////////////////////////
    diag_operation::diag_operation(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    primitive_argument_type diag_operation::diag0d(
        ir::node_data<T>&& arg, std::int64_t k) const
    {
        return primitive_argument_type(std::move(arg));
    }

    template <typename T>
    primitive_argument_type diag_operation::diag1d(
        ir::node_data<T>&& arg, std::int64_t k) const
    {
        auto vecsize = arg.dimension(0);
        auto incr = std::abs(k);

        blaze::DynamicMatrix<T> result(vecsize + incr, vecsize + incr, 0);
        blaze::band(result, k) = arg.vector();

        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    template <typename T>
    primitive_argument_type diag_operation::diag2d(
        ir::node_data<T>&& arg, std::int64_t k) const
    {
        blaze::DynamicVector<T> result(blaze::band(arg.matrix(), k));
        return primitive_argument_type{ir::node_data<T>{std::move(result)}};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type diag_operation::diag0d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return diag0d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return diag0d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return diag0d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return diag0d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "diag_operation::diag0d",
            generate_error_message(
                "the diag primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type diag_operation::diag1d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return diag1d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return diag1d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return diag1d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return diag1d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "diag_operation::diag1d",
            generate_error_message(
                "the diag primitive requires for all arguments to "
                    "be numeric data types"));
    }

    primitive_argument_type diag_operation::diag2d(
        primitive_argument_type&& arg, std::int64_t k) const
    {
        switch (extract_common_type(arg))
        {
        case node_data_type_bool:
            return diag2d(
                extract_boolean_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_int64:
            return diag2d(
                extract_integer_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_double:
            return diag2d(
                extract_numeric_value_strict(std::move(arg), name_, codename_),
                k);

        case node_data_type_unknown:
            return diag2d(
                extract_numeric_value(std::move(arg), name_, codename_), k);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "diag_operation::diag2d",
            generate_error_message(
                "the diag primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> diag_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "diag_operation::diag_operation",
                generate_error_message(
                    "the diag_operation primitive requires "
                    "either one or two arguments"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "diag_operation::eval",
                generate_error_message(
                    "the diag_operation primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 1)
        {
            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f)
                -> primitive_argument_type
                {
                    auto&& arg = f.get();

                    switch (extract_numeric_value_dimension(
                        arg, this_->name_, this_->codename_))
                    {
                    case 0:
                        return this_->diag0d(std::move(arg), 0);

                    case 1:
                        return this_->diag1d(std::move(arg), 0);

                    case 2:
                        return this_->diag2d(std::move(arg), 0);

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "diag_operation::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                "number of dimensions"));
                    }
                },
                value_operand(operands[0], args, name_, codename_, std::move(ctx)));
        }

        auto&& arg1 = value_operand(operands[0], args, name_, codename_, ctx);

        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                hpx::future<primitive_argument_type>&& f1,
                hpx::future<ir::node_data<std::int64_t>>&& f2)
            -> primitive_argument_type
            {
                auto&& arg1 = f1.get();
                auto&& arg2 = f2.get();

                if (arg2.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "diag_operation::eval",
                        this_->generate_error_message(
                            "second operand has to be a scalar value"));
                }

                switch (extract_numeric_value_dimension(
                    arg1, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->diag0d(std::move(arg1), arg2[0]);

                case 1:
                    return this_->diag1d(std::move(arg1), arg2[0]);

                case 2:
                    return this_->diag2d(std::move(arg1), arg2[0]);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "diag_operation::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                            "number of dimensions"));
                }
            },
            std::move(arg1),
            integer_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
