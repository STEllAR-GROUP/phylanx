//  Copyright (c) 2017 Parsa Amini
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/square_root_operation.hpp>
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
    phylanx::execution_tree::primitives::square_root_operation>
    square_root_operation_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(square_root_operation_type,
    phylanx_square_root_operation_component, "phylanx_primitive_component",
    hpx::components::factory_enabled)
    HPX_DEFINE_GET_COMPONENT_TYPE(square_root_operation_type::wrapped_type)

    ///////////////////////////////////////////////////////////////////////////////
    namespace phylanx {
    namespace execution_tree {
        namespace primitives
        {
            ///////////////////////////////////////////////////////////////////////////
            std::vector<match_pattern_type> const square_root_operation::match_data =
            {
                hpx::util::make_tuple("square_root", "square_root(_1, _2)", &create<square_root_operation>)
            };

            ///////////////////////////////////////////////////////////////////////////
            square_root_operation::square_root_operation(std::vector<primitive_argument_type>&& operands)
                : operands_(std::move(operands))
            {}

            ///////////////////////////////////////////////////////////////////////////
            namespace detail
            {
                struct square_root : std::enable_shared_from_this<square_root>
                {
                    square_root() = default;

                protected:
                    using operand_type = ir::node_data<double>;
                    using operands_type = std::vector<operand_type>;

                    primitive_result_type square_root_0d(operands_type && ops) const
                    {
                        ops[0][0] = std::sqrt(ops[0][0]);
                        return std::move(ops[0]);
                    }

                    // lhs_num_dims == 1
                    primitive_result_type square_root_xd(operands_type && ops) const
                    {
                        ops[0].matrix() = blaze::sqrt(ops[0].matrix());

                        return std::move(ops[0]);
                    }

                public:
                    hpx::future<primitive_result_type> eval(
                        std::vector<primitive_argument_type> const& operands,
                        std::vector<primitive_argument_type> const& args)
                    {
                        if (operands.size() != 1)
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "square_root_operation::eval",
                                "the square_root_operation primitive requires "
                                "exactly one operands");
                        }

                        if (!valid(operands[0]))
                        {
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "square_root_operation::eval",
                                "the square_root_operation primitive requires that the "
                                "arguments given by the operands array are valid");
                        }

                        auto this_ = this->shared_from_this();
                        return hpx::dataflow(hpx::util::unwrapping(
                            [this_](operands_type&& ops) -> primitive_result_type
                        {

                            switch (ops[0].num_dimensions())
                            {
                            case 0:
                                return this_->square_root_0d(std::move(ops));

                            case 1: HPX_FALLTHROUGH;
                            case 2:
                                return this_->square_root_xd(std::move(ops));

                            default:
                                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                    "square_root_operation::eval",
                                    "left hand side operand has unsupported "
                                    "number of dimensions");
                            }
                        }),
                            detail::map_operands(operands, numeric_operand, args)
                            );
                    }
                };
            }

            // implement 'square_root' for all possible combinations of lhs and rhs
            hpx::future<primitive_result_type> square_root_operation::eval(
                std::vector<primitive_argument_type> const& args) const
            {
                if (operands_.empty())
                {
                    static std::vector<primitive_argument_type> noargs;
                    return std::make_shared<detail::square_root>()->eval(args, noargs);
                }

                return std::make_shared<detail::square_root>()->eval(operands_, args);
            }
        }
    }
}
