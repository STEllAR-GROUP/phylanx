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

        std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> extract_dimensions(
            ir::range const& shape)
        {
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result = {1, 1};
            if (!shape.empty())
            {
                if (shape.size() == 1)
                {
                    result[1] = extract_scalar_integer_value(*shape.begin());
                }
                else if (shape.size() == 2)
                {
                    auto elem_1 = shape.begin();
                    result[0] = extract_scalar_integer_value(*elem_1);
                    result[1] = extract_scalar_integer_value(*++elem_1);
                }
            }
            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const constant::match_data =
    {
        match_pattern_type{"constant",
            std::vector<std::string>{"constant(_1, _2)", "constant(_1)"},
            &create_constant, &create_primitive<constant>, R"(
            arg1, arg2
            Args:

                arg1 (float): a constant value
                arg2 (int or shape, optional): the number of values

            Returns:

            An array of size arg2 with each element equal to arg1.)",
            true
        },
        match_pattern_type{"constant_like",
            std::vector<std::string>{
                "constant_like(_1, _2)", "constant_like(_1)"
            },
            &create_constant, &create_primitive<constant>, R"(
            arg1, arg2
            Args:

                arg1 (float): a constant value
                arg2 (array-like): the shape of this array-like will be used as
                                   to determine the shape of the result

            Returns:

            An array of the same size as arg2 with each element equal to arg1.)",
            true
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        bool extract_if_like(std::string const& name)
        {
            compiler::primitive_name_parts name_parts;
            if (compiler::parse_primitive_name(name, name_parts))
            {
                return name_parts.primitive.find("constant_like") == 0;
            }
            return name.find("constant_like") == 0;
        }
    }

    constant::constant(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
      , implements_like_operations_(detail::extract_if_like(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant0d_helper(
        primitive_argument_type&& op) const
    {
        if (valid(op))
        {
            return ir::node_data<T>{
                extract_scalar_data<T>(std::move(op), name_, codename_)};
        }

        // create an empty scalar
        return ir::node_data<T>{T{}};
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
            generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant1d_helper(
        primitive_argument_type&& op, std::size_t dim) const
    {
        if (valid(op))
        {
            return ir::node_data<T>{blaze::DynamicVector<T>(
                dim, extract_scalar_data<T>(std::move(op), name_, codename_))};
        }

        // create an empty vector
        return ir::node_data<T>{blaze::DynamicVector<T>(dim)};
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

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant1d_helper<double>(std::move(op), dim);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant1d",
            generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant2d_helper(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim) const
    {
        if (valid(op))
        {
            return ir::node_data<T>{blaze::DynamicMatrix<T>(dim[0], dim[1],
                extract_scalar_data<T>(std::move(op), name_, codename_))};
        }

        // create an empty matrix
        return ir::node_data<T>{blaze::DynamicMatrix<T>(dim[0], dim[1])};
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

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant2d_helper<double>(std::move(op), dim);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant2d",
            generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    hpx::future<primitive_argument_type> constant::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                generate_error_message(
                    "the constant primitive requires "
                        "at least one and at most 2 operands"));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                generate_error_message(
                    "the constant primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_argument_type&& op0,
                        primitive_argument_type&& op1)
                ->  primitive_argument_type
                {
                    if (extract_numeric_value_dimension(op0) != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            this_->generate_error_message(
                                "the first argument must be a literal "
                                    "scalar value"));
                    }

                    std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims;
                    std::size_t numdims = 0;
                    if (is_list_operand_strict(op1))
                    {
                        if (this_->implements_like_operations_)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "constant::eval",
                                this_->generate_error_message(
                                    "for constant_like, the second argument "
                                        "must be an array-like value"));
                        }

                        ir::range&& r = extract_list_value_strict(
                            std::move(op1), this_->name_, this_->codename_);
                        if (r.size() > 2)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "constant::eval",
                                this_->generate_error_message(
                                    "the constant primitive requires "
                                    "for the shape not to have more than "
                                    "two entries"));
                        }

                        dims = detail::extract_dimensions(r);
                        numdims = detail::extract_num_dimensions(r);
                    }
                    else if (is_numeric_operand(op1))
                    {
                        if (this_->implements_like_operations_)
                        {
                            // support ..._like operations
                            dims = extract_numeric_value_dimensions(op1);
                            numdims = extract_numeric_value_dimension(op1);
                        }
                        else
                        {
                            dims[1] = extract_scalar_integer_value(
                                std::move(op1), this_->name_, this_->codename_);
                            numdims = 1;
                        }
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            this_->generate_error_message(
                                "the constant primitive requires "
                                "for the second argument to be either a shape "
                                "or a numeric reference value"));
                    }

                    switch (numdims)
                    {
                    case 0:
                        return this_->constant0d(std::move(op0));

                    case 1:
                        return this_->constant1d(std::move(op0), dims[1]);

                    case 2:
                        return this_->constant2d(std::move(op0), dims);

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            this_->generate_error_message(
                                "left hand side operand has unsupported "
                                    "number of dimensions"));
                    }
                }),
                value_operand(operands[0], args, name_, codename_, ctx),
                value_operand(operands[1], args, name_, codename_, ctx));
        }

        // support empty/empty_like
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& op0)
            ->  primitive_argument_type
            {
                primitive_argument_type value;
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims{1, 1};
                std::size_t numdims = 0;

                if (is_list_operand_strict(op0))
                {
                    // if the first argument is a list of up to two values
                    // (shape) this creates an empty array of the given size
                    if (this_->implements_like_operations_)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            this_->generate_error_message(
                                "for constant_like, the first argument "
                                "must be an array-like value"));
                    }

                    ir::range&& r = extract_list_value_strict(
                        std::move(op0), this_->name_, this_->codename_);

                    if (r.size() > 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            this_->generate_error_message(
                                "the constant primitive requires "
                                "for the shape not to have more than "
                                "two entries"));
                    }

                    dims = detail::extract_dimensions(r);
                    numdims = detail::extract_num_dimensions(r);
                }
                else if (is_numeric_operand(op0))
                {
                    if (this_->implements_like_operations_)
                    {
                        // support empty_like operations
                        dims = extract_numeric_value_dimensions(op0);
                        numdims = extract_numeric_value_dimension(op0);
                    }
                    else
                    {
                        // support constant(42) == 42
                        numdims = 0;
                        value = std::move(op0);
                    }
                }
                else
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "constant::eval",
                        this_->generate_error_message(
                            "the constant primitive requires "
                            "for the second argument to be either a shape "
                            "or a numeric reference value"));
                }

                switch (numdims)
                {
                case 0:
                    return this_->constant0d(std::move(value));

                case 1:
                    return this_->constant1d(std::move(value), dims[1]);

                case 2:
                    return this_->constant2d(std::move(value), dims);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "constant::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            value_operand(operands[0], args, name_, codename_, std::move(ctx)));
    }
}}}
