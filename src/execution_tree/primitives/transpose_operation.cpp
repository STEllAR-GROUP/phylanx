//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/transpose_operation.hpp>
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

#include <unsupported/Eigen/MatrixFunctions>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::transpose_operation>
    transpose_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    transpose_operation_type, phylanx_transpose_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(transpose_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const transpose_operation::match_data =
    {
        "transpose(_1)", &create<transpose_operation>
    };

    ///////////////////////////////////////////////////////////////////////////
    transpose_operation::transpose_operation(
            std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                "the transpose_operation primitive requires"
                "exactly one operand");
        }

        if (!valid(operands_[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "transpose_operation::transpose_operation",
                "the transpose_operation primitive requires that the "
                    "arguments given by the operands array is valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct transpose : std::enable_shared_from_this<transpose>
        {
            transpose(std::vector<primitive_argument_type> const& operands)
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
                            return this_->transpose0d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        case 2:
                            return this_->transposexd(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "transpose_operation::eval",
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

            primitive_result_type transpose0d(operands_type && ops) const
            {
                return std::move(ops[0]);       // no-op
            }

            primitive_result_type transposexd(operands_type && ops) const
            {
                ops[0].matrix().transposeInPlace();
                return std::move(ops[0]);
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    hpx::future<primitive_result_type> transpose_operation::eval() const
    {
        return std::make_shared<detail::transpose>(operands_)->eval();
    }
}}}
