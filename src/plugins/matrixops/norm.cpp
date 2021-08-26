// Copyright (c) 2020 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/norm.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/datastructures.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const norm::match_data = {
        match_pattern_type{"norm", std::vector<std::string>{R"(
                norm(
                    x,
                    __arg(_1_ord, nil),
                    __arg(_2_axis, nil),
                    __arg(_3_keepdims, false)
                )
            )"},
            &create_norm, &create_primitive<norm>, R"(
            x, ord, axis, keepdims
            Args:

                x (array_like) : Input array. If axis is None, x must be 1-D or
                    2-D, unless `ord` is None. If both `axis` and `ord` are None,
                    the 2-norm of x.ravel will be returned.
                ord (int, float, string, v) : non-zero int, `inf`, `-inf`,
                    'fro', or 'nuc'. Order of the norm. The default is None.
                axis (int, optional) : None, int, 2-tuple of ints.
                    If axis is an integer, it specifies the
                    axis of x along which to compute the vector norms. If axis
                    is a 2-tuple, it specifies the axes that hold 2-D matrices,
                    and the matrix norms of these matrices are computed. If
                    axis is None then either a vector norm (when x is 1-D) or
                    a matrix norm (when x is 2-D) is returned. The default is None.
                keepdims (bool, optional) : If this is set to True, the axes
                    which are normed over are left in the result as dimensions
                    with size one. With this option the result will broadcast
                    correctly against the original x. The default is false.

            Returns:

            Norm of the matrix or vector(s).)"}};

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        ir::node_data<T> apply_keepdims_vector(T value, std::uint8_t keepdims)
        {
            if (keepdims)
            {
                return ir::node_data<T>(blaze::DynamicVector<T>(1, value));
            }
            return ir::node_data<T>(value);
        }
    }

    template <typename T>
    T norm::frobenius_norm_vector(ir::node_data<T>&& data) const
    {
        return blaze::l2Norm(data.vector());
    }

    template <typename T>
    T norm::inf_norm_vector(ir::node_data<T>&& data) const
    {
        return blaze::linfNorm(data.vector());
    }

    template <typename T>
    T norm::ninf_norm_vector(ir::node_data<T>&& data) const
    {
        return (blaze::min)(blaze::abs(data.vector()));
    }

    template <typename T>
    T norm::norm_vector(ir::node_data<T>&& data, int ord) const
    {
        switch (ord)
        {
        case 0:
            return blaze::nonZeros(data.vector());

        case 1:
            return blaze::l1Norm(data.vector());

        case 2:
            return blaze::l2Norm(data.vector());

        case 3:
            return blaze::l3Norm(data.vector());

        case 4:
            return blaze::l4Norm(data.vector());

        default:
            break;
        }
        return blaze::lpNorm(data.vector(), ord);
    }

    template <typename T>
    primitive_argument_type norm::norm_helper_vector(ir::node_data<T>&& data,
        ord_type type, hpx::util::optional<int>&& ord,
        hpx::util::optional<int>&& axis, std::uint8_t keepdims,
        eval_context ctx) const
    {
        if (axis && (*axis != 0 && *axis != -1))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::norm_helper_vector",
                generate_error_message(
                    hpx::util::format(
                        "axis {} is out of bounds for array of dimension 1",
                        *axis),
                    std::move(ctx)));
        }

        switch (type)
        {
        case ord_type::default_frobenius:
            return detail::apply_keepdims_vector(
                frobenius_norm_vector(std::move(data)), keepdims);

        case ord_type::inf:
            return detail::apply_keepdims_vector(
                inf_norm_vector(std::move(data)), keepdims);

        case ord_type::ninf:
            return detail::apply_keepdims_vector(
                ninf_norm_vector(std::move(data)), keepdims);

        case ord_type::integer:
            return detail::apply_keepdims_vector(
                norm_vector(std::move(data), *ord), keepdims);

        case ord_type::frobenius:
            HPX_FALLTHROUGH;
        case ord_type::nuclear:
            HPX_FALLTHROUGH;
        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::norm_helper_vector",
            generate_error_message(
                "unsupported requested norm type", std::move(ctx)));
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        ir::node_data<T> apply_keepdims_matrix(T value, std::uint8_t keepdims)
        {
            if (keepdims)
            {
                return ir::node_data<T>(blaze::DynamicMatrix<T>(1, 1, value));
            }
            return ir::node_data<T>(value);
        }
    }

    template <typename T>
    T norm::frobenius_norm_matrix(ir::node_data<T>&& data) const
    {
        return blaze::l2Norm(data.matrix());
    }

    template <typename T>
    T norm::norm_matrix(
        ir::node_data<T>&& data, int ord, eval_context ctx) const
    {
        switch (ord)
        {
        case 2:
            return blaze::l2Norm(data.matrix());

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::norm_matrix",
            generate_error_message(
                "Invalid (or not implemented) norm order for matrices",
                std::move(ctx)));
    }

    template <typename T>
    primitive_argument_type norm::norm_helper_matrix(ir::node_data<T>&& data,
        ord_type type, hpx::util::optional<int>&& ord,
        hpx::util::optional<int>&& axis, std::uint8_t keepdims,
        eval_context ctx) const
    {
        if (axis)
        {
            if (*axis > 1 || *axis < -2)
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "norm::norm_helper_matrix",
                    generate_error_message(
                        hpx::util::format("axis {} is out of bounds for "
                                          "array of dimension 2",
                            *axis),
                        std::move(ctx)));
            }

            HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::norm_helper_matrix",
                generate_error_message(
                    hpx::util::format("axis {} is not implemented for "
                                      "array of dimension 2",
                        *axis),
                    std::move(ctx)));
        }

        switch (type)
        {
        case ord_type::default_frobenius:
            HPX_FALLTHROUGH;
        case ord_type::frobenius:
            return detail::apply_keepdims_matrix(
                frobenius_norm_matrix(std::move(data)), keepdims);

        case ord_type::integer:
            return detail::apply_keepdims_matrix(
                norm_matrix(std::move(data), *ord, std::move(ctx)), keepdims);

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::norm_helper_matrix",
            generate_error_message(
                "Invalid (or not implemented) norm type for matrices",
                std::move(ctx)));
    }

    template <typename T>
    primitive_argument_type norm::norm_helper(ir::node_data<T>&& data,
        ord_type type, hpx::util::optional<int>&& ord,
        hpx::util::optional<int>&& axis, std::uint8_t keepdims,
        eval_context ctx) const
    {
        if (extract_numeric_value_dimension(data, name_, codename_) == 1)
        {
            return norm_helper_vector(std::move(data), type, std::move(ord),
                std::move(axis), keepdims, std::move(ctx));
        }
        return norm_helper_matrix(std::move(data), type, std::move(ord),
            std::move(axis), keepdims, std::move(ctx));
    }

    primitive_argument_type norm::calculate_norm(primitive_argument_type&& data,
        ord_type type, hpx::util::optional<int>&& ord,
        hpx::util::optional<int>&& axis, std::uint8_t keepdims,
        eval_context ctx) const
    {
        switch (extract_common_type(data))
        {
        case node_data_type_bool:
            return norm_helper(
                extract_boolean_value_strict(std::move(data), name_, codename_),
                type, std::move(ord), std::move(axis), keepdims,
                std::move(ctx));

        case node_data_type_int64:
            return norm_helper(
                extract_integer_value_strict(std::move(data), name_, codename_),
                type, std::move(ord), std::move(axis), keepdims,
                std::move(ctx));

        case node_data_type_double:
            return norm_helper(
                extract_numeric_value_strict(std::move(data), name_, codename_),
                type, std::move(ord), std::move(axis), keepdims,
                std::move(ctx));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::calculate_norm",
            generate_error_message(
                "target operand has unsupported type", std::move(ctx)));
    }

    ///////////////////////////////////////////////////////////////////////////
    norm::norm(primitive_arguments_type&& args, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(args), name, codename)
    {
    }

    hpx::future<primitive_argument_type> norm::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 1 || operands.size() > 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::norm",
                generate_error_message(
                    "the norm primitive requires between one and four "
                    "arguments.",
                    std::move(ctx)));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "norm::eval",
                generate_error_message(
                    "at least one of the arguments passed to norm is "
                    "not valid",
                    std::move(ctx)));
        }

        auto ctx_copy = ctx;

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::unwrapping(
            [this_ = std::move(this_), ctx = std::move(ctx)](
                primitive_arguments_type&& args) mutable
            -> primitive_argument_type
            {
                if (!is_numeric_operand(args[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::norm::eval",
                        this_->generate_error_message(
                            "the first argument to the norm primitive "
                            "must be a numeric data type (vector or matrix)",
                            std::move(ctx)));
                }

                std::size_t dims = extract_numeric_value_dimension(
                    args[0], this_->name_, this_->codename_);
                if (dims != 1 && dims != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::norm::eval",
                        this_->generate_error_message(
                            "the first argument to the norm primitive "
                            "must be a numeric data type (vector or matrix)",
                            std::move(ctx)));
                }

                ord_type type = ord_type::default_frobenius;
                hpx::util::optional<int> ord;
                if (args.size() > 1 && valid(args[1]))
                {
                    if (is_integer_operand_strict(args[1]))
                    {
                        type = ord_type::integer;
                        ord = static_cast<int>(
                            extract_scalar_integer_value_strict(
                                std::move(args[1]), this_->name_,
                                this_->codename_));
                    }
                    else if (is_numeric_operand_strict(args[1]))
                    {
                        type = ord_type::integer;
                        auto&& ord_value = extract_numeric_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);
                        if (ord_value.num_dimensions() != 0)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::primitives::norm::"
                                "eval",
                                this_->generate_error_message(
                                    "the ord argument to the norm primitive "
                                    "must be a scalar floating point value",
                                    std::move(ctx)));
                        }

                        double val = ord_value.scalar();
                        if (val == std::numeric_limits<double>::infinity())
                        {
                            type = ord_type::inf;
                        }
                        else if (val ==
                            -std::numeric_limits<double>::infinity())
                        {
                            type = ord_type::ninf;
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::primitives::norm::"
                                "eval",
                                this_->generate_error_message(
                                    "the ord argument to the norm primitive "
                                    "must be 'inf' or 'ninf' ('-inf')",
                                    std::move(ctx)));
                        }
                    }
                    else if (is_string_operand_strict(args[1]))
                    {
                        std::string ord_value = extract_string_value_strict(
                            std::move(args[1]), this_->name_, this_->codename_);
                        if (ord_value == "nuc")
                        {
                            type = ord_type::nuclear;
                        }
                        else if (ord_value == "fro")
                        {
                            type = ord_type::frobenius;
                        }
                        else
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "phylanx::execution_tree::primitives::norm::"
                                "eval",
                                this_->generate_error_message(
                                    "the ord argument to the norm primitive "
                                    "must be either 'nuc' or 'fro'",
                                    std::move(ctx)));
                        }
                    }
                    else
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "phylanx::execution_tree::primitives::norm::eval",
                            this_->generate_error_message(
                                "the ord argument to the norm primitive must "
                                "be either and int, a string or a floating "
                                "point scalar value",
                                std::move(ctx)));
                    }
                }

                hpx::util::optional<int> axis;
                if (args.size() > 2 && valid(args[2]))
                {
                    axis = extract_scalar_integer_value_strict(
                        std::move(args[2]), this_->name_, this_->codename_);
                }

                std::uint8_t keepdims = 0;
                if (args.size() > 3 && valid(args[3]))
                {
                    keepdims = extract_scalar_boolean_value_strict(
                        std::move(args[3]), this_->name_, this_->codename_);
                }

                return this_->calculate_norm(std::move(args[0]), type,
                    std::move(ord), std::move(axis), keepdims, std::move(ctx));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx_copy)));
    }
}}}    // namespace phylanx::execution_tree::primitives
