//  Copyright (c) 2017 Parsa Amini
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/cross_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cross_operation::match_data =
    {
        hpx::util::make_tuple("cross",
            std::vector<std::string>{"cross(_1, _2)"},
            &create<cross_operation>)
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

            primitive_argument_type cross1d(
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

            primitive_argument_type cross1d1d(operand_type& lhs, operand_type& rhs) const
            {
                std::size_t lhs_vector_dims = lhs.size();
                std::size_t rhs_vector_dims = rhs.size();

                if (lhs_vector_dims < 2ul || rhs_vector_dims < 2ul)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross1d1d",
                        "operands have an invalid number of columns");
                }

                // lhs vector has 2 elements
                if (lhs_vector_dims < 3ul)
                {
                    if (lhs.is_ref())
                    {
                        blaze::DynamicVector<double> temp = lhs.vector();
                        temp.resize(3ul);
                        temp[2ul] = 0.0;
                        lhs = std::move(temp);
                    }
                    else
                    {
                        lhs.vector_non_ref().resize(3ul);
                        lhs[2ul] = 0.0;
                    }
                }

                // Only rhs has 2 elements
                if (rhs_vector_dims < 3ul)
                {
                    if (rhs.is_ref())
                    {
                        blaze::DynamicVector<double> temp = rhs.vector();
                        temp.resize(3ul);
                        temp[2ul] = 0.0;
                        rhs = std::move(temp);
                    }
                    else
                    {
                        rhs.vector_non_ref().resize(3ul);
                        rhs[2ul] = 0.0;
                    }
                }

                // Both vectors have 3 elements
                lhs.vector() %= rhs.vector();
                return primitive_argument_type{
                    ir::node_data<double>{std::move(lhs)}};
            }

            primitive_argument_type cross1d2d(
                operand_type& lhs, operand_type& rhs) const
            {
                if (lhs.size() == 2ul)
                {
                    if (lhs.is_ref())
                    {
                        blaze::DynamicVector<double> temp = lhs.vector();
                        temp.resize(3ul);
                        temp[2ul] = 0.0;
                        lhs = std::move(temp);
                    }
                    else
                    {
                        lhs.vector_non_ref().resize(3ul);
                        lhs[2ul] = 0.0;
                    }
                }

                // lhs has to have 3 elements
                if (lhs.size() == 3ul)
                {
                    // rhs has 2 columns per vector
                    if (rhs.dimension(1) == 2ul)
                    {
                        if (rhs.is_ref())
                        {
                            blaze::DynamicMatrix<double> temp = rhs.matrix();
                            temp.resize(rhs.dimension(0), 3ul);
                            blaze::column(temp, 2ul) = 0.0;
                            rhs = std::move(temp);
                        }
                        else
                        {
                            rhs.matrix_non_ref().resize(rhs.dimension(0), 3ul);
                            blaze::column(rhs.matrix_non_ref(), 2ul) = 0.0;
                        }
                    }

                    for (std::size_t i = 0ul; i != rhs.dimension(0); ++i)
                    {
                        blaze::DynamicVector<double> rhs_op_vec{
                            rhs[{i, 0ul}], rhs[{i, 1ul}], rhs[{i, 2ul}]};

                        auto i_vector = lhs.vector() % rhs_op_vec;

                        rhs[{i, 0ul}] = i_vector[0ul];
                        rhs[{i, 1ul}] = i_vector[1ul];
                        rhs[{i, 2ul}] = i_vector[2ul];
                    }

                    return primitive_argument_type{
                        ir::node_data<double>{std::move(rhs)}};
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cross_operation::cross1d2d",
                    "operand vectors have an invalid number of elements");
            }

            primitive_argument_type cross2d(
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

            primitive_argument_type cross2d1d(
                operand_type& lhs, operand_type& rhs) const
            {
                if (rhs.size() == 2ul)
                {
                    if (rhs.is_ref())
                    {
                        blaze::DynamicVector<double> temp = rhs.vector();
                        temp.resize(3ul);
                        temp[2ul] = 0.0;
                        rhs = std::move(temp);
                    }
                    else
                    {
                        rhs.vector_non_ref().resize(3ul);
                        rhs[2ul] = 0.0;
                    }
                }

                // rhs has to have 3 elements
                if (rhs.size() == 3ul)
                {
                    // lhs has 2 columns per vector
                    if (lhs.dimension(1) == 2ul)
                    {
                        if (lhs.is_ref())
                        {
                            blaze::DynamicMatrix<double> temp = lhs.matrix();
                            temp.resize(lhs.dimension(0), 3ul);
                            blaze::column(temp, 2ul) = 0.0;
                            lhs = std::move(temp);
                        }
                        else
                        {
                            lhs.matrix_non_ref().resize(lhs.dimension(0), 3ul);
                            blaze::column(lhs.matrix_non_ref(), 2ul) = 0.0;
                        }
                    }

                    auto lhsm = lhs.matrix();
                    blaze::DynamicMatrix<double> rhs_op_mat{
                        {rhs[0ul], rhs[1ul], rhs[2ul]}};
                    blaze::DynamicMatrix<double> temp = lhs.matrix();

                    for (std::size_t i = 0ul; i != lhs.dimension(0); ++i)
                    {
                        blaze::row(temp, i) = blaze::cross(
                            blaze::row(lhsm, i),
                            blaze::row(rhs_op_mat, 0ul));
                    }

                    return primitive_argument_type{
                        ir::node_data<double>{std::move(temp)}};
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cross_operation::cross2d1d",
                    "operand vectors have an invalid number of elements");
            }

            primitive_argument_type cross2d2d(
                operand_type& lhs, operand_type& rhs) const
            {
                // Both matrices have to have the same number of rows
                if (lhs.dimension(0) != rhs.dimension(0))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::cross2d",
                        "operands have non-matching number of rows");
                }

                // If both have 2 elements per vector
                if (lhs.dimension(1) == 2ul)
                {
                    // If rhs has 2 elements per vector
                    if (rhs.dimension(1) == 2ul)
                    {
                        auto lhsm = lhs.matrix();
                        auto rhsm = rhs.matrix();
                        blaze::DynamicMatrix<double> temp = lhs.matrix();

                        for (std::size_t idx_row = 0;
                             idx_row != lhs.dimension(0);
                             ++idx_row)
                        {
                            blaze::row(temp, idx_row) = blaze::cross(
                                blaze::row(lhsm, idx_row),
                                blaze::row(rhsm, idx_row));
                        }

                        return primitive_argument_type{
                            ir::node_data<double>{std::move(temp)}};
                    }

                    // If only lhs has 2 elements per vector
                    else
                    {
                        if (rhs.is_ref())
                        {
                            blaze::DynamicMatrix<double> temp = lhs.matrix();
                            temp.resize(lhs.dimension(0), 3ul);
                            blaze::column(temp, 2ul) = 0.0;
                            lhs = std::move(temp);
                        }
                        else
                        {
                            lhs.matrix_non_ref().resize(lhs.dimension(0), 3ul);
                            blaze::column(lhs.matrix_non_ref(), 2ul) = 0.0;
                        }
                    }
                }

                // If lhs has 3 elements per vector
                if (lhs.dimension(1) == 3ul)
                {
                    // If rhs has 2 elements per vector
                    if (rhs.dimension(1) == 2ul)
                    {
                        if (rhs.is_ref())
                        {
                            blaze::DynamicMatrix<double> temp = rhs.matrix();
                            temp.resize(rhs.dimension(0), 3ul);
                            blaze::column(temp, 2ul) = 0.0;
                            rhs = std::move(temp);
                        }
                        else
                        {
                            rhs.matrix_non_ref().resize(rhs.dimension(0), 3ul);
                            blaze::column(rhs.matrix_non_ref(), 2ul) = 0.0;
                        }
                    }

                    // Both have 3 elements per vector
                    auto lhsm = lhs.matrix();
                    auto rhsm = rhs.matrix();
                    blaze::DynamicMatrix<double> temp = lhs.matrix();

                    for (std::size_t idx_row = 0; idx_row != lhs.dimension(0);
                        ++idx_row)
                    {
                        blaze::row(temp, idx_row) = blaze::cross(
                            blaze::row(lhsm, idx_row),
                            blaze::row(rhsm, idx_row));
                    }

                    return primitive_argument_type{
                        ir::node_data<double>{std::move(temp)}};
                }

                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cross_operation::cross2d2d",
                    "operand vectors have an invalid number of elements");
            }

        public:
            hpx::future<primitive_argument_type> eval(
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
                    [this_](operands_type&& ops) -> primitive_argument_type
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
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> cross_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::cross>()->eval(args, noargs);
        }
        return std::make_shared<detail::cross>()->eval(operands_, args);
    }
}}}

