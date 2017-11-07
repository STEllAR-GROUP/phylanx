//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/dot_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::dot_operation>
    dot_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(dot_operation_type,
    phylanx_dot_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(dot_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const dot_operation::match_data =
    {
        hpx::util::make_tuple("dot", "dot(_1, _2)", &create<dot_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    dot_operation::dot_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct dot : std::enable_shared_from_this<dot>
        {
            dot() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type dot0d(operands_type && ops) const
            {
                if (ops[1].num_dimensions() != 0UL)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::dot0d",
                        "the operands have incompatible number of dimensions");
                }

                ops[0].scalar() *= ops[1].scalar();
                return std::move(ops[0]);
            }

            // lhs_num_dims == 1
            // Case 1: Inner product of two vectors
            // Case 2: Inner product of a vector and an array of vectors
            primitive_result_type dot1d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                switch (rhs.num_dimensions())
                {
                case 1UL:
                    // If is_vector(lhs) && is_vector(rhs)
                    return dot1d1d(lhs, rhs);

                case 2UL:
                    // lhs_order == 1 && rhs_order == 2
                    // If is_vector(lhs) && is_matrix(rhs)
                    // We can treat this like 2d matrix multiplication
                    // e.g. [2].[2x5]->[5]
                    //    [1x2].[2x5]->[1x5]
                    return dot2d2d(lhs, rhs);

                default:
                    // lhs_order == 1 && rhs_order != 2
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::dot1d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_result_type dot1d1d(
                operand_type& lhs, operand_type& rhs) const
            {
                // lhs.dimension(0) == rhs.dimension(0)
                lhs.scalar(blaze::dot(lhs.vector(), rhs.vector()));

                return std::move(lhs);
            }

            // lhs_num_dims == 2
            // Multiply a matrix with a vector
            // Regular matrix multiplication
            primitive_result_type dot2d(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                switch (rhs.num_dimensions())
                {
                case 1UL:
                    return dot2d1d(lhs, rhs);

                case 2UL:
                    return dot2d2d(lhs, rhs);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::dot2d",
                        "the operands have incompatible number of dimensions");
                }
            }

            primitive_result_type dot2d1d(
                operand_type& lhs, operand_type& rhs) const
            {
                if (lhs.dimension(1) != rhs.dimension(0))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::dot2d1d",
                        "the operands have incompatible number of dimensions");
                }

                rhs.vector(lhs.matrix() * rhs.vector());

                return std::move(rhs);
            }

            primitive_result_type dot2d2d(
                operand_type& lhs, operand_type& rhs) const
            {
                if (lhs.dimension(1) != rhs.dimension(0))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::dot2d2d",
                        "the operands have incompatible number of dimensions");
                }

                lhs.matrix() *= rhs.matrix();
                return std::move(lhs);
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::eval",
                        "the dot_operation primitive requires exactly two "
                            "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "dot_operation::eval",
                        "the dot_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t dims = ops[0].num_dimensions();
                        switch (dims)
                        {
                        case 0UL:
                            return this_->dot0d(std::move(ops));

                        case 1UL:
                            return this_->dot1d(std::move(ops));

                        case 2UL:
                            return this_->dot2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "dot_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    // implement 'dot' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> dot_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::dot>()->eval(args, noargs);
        }

        return std::make_shared<detail::dot>()->eval(operands_, args);
    }
}}}
