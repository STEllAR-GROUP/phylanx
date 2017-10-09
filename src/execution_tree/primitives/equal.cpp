//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/equal.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::equal>
    equal_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    equal_type, phylanx_equal_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(equal_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const equal::match_data =
    {
        "_1 == _2", &create<equal>
    };

    ///////////////////////////////////////////////////////////////////////////
    equal::equal(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal",
                "the equal primitive requires exactly two operands");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal",
                "the equal primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool equal::equal0d(operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return lhs[0] == rhs[0];

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool equal::equal1d1d(operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal1d1d",
                "the dimensions of the operands do not match");
        }

        lhs.matrix().array() =
            (lhs.matrix().array() == rhs.matrix().array()).cast<double>();

        return lhs.matrix().norm() != 0.0;
    }

    bool equal::equal1d(operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 1:
            return equal1d1d(std::move(lhs), std::move(rhs));

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool equal::equal2d2d(operand_type&& lhs, operand_type&& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal2d2d",
                "the dimensions of the operands do not match");
        }

        lhs.matrix().array() =
            (lhs.matrix().array() == rhs.matrix().array()).cast<double>();

        return lhs.matrix().norm() != 0.0;
    }

    bool equal::equal2d(operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return equal2d2d(std::move(lhs), std::move(rhs));

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal2d",
                "the operands have incompatible number of dimensions");
        }
    }

    bool equal::equal_all(operand_type&& lhs, operand_type&& rhs) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return equal0d(std::move(lhs), std::move(rhs));

        case 1:
            return equal1d(std::move(lhs), std::move(rhs));

        case 2:
            return equal2d(std::move(lhs), std::move(rhs));

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "equal::equal_all",
                "left hand side operand has unsupported number of "
                "dimensions");
        }
    }

    namespace detail
    {
        struct visit_equal
        {
            using operand_type = ir::node_data<double>;

            visit_equal(equal const& this_)
              : equal_(this_)
            {}

            template <typename T1, typename T2>
            bool operator()(T1, T2) const
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "equal::eval",
                    "left hand side and right hand side are incompatible and "
                        "can't be compared");
            }

            template <typename T>
            bool operator()(T && lhs, T && rhs) const
            {
                return lhs == rhs;
            }

            bool operator()(operand_type&& lhs, operand_type&& rhs) const
            {
                return equal_.equal_all(std::move(lhs), std::move(rhs));
            }

            equal const& equal_;
        };
    }

    // implement '==' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> equal::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops)
            {
                return primitive_result_type(
                    util::visit(detail::visit_equal(*this),
                        std::move(ops[0]), std::move(ops[1])));
            }),
            detail::map_operands(operands_, literal_operand)
        );
    }
}}}
