//  Copyright (c) 2017 Bibek Wagle
//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/exponential_operation.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cmath>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::exponential_operation>
    exponential_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    exponential_operation_type, phylanx_exponential_operation_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(exponential_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const exponential_operation::match_data =
    {
        hpx::util::make_tuple("exp",
            std::vector<std::string>{"exp(_1)"},
            &create<exponential_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    exponential_operation::exponential_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct exp : std::enable_shared_from_this<exp>
        {
            exp() = default;

        private:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

        protected:
            ir::node_data<double> exponential0d(operands_type&& ops) const
            {
                ops[0] = double(std::exp(ops[0].scalar()));
                return std::move(ops[0]);
            }

            ir::node_data<double> exponential1d(operands_type&& ops) const
            {
                using vector_type = blaze::DynamicVector<double>;

                vector_type result = blaze::exp(ops[0].vector());
                return ir::node_data<double>(std::move(result));
            }

            ir::node_data<double> exponentialxd(operands_type&& ops) const
            {
                using matrix_type = blaze::DynamicMatrix<double>;

                matrix_type result = blaze::exp(ops[0].matrix());
                return ir::node_data<double>(std::move(result));
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "exponential_operation::eval",
                        "the exponential_operation primitive requires"
                            "exactly one operand");
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "exponential_operation::eval",
                        "the exponential_operation primitive requires "
                            "that the arguments given by the operands array"
                            " is valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
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
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }
        };
    }

    // implement 'exp' for all possible combinations of lhs and rhs
    hpx::future<primitive_result_type> exponential_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::exp>()->eval(args, noargs);
        }

        return std::make_shared<detail::exp>()->eval(operands_, args);
    }
}}}
