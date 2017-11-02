//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/cross_operation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/blaze.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::cross_operation>
    cross_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(cross_operation_type,
    phylanx_cross_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(cross_operation_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const cross_operation::match_data =
    {
        hpx::util::make_tuple("cross", "cross(_1, _2)", &create<cross_operation>)
    };

    ///////////////////////////////////////////////////////////////////////////
    cross_operation::cross_operation(std::vector<primitive_argument_type>&& operands)
      : operands_(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct cross : std::enable_shared_from_this<cross>
        {
            cross() = default;

        protected:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

            primitive_result_type cross1d1d(operand_type &lhs, operand_type &rhs) const
            {
                blaze::row(lhs.matrix(), 0UL) = blaze::cross(
                    blaze::row(lhs.matrix(), 0UL),
                    blaze::row(rhs.matrix(), 0UL));

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
                        "cross_operation::eval",
                        "the cross_operation primitive requires exactly two "
                            "operands");
                }

                if (!valid(operands[0]) || !valid(operands[1]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cross_operation::eval",
                        "the cross_operation primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        if (ops[0].num_dimensions() != 1 ||
                            ops[1].num_dimensions() != 1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "cross_operation::eval",
                                "left hand side operand has unsupported "
                                "number of dimensions");
                        }
                        
                        return this_->cross1d1d(ops[0], ops[1]);
                    }),
                    detail::map_operands(operands, numeric_operand, args)
                );
            }
        };
    }

    hpx::future<primitive_result_type> cross_operation::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            static std::vector<primitive_argument_type> noargs;
            return std::make_shared<detail::cross>()->eval(args, noargs);
        }

        return std::make_shared<detail::cross>()->eval(operands_, args);
    }
}}}
