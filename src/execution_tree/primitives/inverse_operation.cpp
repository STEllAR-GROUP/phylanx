//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/inverse_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/eigen.hpp>

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
    match_pattern_type const inverse_operation::match_data =
    {
        "inverse(_1)", &create<inverse_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    inverse_operation::inverse_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse_operation::inverse_operation",
                "the inverse_operation primitive requires"
                "exactly one operand");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "inverse_operation::inverse_operation",
                "the inverse_operation primitive requires that the "
                    "arguments given by the operands array is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct inverse : std::enable_shared_from_this<inverse>
        {
            inverse(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

            hpx::future<primitive_result_type> eval()
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t dims = ops[0].num_dimensions();
                        switch (dims)
                        {
                        case 0:
                            return this_->inverse0d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        case 2:
                            return this_->inversexd(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "inverse_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands_, numeric_operand)
                );
            }

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type inverse0d(operands_type && ops) const
            {
                ops[0][0] = 1 / ops[0][0];
                return std::move(ops[0]);
            }

            primitive_result_type inversexd(operands_type && ops) const
            {
                using matrix_type =
                    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

                matrix_type result = ops[0].matrix().inverse();
                return ir::node_data<double>(std::move(result));
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    hpx::future<primitive_result_type> inverse_operation::eval() const
    {
        return std::make_shared<detail::inverse>(operands_)->eval();
    }
}}}
