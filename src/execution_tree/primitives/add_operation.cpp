//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/add_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <numeric>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::add_operation>
    add_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    add_operation_type, phylanx_add_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(add_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(
            std::vector<ast::literal_value_type>&& literals,
            std::vector<primitive>&& operands)
      : literals_(std::move(literals))
      , operands_(std::move(operands))
    {
        if (operands_.size() < 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add_operation",
                "the add_operation primitive requires at least two operands");
        }

        // Verify that argument arrays are filled properly (this could be
        // converted to asserts).
        if (operands_.size() != literals_.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add_operation",
                "the add_operation primitive requires that the size of the "
                    "literals and operands arrays is the same");
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != literals.size(); ++i)
        {
            if (valid(literals_[i]))
            {
                if (operands_[i].valid())
                {
                    arguments_valid = false;
                }
            }
            else if (!operands_[i].valid())
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add_operation",
                "the add_operation primitive requires that the "
                    "exactly one element of the literals and operands "
                    "arrays is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add0d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 0:
            return ir::node_data<double>(
                std::accumulate(ops.begin(), ops.end(), 0.0,
                    [](double result, ir::node_data<double> const& curr)
                    {
                        return result + curr[0];
                    }));

        case 1: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add0d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add1d1d(operands_type const& ops) const
    {
        std::size_t lhs_size = ops[0].dimension(0);
        std::size_t rhs_size = ops[1].dimension(0);

        if(lhs_size  != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d1d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, 1>;

        Eigen::Matrix<double, Eigen::Dynamic, 1> result =
            std::accumulate(ops.begin(), ops.end(),
                array_type::Zero(lhs_size).eval(),
                [&](array_type const& result, ir::node_data<double> const& curr)
                ->  array_type
                {
                    Eigen::Map<array_type const> lhs(curr.data(), lhs_size);
                    return result + lhs;
                });

        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> add_operation::add1d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 1:
            return add1d1d(ops);

        case 0: HPX_FALLTHROUGH;
        case 2: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add1d",
                "the operands have incompatible number of dimensions");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    ir::node_data<double> add_operation::add2d2d(operands_type const& ops) const
    {
        auto lhs_size = ops[0].dimensions();
        auto rhs_size = ops[1].dimensions();

        if (lhs_size != rhs_size)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d2d",
                "the dimensions of the operands do not match");
        }

        using array_type = Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic>;
        using array_map_type = Eigen::Map<array_type const>;

        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> result =
            std::accumulate(
                ops.begin(), ops.end(),
                array_type::Zero(lhs_size[0], lhs_size[1]).eval(),
                [&](array_type const& result, ir::node_data<double> const& curr)
                ->  array_type
                {
                    return result +
                        array_map_type{curr.data(), lhs_size[0], lhs_size[1]};
                });

        return ir::node_data<double>(std::move(result));
    }

    ir::node_data<double> add_operation::add2d(operands_type const& ops) const
    {
        std::size_t rhs_dims = ops[1].num_dimensions();
        switch(rhs_dims)
        {
        case 2:
            return add2d2d(ops);

        case 0: HPX_FALLTHROUGH;
        case 1: HPX_FALLTHROUGH;
        default:
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "add_operation::add2d",
                "the operands have incompatible number of dimensions");
        }
    }

    namespace detail
    {
        template <typename T1, typename T2, typename F>
        auto map(std::vector<T1> const& in1, std::vector<T2> const& in2, F&& f)
            -> std::vector<decltype(
                hpx::util::invoke(f, std::declval<T1>(), std::declval<T2>()))>
        {
            HPX_ASSERT(in1.size() == in2.size());

            std::vector<decltype(
                hpx::util::invoke(f, std::declval<T1>(), std::declval<T2>()))>
                out;
            out.reserve(in1.size());

            for (std::size_t i = 0; i != in1.size(); ++i)
            {
                out.push_back(hpx::util::invoke(f, in1[i], in2[i]));
            }
            return out;
        }
    }

    // implement '+' for all possible combinations of lhs and rhs
    hpx::future<ir::node_data<double>> add_operation::eval() const
    {
        return hpx::dataflow(hpx::util::unwrapping(
            [this](std::vector<ir::node_data<double>> && ops)
            {
                std::size_t lhs_dims = ops[0].num_dimensions();
                switch (lhs_dims)
                {
                case 0:
                    return add0d(ops);

                case 1:
                    return add1d(ops);

                case 2:
                    return add2d(ops);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::eval",
                        "left hand side operand has unsupported number of "
                        "dimensions");
                }
            }),
            detail::map(literals_, operands_,
                [](ast::literal_value_type const& val, primitive const& p)
                ->  hpx::future<ir::node_data<double>>
                {
                    if (valid(val))
                    {
                        return hpx::make_ready_future(
                            ast::detail::literal_value(val));
                    }

                    HPX_ASSERT(p.valid());
                    return p.eval();
                })
        );
    }
}}}
