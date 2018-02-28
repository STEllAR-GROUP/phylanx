//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/div_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
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
    primitive create_div_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("__div");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const div_operation::match_data =
    {
        hpx::util::make_tuple("__div",
            std::vector<std::string>{"_1 / __2"},
            &create_div_operation, &create_primitive<div_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    div_operation::div_operation(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    struct div_operation::divndnd_simd
    {
        divndnd_simd() = default;

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a, T const& b) const
        ->  decltype(a / b)
        {
            return a / b;
        }

        template <typename T1, typename T2>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDDiv<T1, T2>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(
            T const& a, T const& b) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return a / b;
        }
    };

    struct div_operation::divnd0d_simd
    {
    public:
        explicit divnd0d_simd(double scalar)
            : scalar_(scalar)
        {
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
        ->  decltype(a / std::declval<double>())
        {
            return a / scalar_;
        }

        template <typename T>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDDiv<T, double>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return a / blaze::set(scalar_);
        }

    private:
        double scalar_;
    };

    struct div_operation::div0dnd_simd
    {
    public:
        explicit div0dnd_simd(double scalar)
            : scalar_(scalar)
        {
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE auto operator()(T const& a) const
        ->  decltype(std::declval<double>() / a)
        {
            return scalar_ / a;
        }

        template <typename T>
        static constexpr bool simdEnabled()
        {
            return blaze::HasSIMDDiv<T, double>::value;
        }

        template <typename T>
        BLAZE_ALWAYS_INLINE decltype(auto) load(T const& a) const
        {
            BLAZE_CONSTRAINT_MUST_BE_SIMD_PACK(T);
            return blaze::set(scalar_) / a;
        }

    private:
        double scalar_;
    };

    ///////////////////////////////////////////////////////////////////////

    primitive_argument_type div_operation::div0d0d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        if (ops.size() == 2)
        {
            lhs.scalar() /= rhs.scalar();
            return primitive_argument_type(std::move(lhs));
        }

        return primitive_argument_type(std::accumulate(
            ops.begin() + 1, ops.end(), std::move(lhs),
            [](operand_type& result, operand_type const& curr)
            ->  operand_type
            {
                result[0] /= curr[0];
                return std::move(result);
            }));
    }

    primitive_argument_type div_operation::div0d1d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d1d",
                execution_tree::generate_error_message(
                    "the div_operation primitive can div a single "
                        "value to a vector only if there are exactly "
                        "two operands",
                    name_, codename_));
        }

        ops[1] =
            blaze::map(ops[1].vector(), div0dnd_simd(ops[0].scalar()));
        return primitive_argument_type(std::move(ops[1]));
    }

    primitive_argument_type div_operation::div0d2d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d2d",
                execution_tree::generate_error_message(
                    "the div_operation primitive can div a single "
                        "value to a matrix only if there are exactly "
                        "two operands",
                    name_, codename_));
        }

        ops[1] =
            blaze::map(ops[1].matrix(), div0dnd_simd(ops[0].scalar()));
        return primitive_argument_type(std::move(ops[1]));
    }

    primitive_argument_type div_operation::div0d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div0d0d(std::move(ops));

        case 1:
            return div0d1d(std::move(ops));

        case 2:
            return div0d2d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div0d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type div_operation::div1d0d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d0d",
                execution_tree::generate_error_message(
                    "the div_operation primitive can div a single "
                        "value to a vector only if there are exactly "
                        "two operands",
                    name_, codename_));
        }

        ops[0] =
            blaze::map(ops[0].vector(), divnd0d_simd(ops[1].scalar()));
        return primitive_argument_type(std::move(ops[0]));
    }

    primitive_argument_type div_operation::div1d1d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d1d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        if (ops.size() == 2)
        {
            if (lhs.is_ref())
            {
                lhs = blaze::map(lhs.vector(), rhs.vector(), divndnd_simd());
            }
            else
            {
                lhs.vector() = blaze::map(
                    lhs.vector(), rhs.vector(), divndnd_simd());
            }
                
            return primitive_argument_type(std::move(lhs));
        }

        operand_type& first_term = *ops.begin();
        return primitive_argument_type(std::accumulate(ops.begin() + 1,
            ops.end(), std::move(first_term),
            [](operand_type& result,
                operand_type const& curr) -> operand_type
            {
                if (result.is_ref())
                {
                    result = blaze::map(
                        result.vector(), curr.vector(), divndnd_simd());
                }
                else
                {
                    result.vector() = blaze::map(
                        result.vector(), curr.vector(), divndnd_simd());
                }
                return std::move(result);
            }));
    }

    primitive_argument_type div_operation::div1d2d(operands_type&& ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d2d",
                execution_tree::generate_error_message(
                    "the div_operation primitive can divide a vector "
                        "to a matrix only if there are exactly "
                        "two operands",
                    name_, codename_));
        }

        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        auto cv = lhs.vector();
        auto cm = rhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d2d",
                execution_tree::generate_error_message(
                    "vector size does not match number of matrix "
                        "columns",
                    name_, codename_));
        }

        // TODO: Blaze does not support broadcasting
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cv.size()};
            for (std::size_t i = 0; i < cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::trans(cv) / blaze::row(cm, i);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i < cm.rows(); ++i)
        {
            blaze::row(cm, i) = blaze::trans(cv) / blaze::row(cm, i);
        }
        return primitive_argument_type{std::move(rhs)};
    }

    primitive_argument_type div_operation::div1d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();

        switch(rhs_dims)
        {
        case 0:
            return div1d0d(std::move(ops));

        case 1:
            return div1d1d(std::move(ops));

        case 2:
            return div1d2d(std::move(ops));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    primitive_argument_type div_operation::div2d0d(operands_type && ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d0d",
                execution_tree::generate_error_message(
                    "the div_operation primitive can div a single "
                        "value to a matrix only if there are exactly "
                        "two operands",
                    name_, codename_));
        }

        ops[0] =
            blaze::map(ops[0].matrix(), divnd0d_simd(ops[1].scalar()));
        return primitive_argument_type(std::move(ops[0]));
    }

    primitive_argument_type div_operation::div2d1d(operands_type&& ops) const
    {
        if (ops.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d1d",
                execution_tree::generate_error_message(
                    "the div_operation primitive can divide a matrix "
                        "to a vector only if there are exactly "
                        "two operands",
                    name_, codename_));
        }

        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        auto cv = rhs.vector();
        auto cm = lhs.matrix();

        if (cv.size() != cm.columns())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div1d2d",
                execution_tree::generate_error_message(
                    "vector size does not match number of matrix "
                        "columns",
                    name_, codename_));
        }

        // TODO: Blaze does not support broadcasting
        if (rhs.is_ref())
        {
            blaze::DynamicMatrix<double> m{cm.rows(), cv.size()};
            for (std::size_t i = 0; i < cm.rows(); ++i)
            {
                blaze::row(m, i) = blaze::row(cm, i) / blaze::trans(cv);
            }
            return primitive_argument_type{std::move(m)};
        }

        for (std::size_t i = 0; i < cm.rows(); ++i)
        {
            blaze::row(cm, i) /= blaze::trans(cv);
        }
        return primitive_argument_type{std::move(lhs)};
    }

    primitive_argument_type div_operation::div2d2d(operands_type && ops) const
    {
        operand_type& lhs = ops[0];
        operand_type& rhs = ops[1];

        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d2d",
                execution_tree::generate_error_message(
                    "the dimensions of the operands do not match",
                    name_, codename_));
        }

        if (ops.size() == 2)
        {
            lhs.matrix() =
                blaze::map(lhs.matrix(), rhs.matrix(), divndnd_simd());
            return primitive_argument_type(std::move(lhs));
        }

        operand_type& first_term = *ops.begin();
        return primitive_argument_type(std::accumulate(ops.begin() + 1,
            ops.end(), std::move(first_term),
            [](operand_type& result,
                operand_type const& curr) -> operand_type
            {
                result.matrix() = blaze::map(
                    result.matrix(), curr.matrix(), divndnd_simd());
                return std::move(result);
            }));
    }

    primitive_argument_type div_operation::div2d(operands_type && ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return div2d0d(std::move(ops));

        case 2:
            return div2d2d(std::move(ops));

        case 1:
            return div2d1d(std::move(ops));
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::div2d",
                execution_tree::generate_error_message(
                    "the operands have incompatible number of "
                        "dimensions",
                    name_, codename_));
        }
    }

    hpx::future<primitive_argument_type> div_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "div_operation::eval",
                execution_tree::generate_error_message(
                    "the div_operation primitive requires at least "
                        "two operands",
                    name_, codename_));
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
                "div_operation::eval",
                execution_tree::generate_error_message(
                    "the div_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](operands_type&& ops) -> primitive_argument_type
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return this_->div0d(std::move(ops));

                case 1:
                    return this_->div1d(std::move(ops));

                case 2:
                    return this_->div2d(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "div_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    // implement '/' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> div_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
