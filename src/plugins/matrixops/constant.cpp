//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
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
                    result[0] = extract_scalar_nonneg_integer_value_strict(
                        *shape.begin());
                }
                else if (shape.size() == 2)
                {
                    auto elem_1 = shape.begin();
                    result[0] =
                        extract_scalar_nonneg_integer_value_strict(*elem_1);
                    result[1] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                }
                else if (shape.size() == 3)
                {
                    auto elem_1 = shape.begin();
                    result[0] =
                        extract_scalar_nonneg_integer_value_strict(*elem_1);
                    result[1] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                    result[2] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                }
                else if (shape.size() == 4)
                {
                    auto elem_1 = shape.begin();
                    result[0] =
                        extract_scalar_nonneg_integer_value_strict(*elem_1);
                    result[1] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                    result[2] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
                    result[3] =
                        extract_scalar_nonneg_integer_value_strict(*++elem_1);
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
        primitive_argument_type&& op, node_data_type dtype) const
    {
        if (dtype == node_data_type_unknown)
        {
            HPX_ASSERT(implements_like_operations_);
            dtype = extract_common_type(op);
        }

        switch (dtype)
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
            return ir::node_data<T>{blaze::UniformVector<T>(
                dim, extract_scalar_data<T>(std::move(op), name_, codename_))};
        }

        // create an empty vector
        return ir::node_data<T>{blaze::UniformVector<T>(dim)};
    }

    primitive_argument_type constant::constant1d(primitive_argument_type&& op,
        std::size_t dim, node_data_type dtype) const
    {
        if (dtype == node_data_type_unknown)
        {
            HPX_ASSERT(implements_like_operations_);
            dtype = extract_common_type(op);
        }

        switch (dtype)
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
            return ir::node_data<T>{blaze::UniformMatrix<T>(dim[0], dim[1],
                extract_scalar_data<T>(std::move(op), name_, codename_))};
        }

        // create an empty matrix
        return ir::node_data<T>{blaze::UniformMatrix<T>(dim[0], dim[1])};
    }

    primitive_argument_type constant::constant2d(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, node_data_type dtype) const
    {
        if (dtype == node_data_type_unknown)
        {
            HPX_ASSERT(implements_like_operations_);
            dtype = extract_common_type(op);
        }

        switch (dtype)
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

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant3d_helper(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim) const
    {
        if (valid(op))
        {
            return ir::node_data<T>{
                blaze::UniformTensor<T>(dim[0], dim[1], dim[2],
                    extract_scalar_data<T>(std::move(op), name_, codename_))};
        }

        // create an empty tensor
        return ir::node_data<T>{
            blaze::UniformTensor<T>(dim[0], dim[1], dim[2])};
    }

    primitive_argument_type constant::constant3d(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, node_data_type dtype) const
    {
        if (dtype == node_data_type_unknown)
        {
            HPX_ASSERT(implements_like_operations_);
            dtype = extract_common_type(op);
        }

        switch (dtype)
        {
        case node_data_type_bool:
            return constant3d_helper<std::uint8_t>(std::move(op), dim);

        case node_data_type_int64:
            return constant3d_helper<std::int64_t>(std::move(op), dim);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant3d_helper<double>(std::move(op), dim);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant3d",
            generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T>
    ir::node_data<T> constant::constant4d_helper(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim) const
    {
        if (valid(op))
        {
            auto a = ir::node_data<T>{
                blaze::DynamicArray<4UL, T>(blaze::init_from_value,
                    extract_scalar_data<T>(std::move(op), name_, codename_),
                    dim[0], dim[1], dim[2], dim[3])};
            auto b = extract_scalar_data<T>(std::move(op), name_, codename_);
            return ir::node_data<T>{
                blaze::DynamicArray<4UL, T>(blaze::init_from_value,
                    extract_scalar_data<T>(std::move(op), name_, codename_),
                    dim[0], dim[1], dim[2], dim[3])};
        }

        // create an empty 4d array
        return ir::node_data<T>{
            blaze::DynamicArray<4UL, T>(dim[0], dim[1], dim[2], dim[3])};
    }

    primitive_argument_type constant::constant4d(primitive_argument_type&& op,
        operand_type::dimensions_type const& dim, node_data_type dtype) const
    {
        if (dtype == node_data_type_unknown)
        {
            HPX_ASSERT(implements_like_operations_);
            dtype = extract_common_type(op);
        }

        switch (dtype)
        {
        case node_data_type_bool:
            return constant4d_helper<std::uint8_t>(std::move(op), dim);

        case node_data_type_int64:
            return constant4d_helper<std::int64_t>(std::move(op), dim);

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return constant4d_helper<double>(std::move(op), dim);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "phylanx::execution_tree::primitives::"
                "constant::constant4d",
            generate_error_message(
                "the constant primitive requires for all arguments to "
                    "be numeric data types"));
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
                        dims[0] = extract_scalar_nonneg_integer_value_strict(
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

                switch (numdims)
                {
                case 0:
                    return this_->constant0d(std::move(value), dtype);

                case 1:
                    return this_->constant1d(std::move(value), dims[0], dtype);

                case 2:
                    return this_->constant2d(std::move(value), dims, dtype);

                case 3:
                    return this_->constant3d(std::move(value), dims, dtype);

                case 4:
                    return this_->constant4d(std::move(value), dims, dtype);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "constant::eval",
                        this_->generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions"));
                }
            }),
            value_operand(std::move(ops[0]), args, name_, codename_, ctx),
            value_operand(std::move(ops[1]), args, name_, codename_, ctx),
            value_operand(std::move(ops[2]), args, name_, codename_, ctx));
    }
}}}
