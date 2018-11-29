// Copyright (c) 2017-2018 Hartmut Kaiser
// Copyright (c) 2018 Shahrzad Shirzad
// Copyright (c) 2018 Parsa Amini
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_NUMERIC_IMPL_OCT_31_2018_0138PM)
#define PHYLANX_PRIMITIVES_NUMERIC_IMPL_OCT_31_2018_0138PM

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/arithmetics/numeric.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
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
    template <typename Op, typename Derived>
    numeric<Op, Derived>::numeric(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , dtype_(extract_dtype(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric0d0d(
        arg_type<T>&& lhs, arg_type<T>&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when
        // possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = Op{}(lhs.scalar(), rhs.scalar());
            }
            else
            {
                // Reuse the memory from rhs operand
                rhs.scalar() = Op{}(lhs.scalar(), rhs.scalar());
            }
            return primitive_argument_type(std::move(rhs));
        }

        // Reuse the memory from lhs operand
        auto& s = lhs.scalar();
        Op{}.op_assign(s, rhs.scalar());

        return primitive_argument_type(std::move(lhs));
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric0d0d(args_type<T> && args) const
    {
        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type<T>& result, arg_type<T> const& curr) -> arg_type<T>
            {
                if (result.is_ref())
                {
                    result = Op{}(result.scalar(), curr.scalar());
                    return std::move(result);
                }
                else
                {
                    auto& s = result.scalar();
                    Op{}.op_assign(s, curr.scalar());
                }
                return std::move(result);
            })};
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric1d1d(
        arg_type<T>&& lhs, arg_type<T>&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when
        // possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = Op{}(lhs.vector(), rhs.vector());
            }
            else
            {
                // Reuse the memory from rhs operand
                rhs.vector() = Op{}(lhs.vector(), rhs.vector());
            }
            return primitive_argument_type(std::move(rhs));
        }

        // Reuse the memory from lhs operand
        auto v = lhs.vector();
        Op{}.op_assign(v, rhs.vector());

        return primitive_argument_type(std::move(lhs));
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric1d1d(args_type<T>&& args) const
    {
        return primitive_argument_type(std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type<T>& result, arg_type<T> const& curr) -> arg_type<T>
            {
                if (result.is_ref())
                {
                    result = Op{}(result.vector(), curr.vector());
                    return std::move(result);
                }
                else
                {
                    auto v = result.vector();
                    Op{}.op_assign(v, curr.vector());
                }
                return std::move(result);
            }));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric2d2d(
        arg_type<T>&& lhs, arg_type<T>&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = Op{}(lhs.matrix(), rhs.matrix());
            }
            else
            {
                // Reuse the memory from rhs operand
                rhs.matrix() = Op{}(lhs.matrix(), rhs.matrix());
            }
            return primitive_argument_type(std::move(rhs));
        }

        // Reuse the memory from lhs operand
        auto m = lhs.matrix();
        Op{}.op_assign(m, rhs.matrix());

        return primitive_argument_type(std::move(lhs));
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric2d2d(args_type<T> && args) const
    {
        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type<T>& result, arg_type<T> const& curr) -> arg_type<T>
            {
                if (result.is_ref())
                {
                    result = Op{}(result.matrix(), curr.matrix());
                }
                else
                {
                    auto m = result.matrix();
                    Op{}.op_assign(m, curr.matrix());
                }
                return std::move(result);
            })};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric3d3d(
        arg_type<T>&& lhs, arg_type<T>&& rhs) const
    {
        // Avoid overwriting references, avoid memory reallocation when possible
        if (lhs.is_ref())
        {
            // Cannot reuse the memory if an operand is a reference
            if (rhs.is_ref())
            {
                rhs = Op{}(lhs.tensor(), rhs.tensor());
            }
            else
            {
                // Reuse the memory from rhs operand
                rhs.tensor() = Op{}(lhs.tensor(), rhs.tensor());
            }
            return primitive_argument_type(std::move(rhs));
        }

        // Reuse the memory from lhs operand
        auto m = lhs.matrix();
        Op{}.op_assign(m, rhs.matrix());

        return primitive_argument_type(std::move(lhs));
    }

    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::numeric3d3d(
        args_type<T>&& args) const
    {
        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(args[0]),
            [](arg_type<T>& result, arg_type<T> const& curr) -> arg_type<T>
            {
                if (result.is_ref())
                {
                    result = Op{}(result.tensor(), curr.tensor());
                }
                else
                {
                    auto m = result.tensor();
                    Op{}.op_assign(m, curr.tensor());
                }
                return std::move(result);
            })};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type numeric<Op, Derived>::handle_numeric_operands_helper(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const
    {
        auto sizes = extract_largest_dimensions(name_, codename_, op1, op2);
        switch (extract_largest_dimension(name_, codename_, op1, op2))
        {
        case 0:
            {
                auto lhs = extract_value_scalar<T>(
                    std::move(op1), name_, codename_);
                auto rhs = extract_value_scalar<T>(
                    std::move(op2), name_, codename_);

                return numeric0d0d<T>(std::move(lhs), std::move(rhs));
            }

        case 1:
            {
                auto lhs = extract_value_vector<T>(
                    std::move(op1), sizes[1], name_, codename_);
                auto rhs = extract_value_vector<T>(
                    std::move(op2), sizes[1], name_, codename_);

                return numeric1d1d<T>(std::move(lhs), std::move(rhs));
            }

        case 2:
            {
                auto lhs = extract_value_matrix<T>(
                    std::move(op1), sizes[0], sizes[1], name_, codename_);
                auto rhs = extract_value_matrix<T>(
                    std::move(op2), sizes[0], sizes[1], name_, codename_);

                return numeric2d2d<T>(std::move(lhs), std::move(rhs));
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            {
                auto lhs = extract_value_tensor<T>(std::move(op1), sizes[0],
                    sizes[1], sizes[2], name_, codename_);
                auto rhs = extract_value_tensor<T>(std::move(op2), sizes[0],
                    sizes[1], sizes[2], name_, codename_);

                return numeric3d3d<T>(std::move(lhs), std::move(rhs));
            }
#endif

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "numeric::handle_numeric_operands",
            util::generate_error_message(
                "operands have unsupported number of dimensions",
                name_, codename_));
    }

    template <typename Op, typename Derived>
    primitive_argument_type numeric<Op, Derived>::handle_numeric_operands(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(op1, op2);
        }

        switch (t)
        {
        case node_data_type_bool:
            return derived()
                .template handle_numeric_operands_helper<std::uint8_t>(
                    std::move(op1), std::move(op2));

        case node_data_type_int64:
            return derived()
                .template handle_numeric_operands_helper<std::int64_t>(
                    std::move(op1), std::move(op2));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return derived().template handle_numeric_operands_helper<double>(
                std::move(op1), std::move(op2));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "numeric::handle_numeric_operands",
            generate_error_message("operand has unsupported type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    template <typename T>
    primitive_argument_type
    numeric<Op, Derived>::handle_numeric_operands_helper(
        primitive_arguments_type&& ops) const
    {
        auto sizes = extract_largest_dimensions(ops, name_, codename_);
        switch (extract_largest_dimension(ops, name_, codename_))
        {
        case 0:
            {
                args_type<T> args;
                args.reserve(ops.size());

                for (auto && op : std::move(ops))
                {
                    args.emplace_back(extract_value_scalar<T>(
                        std::move(op), name_, codename_));
                }

                return numeric0d0d<T>(std::move(args));
            }

        case 1:
            {
                args_type<T> args;
                args.reserve(ops.size());

                for (auto && op : std::move(ops))
                {
                    args.emplace_back(extract_value_vector<T>(
                        std::move(op), sizes[1], name_, codename_));
                }

                return numeric1d1d<T>(std::move(args));
            }

        case 2:
            {
                args_type<T> args;
                args.reserve(ops.size());

                for (auto && op : std::move(ops))
                {
                    args.emplace_back(extract_value_matrix<T>(
                        std::move(op), sizes[0], sizes[1], name_, codename_));
                }

                return numeric2d2d<T>(std::move(args));
            }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3:
            {
                args_type<T> args;
                args.reserve(ops.size());

                for (auto && op : std::move(ops))
                {
                    args.emplace_back(extract_value_tensor<T>(std::move(op),
                        sizes[0], sizes[1], sizes[2], name_, codename_));
                }

                return numeric3d3d<T>(std::move(args));
            }
#endif

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "numeric::handle_numeric_operands",
            generate_error_message(
                "operands have unsupported number of dimensions"));
    }

    template <typename Op, typename Derived>
    primitive_argument_type numeric<Op, Derived>::handle_numeric_operands(
        primitive_arguments_type&& ops) const
    {
        node_data_type t = dtype_;
        if (t == node_data_type_unknown)
        {
            t = extract_common_type(ops);
        }

        switch (t)
        {
        case node_data_type_bool:
            return derived()
                .template handle_numeric_operands_helper<std::uint8_t>(
                    std::move(ops));

        case node_data_type_int64:
            return derived()
                .template handle_numeric_operands_helper<std::int64_t>(
                    std::move(ops));

        case node_data_type_unknown: HPX_FALLTHROUGH;
        case node_data_type_double:
            return derived().template handle_numeric_operands_helper<double>(
                std::move(ops));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "numeric::handle_numeric_operands",
            generate_error_message("operand has unsupported type"));
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename Op, typename Derived>
    hpx::future<primitive_argument_type> numeric<Op, Derived>::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "numeric::eval",
                generate_error_message("the numeric primitive requires "
                    "at least two operands"));
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
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "numeric::eval",
                generate_error_message(
                    "the numeric primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            // special case for 2 operands
            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& lhs,
                    hpx::future<primitive_argument_type>&& rhs)
                -> primitive_argument_type
                {
                    return this_->handle_numeric_operands(lhs.get(), rhs.get());
                },
                value_operand(operands[0], args, name_, codename_, ctx),
                value_operand(operands[1], args, name_, codename_, ctx));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& ops)
            ->  primitive_argument_type
            {
                return this_->handle_numeric_operands(std::move(ops));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

#endif
