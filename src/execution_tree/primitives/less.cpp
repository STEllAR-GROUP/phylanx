//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/less.hpp>
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
    phylanx::execution_tree::primitives::less>
    less_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    less_type, phylanx_less_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(less_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const less::match_data =
    {
        "_1 < _2", &create<less>
    };

    ///////////////////////////////////////////////////////////////////////////
    less::less(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less",
                "the less primitive requires exactly two operands");
        }

        if (!valid(operands_[0]) || !valid(operands_[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less",
                "the less primitive requires that the arguments given "
                    "by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool less::less0d(ir::node_data<double> const& lhs,
        ir::node_data<double> const& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return lhs[0] < rhs[0];

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool less::less1d1d(ir::node_data<double> const& lhs,
        ir::node_data<double> const& rhs) const
    {
        std::size_t lhs_size = lhs.dimension(0);
        std::size_t rhs_size = rhs.dimension(0);

        if (lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d1d",
                "the dimensions of the operands do not match");
        }

        Eigen::Matrix<double, Eigen::Dynamic, 1> result =
            (lhs.matrix().array() < rhs.matrix().array()).cast<double>();

        return result.norm() != 0.0;
    }

    bool less::less1d(ir::node_data<double> const& lhs,
        ir::node_data<double> const& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 1:
            return less1d1d(lhs, rhs);

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    bool less::less2d2d(ir::node_data<double> const& lhs,
        ir::node_data<double> const& rhs) const
    {
        auto lhs_size = lhs.dimensions();
        auto rhs_size = rhs.dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d2d",
                "the dimensions of the operands do not match");
        }

        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> result =
            (lhs.matrix().array() < rhs.matrix().array()).cast<double>();

        return result.norm() != 0.0;
    }

    bool less::less2d(ir::node_data<double> const& lhs,
        ir::node_data<double> const& rhs) const
    {
        std::size_t rhs_dims = rhs.num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return less2d2d(lhs, rhs);

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less2d",
                "the operands have incompatible number of dimensions");
        }
    }

    bool less::less_all(ir::node_data<double> const& lhs,
        ir::node_data<double> const& rhs) const
    {
        std::size_t lhs_dims = lhs.num_dimensions();
        switch (lhs_dims)
        {
        case 0:
            return less0d(lhs, rhs);

        case 1:
            return less1d(lhs, rhs);

        case 2:
            return less2d(lhs, rhs);

        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "less::less_all",
                "left hand side operand has unsupported number of "
                "dimensions");
        }
    }

    namespace detail
    {
        struct visit_less
        {
            visit_less(less const& this_)
              : less_(this_)
            {}

            template <typename T1, typename T2>
            bool operator()(T1, T2) const
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "less::eval",
                    "left hand side and right hand side are incompatible and "
                        "can't be compared");
            }

            template <typename T>
            bool operator()(T const& lhs, T const& rhs) const
            {
                return lhs < rhs;
            }

            bool operator()(ir::node_data<double> const& lhs,
                ir::node_data<double> const& rhs) const
            {
                return less_.less_all(lhs, rhs);
            }

            less const& less_;
        };
    }

    // implement '<' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> less::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](operands_type && ops)
            {
                return primitive_result_type(
                    util::visit(detail::visit_less(*this), ops[0], ops[1]));
            }),
            detail::map_operands(operands_, literal_operand)
        );
    }
}}}
