//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/add_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include <blaze/Math.h>

//////////////////////////////////////////////////////////////////////////////
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
    std::vector<match_pattern_type> const add_operation::match_data =
    {
        hpx::util::make_tuple("add", "_1 + __2", &create<add_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    add_operation::add_operation(
            std::vector<primitive_argument_type> && operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct add : std::enable_shared_from_this<add>
        {
            add() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;

            primitive_result_type add0d0d(args_type && args) const
            {
                arg_type& lhs = args[0];
                arg_type& rhs = args[1];

                if (args.size() == 2)
                {
                    lhs.scalar() += rhs.scalar();
                    return primitive_result_type(std::move(lhs));
                }

                return primitive_result_type(std::accumulate(
                    args.begin() + 1, args.end(), std::move(lhs),
                    [](arg_type& result, arg_type const& curr)
                    ->  arg_type
                    {
                        result.scalar() += curr.scalar();
                        return std::move(result);
                    }));
            }

            primitive_result_type add0d1d(args_type && args) const
            {
                if (args.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add0d1d",
                        "the add_operation primitive can add a single value "
                            "to a vector only if there are exactly 2 operands");
                }

                args[1].vector(blaze::map(args[1].vector(),
                    [&](double x) { return args[0].scalar() + x; }));

                return primitive_result_type(std::move(args[1]));
            }

            primitive_result_type add0d2d(args_type && args) const
            {
                if (args.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add0d2d",
                        "the add_operation primitive can add a single value "
                            "to a matrix only if there are exactly 2 operands");
                }

                args[1].matrix(blaze::map(args[1].matrix(),
                    [&](double x) { return args[0].scalar() + x; }));

                return primitive_result_type(std::move(args[1]));
            }

            primitive_result_type add0d(args_type && args) const
            {
                std::size_t rhs_dims = args[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return add0d0d(std::move(args));

                case 1:
                    return add0d1d(std::move(args));

                case 2:
                    return add0d2d(std::move(args));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add0d",
                        "the operands have incompatible number of dimensions");
                }
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_result_type add1d0d(args_type && args) const
            {
                if (args.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add0d1d",
                        "the add_operation primitive can add a single value "
                            "to a vector only if there are exactly 2 operands");
                }

                args[0].vector(blaze::map(args[0].vector(),
                    [&](double x) { return x + args[1].scalar(); }));

                return primitive_result_type(std::move(args[0]));
            }

            primitive_result_type add1d1d(args_type && args) const
            {
                arg_type& lhs = args[0];
                arg_type& rhs = args[1];

                std::size_t lhs_size = lhs.dimension(0);
                std::size_t rhs_size = rhs.dimension(0);

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add1d1d",
                        "the dimensions of the operands do not match");
                }

                if (args.size() == 2)
                {
                    lhs.vector() += rhs.vector();
                    return primitive_result_type(std::move(lhs));
                }

                arg_type& first_term = *args.begin();
                return primitive_result_type(std::accumulate(
                    args.begin() + 1, args.end(), std::move(first_term),
                    [](arg_type& result, arg_type const& curr) -> arg_type
                    {
                        result.vector() += curr.vector();
                        return std::move(result);
                    }));
            }

            primitive_result_type add1d(args_type && args) const
            {
                std::size_t rhs_dims = args[1].num_dimensions();

                switch(rhs_dims)
                {
                case 0:
                    return add1d0d(std::move(args));

                case 1:
                    return add1d1d(std::move(args));

                case 2: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            ///////////////////////////////////////////////////////////////////////////
            primitive_result_type add2d0d(args_type && args) const
            {
                if (args.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add0d2d",
                        "the add_operation primitive can add a single value "
                            "to a matrix only if there are exactly 2 operands");
                }

                args[0].matrix() = blaze::map(
                        args[0].matrix(),
                        [&](double x) { return x + args[1].scalar(); });
                return primitive_result_type(std::move(args[0]));
            }

            primitive_result_type add2d2d(args_type && args) const
            {
                arg_type& lhs = args[0];
                arg_type& rhs = args[1];

                auto lhs_size = lhs.dimensions();
                auto rhs_size = rhs.dimensions();

                if (lhs_size != rhs_size)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add2d2d",
                        "the dimensions of the operands do not match");
                }

                if (args.size() == 2)
                {
                    lhs.matrix() += rhs.matrix();
                    return primitive_result_type(std::move(lhs));
                }

                arg_type& first_term = *args.begin();
                return primitive_result_type(std::accumulate(
                    args.begin() + 1, args.end(), std::move(first_term),
                    [](arg_type& result, arg_type const& curr)
                    ->  arg_type
                    {
                        result.matrix() += curr.matrix();
                        return std::move(result);
                    }));
            }

            primitive_result_type add2d(args_type && args) const
            {
                std::size_t rhs_dims = args[1].num_dimensions();
                switch(rhs_dims)
                {
                case 0:
                    return add2d0d(std::move(args));

                case 2:
                    return add2d2d(std::move(args));

                case 1: HPX_FALLTHROUGH;
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::add2d",
                        "the operands have incompatible number of dimensions");
                }
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() < 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "add_operation::eval",
                        "the add_operation primitive requires at least two "
                            "operands");
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
                        "add_operation::eval",
                        "the add_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type && args) -> primitive_result_type
                    {
                        std::size_t lhs_dims = args[0].num_dimensions();
                        switch (lhs_dims)
                        {
                        case 0:
                            return this_->add0d(std::move(args));

                        case 1:
                            return this_->add1d(std::move(args));

                        case 2:
                            return this_->add2d(std::move(args));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "add_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    // implement '+' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> add_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::add>()->eval(args, noargs);
        }

        return std::make_shared<detail::add>()->eval(operands_, args);
    }
}}}
