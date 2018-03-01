// Copyright (c) 2017 Bibek Wagle
// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/exponential_operation.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_exponential_operation(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("exp");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

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
    primitive_argument_type exponential_operation::exponential0d(operands_type&& ops) const
    {
        ops[0] = double(std::exp(ops[0].scalar()));
        return primitive_argument_type{std::move(ops[0])};
    }

    primitive_argument_type exponential_operation::exponential1d(operands_type&& ops) const
    {
        using vector_type = blaze::DynamicVector<double>;

        vector_type result = blaze::exp(ops[0].vector());
        return primitive_argument_type{
            ir::node_data<double>(std::move(result))};
    }

    primitive_argument_type exponential_operation::exponentialxd(operands_type&& ops) const
    {
        using matrix_type = blaze::DynamicMatrix<double>;

        matrix_type result = blaze::exp(ops[0].matrix());
        return primitive_argument_type{
            ir::node_data<double>(std::move(result))};
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
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](operands_type&& ops) -> primitive_argument_type
            {
                std::size_t dims = ops[0].num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->exponential0d(std::move(ops));

                case 1:
                    return this_->exponential1d(std::move(ops));

                case 2:
                    return this_->exponentialxd(std::move(ops));

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "exponential_operation::eval",
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
