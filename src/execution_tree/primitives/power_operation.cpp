//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/power_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>
#include <cmath>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::power_operation>
    power_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(power_operation_type,
    phylanx_power_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(power_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const power_operation::match_data =
    {
        hpx::util::make_tuple("power", "power(_1, _2)", &create<power_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    power_operation::power_operation(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct power : std::enable_shared_from_this<power>
        {
            power() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type power0d(operands_type && ops) const
            {
                ops[0][0] = std::pow(ops[0][0], ops[1][0]);
                return std::move(ops[0]);
            }

            primitive_result_type powerxd(operands_type && ops) const
            {
                operand_type& lhs = ops[0];
                operand_type& rhs = ops[1];

                lhs.matrix() = blaze::pow(lhs.matrix(), rhs[0]);

                return std::move(lhs);
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 2)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        "the power_operation primitive requires exactly two "
                            "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "power_operation::eval",
                        "the power_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        if (ops[1].num_dimensions() != 0)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "power_operation::eval",
                                "right hand side operand has to be a scalar "
                                "value");
                        }

                        switch (ops[0].num_dimensions())
                        {
                        case 0:
                            return this_->power0d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        case 2:
                            return this_->powerxd(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "power_operation::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    hpx::future<primitive_result_type> power_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::power>()->eval(args, noargs);
        }

        return std::make_shared<detail::power>()->eval(operands_, args);
    }
}}}
