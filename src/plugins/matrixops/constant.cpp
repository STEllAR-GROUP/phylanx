//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/matrixops/constant.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
#include <cmath>
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
    namespace detail
    {
        std::size_t extract_num_dimensions(ir::range const& shape)
        {
            return shape.size();
        }

        std::array<std::size_t, 2> extract_dimensions(ir::range const& shape)
        {
            std::array<std::size_t, 2> result = {0, 0};
            result[0] = extract_scalar_integer_value(*shape.begin());
            if (shape.size() > 1)
            {
                auto elem_1 = shape.begin();
                result[1] = extract_scalar_integer_value(*++elem_1);
            }
            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const constant::match_data =
    {
        match_pattern_type{"constant",
            std::vector<std::string>{"constant(_1, _2)", "constant(_1)"},
            &create_constant, &create_primitive<constant>, R"(
            arg1, arg2
            Args:

                arg1 (float): a constant value
                arg2 (int, optional): the number of values

            Returns:

            An array of size arg2 with each element equal to arg1.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    constant::constant(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant0d_helper(
        primitive_argument_type&& op) const
    {
        return ir::node_data<T>{
            extract_scalar_data<T>(std::move(op), name_, codename_)};
    }

    primitive_argument_type constant::constant0d(
        primitive_argument_type&& op) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op);
        }

        switch (t)
        {
        case node_data_type_bool:
            return constant0d_helper<std::uint8_t>(std::move(op));

        case node_data_type_int64:
            return constant0d_helper<std::int64_t>(std::move(op));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant0d_helper<double>(std::move(op));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant0d",
            util::generate_error_message(
                "the contsnat primitive requires for all arguments to "
                    "be numeric data types",
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant1d_helper(
        primitive_argument_type&& op, std::size_t dim) const
    {
        return ir::node_data<T>{blaze::DynamicVector<T>(
            dim, extract_scalar_data<T>(std::move(op), name_, codename_))};
    }

    primitive_argument_type constant::constant1d(
        primitive_argument_type&& op, std::size_t dim) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op);
        }

        switch (t)
        {
        case node_data_type_bool:
            return constant1d_helper<std::uint8_t>(std::move(op), dim);

        case node_data_type_int64:
            return constant1d_helper<std::int64_t>(std::move(op), dim);

        case node_data_type_double:
            return constant1d_helper<double>(std::move(op), dim);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant1d",
            util::generate_error_message(
                "the contsnat primitive requires for all arguments to "
                    "be numeric data types",
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant2d_helper(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim) const
    {
        return ir::node_data<T>{blaze::DynamicMatrix<T>(dim[0], dim[1],
            extract_scalar_data<T>(std::move(op), name_, codename_))};
    }

    primitive_argument_type constant::constant2d(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op);
        }

        switch (t)
        {
        case node_data_type_bool:
            return constant2d_helper<std::uint8_t>(std::move(op), dim);

        case node_data_type_int64:
            return constant2d_helper<std::int64_t>(std::move(op), dim);

        case node_data_type_double:
            return constant2d_helper<double>(std::move(op), dim);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant2d",
            util::generate_error_message(
                "the contsnat primitive requires for all arguments to "
                    "be numeric data types",
                name_, codename_));
    }

    hpx::future<primitive_argument_type> constant::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                util::generate_error_message(
                    "the constant primitive requires "
                        "at least one and at most 2 operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                util::generate_error_message(
                    "the constant primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](
                        primitive_argument_type&& op0, ir::range&& op1)
                ->  primitive_argument_type
                {
                    if (extract_numeric_value_dimension(op0) != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            util::generate_error_message(
                                "the first argument must be a literal "
                                    "scalar value",
                                this_->name_, this_->codename_));
                    }

                    if (op1.empty())
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::extract_num_dimensions",
                            util::generate_error_message(
                                "the constant primitive requires "
                                    "for the shape not to be empty",
                                this_->name_, this_->codename_));
                    }

                    if (op1.size() > 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::extract_num_dimensions",
                            util::generate_error_message(
                                "the constant primitive requires "
                                    "for the shape not to have more than "
                                    "two entries",
                                this_->name_, this_->codename_));
                    }

                    auto dims = detail::extract_dimensions(op1);
                    switch (detail::extract_num_dimensions(op1))
                    {
                    case 0:
                        return this_->constant0d(std::move(op0));

                    case 1:
                        return this_->constant1d(std::move(op0), dims[0]);

                    case 2:
                        return this_->constant2d(std::move(op0), dims);

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            util::generate_error_message(
                                "left hand side operand has unsupported "
                                    "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
                value_operand(operands[0], args, name_, codename_),
                list_operand(operands[1], args, name_, codename_));
        }

        // if constant() was invoked with one argument, we simply
        // provide the argument as the desired result
        return value_operand(operands[0], args, name_, codename_);
    }

    hpx::future<primitive_argument_type> constant::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
