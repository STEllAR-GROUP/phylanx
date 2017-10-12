//   Copyright (c) 2017 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/constant.hpp>
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
        "constant(_1, _2)", &create<constant>
    };

    ///////////////////////////////////////////////////////////////////////////
    constant::constant(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {
        if (operands_.size() != 1 && operands_.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::constant",
                "the constant primitive requires"
                    "at least one annd at most 2 operands");
        }

        if (!valid(operands_[0]) ||
            (operands_.size() == 2 && !valid(operands_[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "constant::constant",
                "the constant primitive requires that the "
                    "arguments given by the operands array are valid");
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct constant : std::enable_shared_from_this<constant>
        {
            constant(std::vector<primitive_argument_type> const& operands)
              : operands_(operands)
            {}

            hpx::future<primitive_result_type> eval()
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t dims = ops[0].num_dimensions();
                        if (ops.size() > 1)
                        {
                            dims = ops[1].num_dimensions();
                        }

                        switch (dims)
                        {
                        case 0:
                            return this_->constant0d(std::move(ops));

                        case 1:
                            return this_->constant1d(std::move(ops));

                        case 2:
                            return this_->constant2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "constant::eval",
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

            primitive_result_type constant0d(operands_type && ops) const
            {
                return std::move(ops[0]);       // no-op
            }

            primitive_result_type constant1d(operands_type && ops) const
            {
                std::ptrdiff_t dim = ops[0].dimension(0);
                if (ops.size() > 1)
                {
                    dim = ops[1].dimension(0);
                }

                using vector_type = Eigen::Matrix<double, Eigen::Dynamic, 1>;

                vector_type result =
                    Eigen::VectorXd::Constant(dim, ops[0][0]);
                return operand_type(std::move(result));
            }

            primitive_result_type constant2d(operands_type && ops) const
            {
                auto dim = ops[0].dimensions();
                if (ops.size() > 1)
                {
                    dim = ops[1].dimensions();
                }

                using matrix_type =
                    Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>;

                matrix_type result =
                    Eigen::VectorXd::Constant(dim[0], dim[1], ops[0][0]);
                return operand_type(std::move(result));
            }

        private:
            std::vector<primitive_argument_type> operands_;
        };
    }

    hpx::future<primitive_result_type> constant::eval() const
    {
        return std::make_shared<detail::constant>(operands_)->eval();
    }
}}}
