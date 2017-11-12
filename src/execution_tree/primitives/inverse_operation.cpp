//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/inverse_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::inverse_operation>
    inverse_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    inverse_operation_type, phylanx_inverse_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(inverse_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const inverse_operation::match_data =
    {
        hpx::util::make_tuple(
            "inverse", "inverse(_1)", &create<inverse_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    inverse_operation::inverse_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct inverse : std::enable_shared_from_this<inverse>
        {
            inverse() = default;

            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "inverse_operation::eval",
                        "the inverse_operation primitive requires"
                        "exactly one operand");
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "inverse_operation::eval",
                        "the inverse_operation primitive requires that the "
                            "arguments given by the operands array is valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t dims = ops[0].num_dimensions();
                        switch (dims)
                        {
                        case 0:
                            return this_->inverse0d(std::move(ops));

                        case 2:
                            return this_->inverse2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "inverse_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type inverse0d(operands_type && ops) const
            {
                ops[0].scalar() = 1 / ops[0].scalar();
                return std::move(ops[0]);
            }

            primitive_result_type inverse2d(operands_type && ops) const
            {
                if (ops[0].dimension(0) != ops[0].dimension(1))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "inverse::inverse2d",
                        "matrices to inverse have to be quadratic");
                }

                using matrix_type = blaze::DynamicMatrix<double>;

                blaze::invert(ops[0].matrix());
                return std::move(ops[0]);
            }
        };
    }

    hpx::future<primitive_result_type> inverse_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::inverse>()->eval(args, noargs);
        }

        return std::make_shared<detail::inverse>()->eval(operands_, args);
    }
}}}
