//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/constant.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <array>
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
    namespace detail
    {
        std::size_t extract_num_dimensions(
            std::vector<primitive_argument_type> const& shape)
        {
            return shape.size();
        }

        std::array<std::size_t, 2> extract_dimensions(
            std::vector<primitive_argument_type> const& shape)
        {
            std::array<std::size_t, 2> result = {0, 0};
            result[0] = extract_integer_value(shape[0]);
            if (shape.size() > 1)
            {
                result[1] = extract_integer_value(shape[1]);
            }
            return result;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive create_constant(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("constant");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const constant::match_data =
    {
        hpx::util::make_tuple("constant",
            std::vector<std::string>{"constant(_1, _2)", "constant(_1)"},
            &create_constant, &create_primitive<constant>)
    };

    ///////////////////////////////////////////////////////////////////////////
    constant::constant(std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type constant::constant0d(operand_type && op) const
    {
        return primitive_argument_type{std::move(op)};       // no-op
    }

    primitive_argument_type constant::constant1d(
        operand_type&& op, std::size_t dim) const
    {
        using vector_type = blaze::DynamicVector<double>;
        return primitive_argument_type{
            operand_type{vector_type(dim, op[0])}};
    }

    primitive_argument_type constant::constant2d(operand_type&& op,
        operand_type::dimensions_type const& dim) const
    {
        using matrix_type = blaze::DynamicMatrix<double>;
        return primitive_argument_type{
            operand_type{matrix_type{dim[0], dim[1], op[0]}}};
    }

    hpx::future<primitive_argument_type> constant::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                execution_tree::generate_error_message(
                    "the constant primitive requires "
                        "at least one and at most 2 operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::eval",
                execution_tree::generate_error_message(
                    "the constant primitive requires that the "
                    "arguments given by the operands array are valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        if (operands.size() == 2)
        {
            return hpx::dataflow(hpx::util::unwrapping(
                [this_](operand_type&& op0,
                        std::vector<primitive_argument_type>&& op1)
                ->  primitive_argument_type
                {
                    if (op0.num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            execution_tree::generate_error_message(
                                "the first argument must be a literal "
                                    "scalar value",
                                this_->name_, this_->codename_));
                    }
                    if (op1.empty())
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::extract_num_dimensions",
                            execution_tree::generate_error_message(
                                "the constant primitive requires "
                                    "for the shape not to be empty",
                                this_->name_, this_->codename_));
                    }
                    if (op1.size() > 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::extract_num_dimensions",
                            execution_tree::generate_error_message(
                                "the constant primitive requires "
                                    "for the shape not to have more than "
                                    "two entries",
                                this_->name_, this_->codename_));
                    }

                    auto dims = detail::extract_dimensions(op1);
                    switch (detail::extract_num_dimensions(op1))
                    {
                    case 0:
                        return this_->constant0d(std::move(op0));

                    case 1:
                        return this_->constant1d(std::move(op0), dims[0]);

                    case 2:
                        return this_->constant2d(std::move(op0), dims);

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "constant::eval",
                            execution_tree::generate_error_message(
                                "left hand side operand has unsupported "
                                    "number of dimensions",
                                this_->name_, this_->codename_));
                    }
                }),
                numeric_operand(operands[0], args, name_, codename_),
                list_operand(operands[1], args, name_, codename_));
        }

        // if constant() was invoked with one argument, we simply
        // provide the argument as the desired result
        return literal_operand(operands[0], args, name_, codename_);
    }

    hpx::future<primitive_argument_type> constant::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
