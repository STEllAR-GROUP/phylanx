// Copyright (c) 2017 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/exponential_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const exponential_operation::match_data =
    {
        hpx::util::make_tuple("exp",
            std::vector<std::string>{"exp(_1)"},
            &create_exponential_operation,
            &create_primitive<exponential_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    exponential_operation::exponential_operation(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type exponential_operation::exponential0d(
        operand_type&& op) const
    {
        return primitive_argument_type{
            ir::node_data<double>{std::exp(op.scalar())}};
    }

    primitive_argument_type exponential_operation::exponential1d(
        operand_type&& op) const
    {
        using vector_type = blaze::DynamicVector<double>;

        if (op.is_ref())
        {
            vector_type result = blaze::map(op.vector(), blaze::Exp{});
            return primitive_argument_type{
                ir::node_data<double>(std::move(result))};
        }

        op.vector() = blaze::map(op.vector(), blaze::Exp{});
        return primitive_argument_type{
            ir::node_data<double>(std::move(op))};
    }

    primitive_argument_type exponential_operation::exponentialxd(
        operand_type&& op) const
    {
        using matrix_type = blaze::DynamicMatrix<double>;

        if (op.is_ref())
        {
            matrix_type result = blaze::exp(op.matrix());
            return primitive_argument_type{
                ir::node_data<double>(std::move(result))};
        }

        op.matrix() = blaze::exp(op.matrix());
        return primitive_argument_type{
            ir::node_data<double>(std::move(op))};
    }

    hpx::future<primitive_argument_type> exponential_operation::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "exponential_operation::eval",
                execution_tree::generate_error_message(
                    "the exponential_operation primitive requires"
                        "exactly one operand",
                    name_, codename_));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "exponential_operation::eval",
                execution_tree::generate_error_message(
                    "the exponential_operation primitive requires "
                        "that the arguments given by the operands "
                        "array is valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](operand_type&& op) -> primitive_argument_type
            {
                std::size_t dims = op.num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->exponential0d(std::move(op));

                case 1:
                    return this_->exponential1d(std::move(op));

                case 2:
                    return this_->exponentialxd(std::move(op));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "exponential_operation::eval",
                        execution_tree::generate_error_message(
                            "left hand side operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_));
    }

    // Implement 'exp' for all possible combinations of lhs and rhs
    hpx::future<primitive_argument_type> exponential_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
