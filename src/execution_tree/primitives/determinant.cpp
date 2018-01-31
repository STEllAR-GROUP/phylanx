//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/determinant.hpp>
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
        phylanx::execution_tree::primitives::determinant
    > determinant_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    determinant_type, phylanx_determinant_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(determinant_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const determinant::match_data =
    {
        hpx::util::make_tuple("determinant",
            std::vector<std::string>{"determinant(_1)"},
            &create<determinant>)
    };

    ///////////////////////////////////////////////////////////////////////////
    determinant::determinant(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct determinant : std::enable_shared_from_this<determinant>
        {
            determinant() = default;

        private:
            using operand_type = ir::node_data<double>;
            using operands_type = std::vector<operand_type>;

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        "the determinant primitive requires"
                            "exactly one operand");
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "determinant::eval",
                        "the determinant primitive requires that the "
                            "argument given by the operands array is valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t dims = ops[0].num_dimensions();
                        switch (dims)
                        {
                        case 0:
                            return this_->determinant0d(std::move(ops));

                        case 2:
                            return this_->determinant2d(std::move(ops));

                        case 1: HPX_FALLTHROUGH;
                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "determinant::eval",
                                "left hand side operand has unsupported "
                                    "number of dimensions");
                        }
                    }),
                    detail::map_operands(
                        operands, functional::numeric_operand{}, args));
            }

        protected:
            primitive_result_type determinant0d(operands_type && ops) const
            {
                return std::move(ops[0]);       // no-op
            }

            primitive_result_type determinant2d(operands_type && ops) const
            {
                double d = blaze::det(ops[0].matrix());
                return operand_type(d);
            }
        };
    }

    hpx::future<primitive_result_type> determinant::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::determinant>()->eval(args, noargs);
        }

        return std::make_shared<detail::determinant>()->eval(operands_, args);
    }
}}}
