//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/common/constant_nd.hpp>
#include <phylanx/plugins/matrixops/constant.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

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
            std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> result = {0, 0};
            if (!shape.empty())
            {
                if (shape.size() == 1)
                {
                    result[0] =
                        extract_scalar_positive_integer_value(*shape.begin());
                }
                else if (shape.size() == 2)
                {
                    auto elem_1 = shape.begin();
                    result[0] = extract_scalar_positive_integer_value(*elem_1);
                    result[1] =
                        extract_scalar_positive_integer_value(*++elem_1);
                }
                else if (shape.size() == 3)
                {
                    auto elem_1 = shape.begin();
                    result[0] = extract_scalar_positive_integer_value(*elem_1);
                    result[1] =
                        extract_scalar_positive_integer_value(*++elem_1);
                    result[2] =
                        extract_scalar_positive_integer_value(*++elem_1);
                }
                else if (shape.size() == 4)
                {
                    auto elem_1 = shape.begin();
                    result[0] = extract_scalar_positive_integer_value(*elem_1);
                    result[1] =
                        extract_scalar_positive_integer_value(*++elem_1);
                    result[2] =
                        extract_scalar_positive_integer_value(*++elem_1);
                    result[3] =
                        extract_scalar_positive_integer_value(*++elem_1);
                }
            }
            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const constant::match_data =
    {
        match_pattern_type{"constant",
            std::vector<std::string>{R"(
                constant(
                    _1_value,
                    __arg(_2_shape, nil),
                    __arg(_3_dtype, "float64")
                )
            )"},
            &create_constant, &create_primitive<constant>, R"(
            value, shape, dtype
            Args:

                value (float): a constant value, if 'None' this operation
                  returns an uninitialized array-like, also in this case 'shape'
                  should not be 'None'
                shape (int or shape, optional): the number of values
                dtype (string, optional): the data-type of the returned array,
                  defaults to 'float'.

            Returns:

            An array of size 'shape' with each element equal to 'value'. If
            'value' is equal to None, the array elements are uninitialized.)"
        },
        match_pattern_type{"constant_like",
            std::vector<std::string>{R"(
                constant_like(
                    _1_value,
                    _2_a,
                    __arg(_3_dtype, nil)
                )
            )"},
            &create_constant, &create_primitive<constant>, R"(
            value, a, dtype
            Args:

                value (float): a constant value, if 'None' this operation
                  returns an uninitialized array-like
                a (array-like): the shape of this array-like will be used as
                                to determine the shape of the result
                dtype (string, optional): the data-type of the returned array,
                  defaults to 'float'.

            Returns:

            An array of the same size as 'a' with each element equal to
            'value'. If 'value' is equal to None, the array elements are
            uninitialized.)"
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
      , implements_like_operations_(detail::extract_if_like(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type constant::constant_nd(std::size_t numdims,
        primitive_argument_type&& value,
        operand_type::dimensions_type const& dims, node_data_type dtype,
        std::string const& name_, std::string const& codename_) const
    {
        switch (numdims)
        {
        case 0:
            return common::constant0d(
                std::move(value), dtype, name_, codename_);

        case 1:
            return common::constant1d(
                std::move(value), dims[0], dtype, name_, codename_);

        case 2:
            return common::constant2d(
                std::move(value), dims, dtype, name_, codename_);

        case 3:
            return common::constant3d(
                std::move(value), dims, dtype, name_, codename_);

        case 4:
            return common::constant4d(
                std::move(value), dims, dtype, name_, codename_);

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "constant::constant_nd",
            util ::generate_error_message(
                "left hand side operand has unsupported "
                "number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> constant::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        // verify arguments
        if ((implements_like_operations_ && operands.size() < 2) ||
            (!implements_like_operations_ && operands.empty()) ||
            operands.size() > 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                generate_error_message(
                    "the constant primitive requires "
                        "at least one and at most 3 operands"));
        }

        // support empty/empty_like, i.e. allow for operands[0] to be nil
        if (implements_like_operations_)
        {
            // for constant_like, reference array must be always be given
            if (!valid(operands[1]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "constant::eval",
                    generate_error_message(
                        "the constant primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }
        else if (!valid(operands[0]))
        {
            // for empty (operands[0] == nil), operands[1] (shape) must be valid
            if (operands.size() < 2 || !valid(operands[1]))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "constant::eval",
                    generate_error_message(
                        "the constant primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        // supply missing default arguments
        primitive_arguments_type ops = operands;
        if (operands.size() != 3)
        {
            ops.resize(3);      // fill rest with 'nil'
            if (!implements_like_operations_)
            {
                // default 'dtype' for constant is 'float64'
                ops[2] = primitive_argument_type{"float"};
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_argument_type&& value,
                    primitive_argument_type&& op1,
                    primitive_argument_type&& dtype_op)
            ->  primitive_argument_type
            {
                if (valid(value) && extract_numeric_value_dimension(value) != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "constant::eval",
                        this_->generate_error_message(
                            "the first argument must be a literal "
                            "scalar value"));
                }

                // if the second argument is a list of up to two values
                // (shape) this creates an empty array of the given size
                std::array<std::size_t, PHYLANX_MAX_DIMENSIONS> dims{0};
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

                    if (r.size() > PHYLANX_MAX_DIMENSIONS)
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
                        // support constant_like operations
                        dims = extract_numeric_value_dimensions(
                            op1, this_->name_, this_->codename_);
                        numdims = extract_numeric_value_dimension(
                            op1, this_->name_, this_->codename_);
                    }
                    else
                    {
                        // support constant(42, 3) == [42, 42, 42]
                        numdims = 1;
                        dims[0] = extract_scalar_integer_value(
                            std::move(op1), this_->name_, this_->codename_);
                    }
                }
                else
                {
                    // support constant(42) == 42
                    numdims = 0;
                }

                // constant_like() without dtype defaults type of operands
                node_data_type dtype = node_data_type_unknown;
                if (valid(dtype_op))
                {
                    dtype = map_dtype(extract_string_value(
                        std::move(dtype_op), this_->name_, this_->codename_));
                }

                // constant() without dtype defaults to float64
                if (!this_->implements_like_operations_ &&
                    dtype == node_data_type_unknown)
                {
                    dtype = node_data_type_double;
                }

                return this_->constant_nd(numdims, std::move(value), dims,
                    dtype, this_->name_, this_->codename_);

            }),
            value_operand(std::move(ops[0]), args, name_, codename_, ctx),
            value_operand(std::move(ops[1]), args, name_, codename_, ctx),
            value_operand(std::move(ops[2]), args, name_, codename_, ctx));
    }
}}}
