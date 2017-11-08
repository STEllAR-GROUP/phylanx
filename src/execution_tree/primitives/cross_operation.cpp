//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/cross_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::cross_operation>
    cross_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(cross_operation_type,
    phylanx_cross_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(cross_operation_type::wrapped_type)


///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    std::vector<match_pattern_type> const cross_operation::match_data =
    {
        hpx::util::make_tuple("cross", "cross(_1, _2)", &create<cross_operation>)
    };

    cross_operation::cross_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct cross : std::enable_shared_from_this<cross>
        {
            cross() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;
            using data_type = blaze::DynamicMatrix<double>;
            using vector_type = blaze::DynamicVector<double, blaze::rowVector>;

            primitive_result_type cross1d(
                operand_type& lhs, operand_type& rhs) const
            {
                switch (rhs.num_dimensions())
                {
                case 1:
                    return cross1d1d(lhs, rhs);

                case 2:
                    return cross1d2d(lhs, rhs);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross1d",
                        "right hand side operand has unsupported number of "
                        "dimensions");
                }
            }

            primitive_result_type cross1d1d(operand_type& lhs, operand_type& rhs) const
            {
                std::size_t lhs_vector_dims = lhs.dimension(1);
                std::size_t rhs_vector_dims = rhs.dimension(1);

                if (lhs_vector_dims < 2UL || rhs_vector_dims < 2UL)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross1d1d",
                        "operands have an invalid number of columns");
                }

                // lhs vector has 2 elements
                if (lhs_vector_dims < 3UL)
                {
                    lhs.matrix().resize(1UL, 3UL);
                    lhs.matrix()(0UL, 2UL) = 0.0;
                }

                // Only rhs has 2 elements
                if (rhs_vector_dims < 3UL)
                {
                    blaze::row(lhs.matrix(), 0UL) = blaze::cross(
                        blaze::row(lhs.matrix(), 0UL),
                        get_3d_vector_from_2d(rhs.matrix(), 0UL));
                }
                else
                {
                    // Both vectors have 3 elements
                    blaze::row(lhs.matrix(), 0UL) = blaze::cross(
                        blaze::row(lhs.matrix(), 0UL),
                        blaze::row(rhs.matrix(), 0UL));
                }

                return std::move(lhs);
            }

            primitive_result_type cross1d2d(
                operand_type& lhs, operand_type& rhs) const
            {
                std::size_t lhs_vector_dims = lhs.dimension(1);
                std::size_t rhs_vector_dims = rhs.dimension(1);

                if (lhs_vector_dims < 2UL || rhs_vector_dims < 2UL)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross1d1d",
                        "operands have an invalid number of columns");
                }

                // rhs vector has 2 elements
                if (rhs_vector_dims < 3UL)
                {
                    rhs.matrix().resize(rhs.matrix().rows(), 3UL);
                    blaze::column(rhs.matrix(), 2UL) = 0.0;
                }

                // Only lhs has 2 elements
                if (lhs_vector_dims < 3UL)
                {
                    auto left_op = get_3d_vector_from_2d(lhs.matrix(), 0UL);
                    for (std::size_t i = 0UL; i != rhs.matrix().rows(); ++i)
                    {
                        blaze::row(rhs.matrix(), i) = blaze::cross(
                            left_op,
                            blaze::row(rhs.matrix(), i));
                    }
                }
                // Both vectors have 3 elements
                else
                {
                    for (std::size_t i = 0UL; i != rhs.matrix().rows(); ++i)
                    {
                        blaze::row(rhs.matrix(), i) = blaze::cross(
                            blaze::row(lhs.matrix(), 0UL),
                            blaze::row(rhs.matrix(), i));
                    }
                }

                return std::move(rhs);
            }

            vector_type get_3d_vector_from_2d(data_type &m, std::size_t idx) const
            {
                return vector_type{ m(0UL, 0UL), m(0UL, 1UL), 0.0 };
            }

            primitive_result_type cross2d(
                operand_type& lhs, operand_type& rhs) const
            {
                switch (rhs.num_dimensions())
                {
                case 1:
                    return cross2d1d(lhs, rhs);

                case 2:
                    return cross2d2d(lhs, rhs);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross2d",
                        "right hand side operand has unsupported number of "
                        "dimensions");
                }
            }

            primitive_result_type cross2d1d(
                operand_type& lhs, operand_type& rhs) const
            {
                std::size_t lhs_vector_dims = lhs.dimension(1);
                std::size_t rhs_vector_dims = rhs.dimension(1);

                if (lhs_vector_dims < 2UL || rhs_vector_dims < 2UL)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross1d1d",
                        "operands have an invalid number of columns");
                }

                // lhs vector has 2 elements
                if (lhs_vector_dims < 3UL)
                {
                    lhs.matrix().resize(lhs.matrix().rows(), 3UL);
                    blaze::column(lhs.matrix(), 2UL) = 0.0;
                }

                // Only rhs has 2 elements
                if (rhs_vector_dims < 3UL)
                {
                    auto right_op = get_3d_vector_from_2d(rhs.matrix(), 0UL);
                    for (std::size_t i = 0UL; i != lhs.matrix().rows(); ++i)
                    {
                        blaze::row(lhs.matrix(), i) = blaze::cross(
                            blaze::row(lhs.matrix(), i),
                            right_op);
                    }
                }
                else
                {
                    // Both vectors have 3 elements
                    for (std::size_t i = 0UL; i != lhs.matrix().rows(); ++i)
                    {
                        blaze::row(lhs.matrix(), i) = blaze::cross(
                            blaze::row(lhs.matrix(), i),
                            blaze::row(rhs.matrix(), 0UL));
                    }
                }

                return std::move(lhs);
            }

            primitive_result_type cross2d2d(
                operand_type& lhs, operand_type& rhs) const
            {
                for (std::size_t idx_row = 0; idx_row != lhs.dimension(0);
                     ++idx_row)
                {
                    blaze::row(lhs.matrix(), idx_row) = blaze::cross(
                        blaze::row(lhs.matrix(), idx_row),
                        blaze::row(rhs.matrix(), idx_row));
                }

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
                        "cross_operation::eval",
                        "the cross_operation primitive requires exactly two "
                            "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::eval",
                        "the cross_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        switch (ops[0].num_dimensions())
                        {
                        case 1:
                            return this_->cross1d(ops[0], ops[1]);

                        case 2:
                            return this_->cross2d(ops[0], ops[1]);

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "cross_operation::eval",
                                "left hand side operand has unsupported "
                                "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    hpx::future<primitive_result_type> cross_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::cross>()->eval(args, noargs);
        }
        return std::make_shared<detail::cross>()->eval(operands_, args);
    }
}}}

