// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/arithmetics/add_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    namespace detail
    {
        struct add_simd
        {
        public:
            explicit add_simd(double scalar)
                : scalar_(scalar)
            {
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
                ->  decltype(a + std::declval<double>())
            {
                return a + scalar_;
            }

            template <typename T>
            static constexpr bool simdEnabled()
            {
                return blaze::HasSIMDAdd<T, double>::value;
            }

            template <typename T>
            BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
            {
                BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
                return a + blaze::set(scalar_);
            }

        private:
            double scalar_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const add_operation::match_data =
    {
        hpx::util::make_tuple("__add",
            std::vector<std::string>{"_1 + __2", "__add(_1, __2)"},
            &create_add_operation, &create_primitive<add_operation>)
    };

    //////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add0d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        lhs.scalar() += rhs.scalar();
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add0d0d(args_type && args) const
    {
        arg_type& lhs = args[0];
        arg_type& rhs = args[1];

        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(lhs),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                result.scalar() += curr.scalar();
                return std::move(result);
            })};
    }

    primitive_argument_type add_operation::add0d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.vector(), detail::add_simd(lhs.scalar()));
        }
        else
        {
            rhs.vector() =
                blaze::map(rhs.vector(), detail::add_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type add_operation::add0d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (rhs.is_ref())
        {
            rhs = blaze::map(rhs.matrix(), detail::add_simd(lhs.scalar()));
        }
        else
        {
            rhs.matrix() = blaze::map(
                rhs.matrix(), detail::add_simd(lhs.scalar()));
        }
        return primitive_argument_type(std::move(rhs));
    }

    primitive_argument_type add_operation::add0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add0d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add0d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add0d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                "the operands have incompatible number of dimensions");
        }
    }

    primitive_argument_type add_operation::add0d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add0d0d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add1d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.vector(), detail::add_simd(rhs.scalar()));
        }
        else
        {
            lhs.vector() =
                blaze::map(lhs.vector(), detail::add_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add1d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        if (lhs.is_ref())
        {
            lhs = lhs.vector() + rhs.vector();
        }
        else
        {
            lhs.vector() += rhs.vector();
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add1d1d(args_type && args) const
    {
        arg_type& lhs = args[0];
        arg_type& rhs = args[1];

        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        arg_type& first_term = *args.begin();
        return primitive_argument_type(std::accumulate(
            args.begin() + 1, args.end(), std::move(first_term),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.vector() + curr.vector();
                }
                else
                {
                    result.vector() += curr.vector();
                }
                return std::move(result);
            }));
    }

    primitive_argument_type add_operation::add1d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d2d",
                execution_tree::generate_error_message(
                    "vector size does not match number of matrix columns",
                    name_, codename_));
        }

        // TODO: Blaze does not support broadcasting
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cm.columns()};
            for (std::size_t i = 0; i != cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::row(cm, i) + blaze::trans(cv);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) += blaze::trans(cv);
        }

        return primitive_argument_type{std::move(rhs)};
    }

    primitive_argument_type add_operation::add1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();

        switch (rhs_dims)
        {
        case 0:
            return add1d0d(std::move(lhs), std::move(rhs));

        case 1:
            return add1d1d(std::move(lhs), std::move(rhs));

        case 2:
            return add1d2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add1d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();

        switch (rhs_dims)
        {
        case 1:
            return add1d1d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type add_operation::add2d0d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        if (lhs.is_ref())
        {
            lhs = blaze::map(lhs.matrix(), detail::add_simd(rhs.scalar()));
        }
        else
        {
            lhs.matrix() =
                blaze::map(lhs.matrix(), detail::add_simd(rhs.scalar()));
        }
        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add2d1d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d1d",
                execution_tree::generate_error_message(
                    "vector size does not match number of matrix columns",
                    name_, codename_));
        }

        // TODO: Blaze does not support broadcasting
        if (lhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{ cm.rows(), cm.columns() };
            for (std::size_t i = 0; i != cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::row(cm, i) + blaze::trans(cv);
            }
            return primitive_argument_type{ std::move(m) };
        }

        for (std::size_t i = 0; i != cm.rows(); ++i)
        {
            blaze::row(cm, i) += blaze::trans(cv);
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::add2d2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        if (lhs.is_ref())
        {
            lhs = lhs.matrix() + rhs.matrix();
        }
        else
        {
            lhs.matrix() += rhs.matrix();
        }

        return primitive_argument_type(std::move(lhs));
    }

    primitive_argument_type add_operation::add2d2d(args_type && args) const
    {
        arg_type& lhs = args[0];
        arg_type& rhs = args[1];

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        arg_type& first_term = *args.begin();
        return primitive_argument_type{std::accumulate(
            args.begin() + 1, args.end(), std::move(first_term),
            [](arg_type& result, arg_type const& curr) -> arg_type
            {
                if (result.is_ref())
                {
                    result = result.matrix() + curr.matrix();
                }
                else
                {
                    result.matrix() += curr.matrix();
                }
                return std::move(result);
            })};
    }

    primitive_argument_type add_operation::add2d(
        arg_type&& lhs, arg_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch (rhs_dims)
        {
        case 0:
            return add2d0d(std::move(lhs), std::move(rhs));

        case 2:
            return add2d2d(std::move(lhs), std::move(rhs));

        case 1:
            return add2d1d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type add_operation::add2d(args_type && args) const
    {
        std::size_t rhs_dims = args[1].num_dimensions();
        switch (rhs_dims)
        {
        case 2:
            return add2d2d(std::move(args));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of dimensions",
                    name_, codename_));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    void add_operation::append_element(
        std::vector<primitive_argument_type>& result,
        primitive_argument_type&& rhs) const
    {
        if (is_list_operand_strict(rhs))
        {
            auto&& rhs_list =
                extract_list_value_strict(std::move(rhs), name_, codename_);

            for (auto&& elem : std::move(rhs_list))
            {
                result.emplace_back(std::move(elem));
            }
        }
        else
        {
            result.emplace_back(std::move(rhs));
        }
    }

    primitive_argument_type add_operation::handle_list_operands(
        primitive_argument_type&& op1, primitive_argument_type&& rhs) const
    {
        ir::range lhs =
            extract_list_value_strict(std::move(op1), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            append_element(result, std::move(rhs));
            return primitive_argument_type{std::move(result)};
        }

        append_element(lhs.args(), std::move(rhs));
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::handle_list_operands(
        std::vector<primitive_argument_type>&& ops) const
    {
        auto it = ops.begin();
        auto end = ops.end();

        ir::range lhs =
            extract_list_value_strict(std::move(ops[0]), name_, codename_);

        if (lhs.is_ref())
        {
            auto result = lhs.copy();
            for (++it; it != end; ++it)
            {
                append_element(result, std::move(*it));
            }
            return primitive_argument_type{std::move(result)};
        }

        for (++it; it != end; ++it)
        {
            append_element(lhs.args(), std::move(*it));
        }

        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type add_operation::handle_numeric_operands(
        primitive_argument_type&& op1, primitive_argument_type&& op2) const
    {
        arg_type lhs = extract_numeric_value(std::move(op1), name_, codename_);
        arg_type rhs = extract_numeric_value(std::move(op2), name_, codename_);

        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return add0d(std::move(lhs), std::move(rhs));

        case 1:
            return add1d(std::move(lhs), std::move(rhs));

        case 2:
            return add2d(std::move(lhs), std::move(rhs));

        default:
            break;
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::handle_numeric_operands",
            generate_error_message(
                "left hand side operand has unsupported number of dimensions"));
    }

    primitive_argument_type add_operation::handle_numeric_operands(
        std::vector<primitive_argument_type>&& ops) const
    {
        args_type args;
        args.reserve(ops.size());

        for (auto && op : std::move(ops))
        {
            args.emplace_back(
                extract_numeric_value(std::move(op), name_, codename_));
        }

        std::size_t lhs_dims = args[0].num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return add0d(std::move(args));

        case 1:
            return add1d(std::move(args));

        case 2:
            return add2d(std::move(args));

        default:
            break;
        }
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "add_operation::handle_numeric_operands",
            generate_error_message(
                "left hand side operand has unsupported number of dimensions"));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> add_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "add_operation::eval",
                generate_error_message("the add_operation primitive requires "
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
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "add_operation::eval",
                generate_error_message(
                    "the add_operation primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            // special case for 2 operands
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_](primitive_argument_type&& lhs,
                        primitive_argument_type&& rhs)
                -> primitive_argument_type
                {
                    if (is_list_operand_strict(lhs))
                    {
                        return this_->handle_list_operands(
                            std::move(lhs), std::move(rhs));
                    }
                    return this_->handle_numeric_operands(
                        std::move(lhs), std::move(rhs));
                }),
                value_operand(operands[0], args, name_, codename_),
                value_operand(operands[1], args, name_, codename_));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](std::vector<primitive_argument_type>&& ops)
            ->  primitive_argument_type
            {
                if (is_list_operand_strict(ops[0]))
                {
                    return this_->handle_list_operands(std::move(ops));
                }
                return this_->handle_numeric_operands(std::move(ops));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    // Implement '+' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> add_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
