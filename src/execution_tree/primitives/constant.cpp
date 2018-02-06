//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/constant.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::constant>
    constant_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    constant_type, phylanx_constant_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(constant_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const constant::match_data =
    {
        hpx::util::make_tuple("constant",
            std::vector<std::string>{"constant(_1, _2)", "constant(_1)"},
            &create<constant>)
    };

    ///////////////////////////////////////////////////////////////////////////
    constant::constant(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

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

        struct constant : std::enable_shared_from_this<constant>
        {
            constant() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_argument_type constant0d(operand_type && op) const
            {
                return primitive_argument_type{std::move(op)};       // no-op
            }

            primitive_argument_type constant1d(
                operand_type&& op, std::size_t dim) const
            {
                using vector_type = blaze::DynamicVector<double>;
                return primitive_argument_type{
                    operand_type{vector_type(dim, op[0])}};
            }

            primitive_argument_type constant2d(operand_type&& op,
                operand_type::dimensions_type const& dim) const
            {
                using matrix_type = blaze::DynamicMatrix<double>;
                return primitive_argument_type{
                    operand_type{matrix_type{dim[0], dim[1], op[0]}}};
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 1 && operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "constant::eval",
                        "the constant primitive requires "
                            "at least one and at most 2 operands");
                }

                if (!valid(operands[0]) ||
                    (operands.size() == 2 && !valid(operands[1])))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "constant::eval",
                        "the constant primitive requires that the "
                            "arguments given by the operands array are valid");
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
                                    "the first argument must be a literal "
                                        "scalar value");
                            }
                            if (op1.empty())
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "constant::extract_num_dimensions",
                                    "the constant primitive requires "
                                        "for the shape not to be empty");
                            }
                            if (op1.size() > 2)
                            {
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "constant::extract_num_dimensions",
                                    "the constant primitive requires "
                                        "for the shape not to have more than "
                                        "two entries");
                            }

                            auto dims = extract_dimensions(op1);
                            switch (extract_num_dimensions(op1))
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
                                    "left hand side operand has unsupported "
                                        "number of dimensions");
                            }
                        }),
                        numeric_operand(operands[0], args),
                        list_operand(operands[1], args));
                }

                // if constant() was invoked with one argument, we simply
                // provide the argument as the desired result
                return literal_operand(operands[0], args);
            }
        };
    }

    hpx::future<primitive_argument_type> constant::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::constant>()->eval(args, noargs);
        }

        return std::make_shared<detail::constant>()->eval(operands_, args);
    }
}}}
