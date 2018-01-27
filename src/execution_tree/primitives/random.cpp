//   Copyright (c) 2017-2018 Hartmut Kaiser
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ast/detail/is_literal_value.hpp>
#include <phylanx/execution_tree/primitives/random.hpp>
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
    phylanx::execution_tree::primitives::random>
    random_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    random_type, phylanx_random_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(random_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const random::match_data =
    {
        hpx::util::make_tuple("random",
            std::vector<std::string>{"random(_1)"},
            &create<random>)
    };

    ///////////////////////////////////////////////////////////////////////////
    random::random(std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct random : std::enable_shared_from_this<random>
        {
            random() = default;

            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() > 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "random::eval",
                        "the random primitive requires"
                            "at most one operand");
                }

                if (!valid(operands[0]))
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "random::eval",
                        "the random primitive requires that the "
                            "arguments given by the operands array are valid");
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](operands_type&& ops) -> primitive_result_type
                    {
                        std::size_t dims = 0;
                        if (!ops.empty())
                        {
                            dims = ops[0].num_dimensions();
                        }

                        switch (dims)
                        {
                        case 0:
                            return this_->random0d(std::move(ops));

                        case 1:
                            return this_->random1d(std::move(ops));

                        case 2:
                            return this_->random2d(std::move(ops));

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "random::eval",
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

            primitive_result_type random0d(operands_type && ops) const
            {
                blaze::Rand<double> gen{};
                return operand_type{gen.generate()};
            }

            primitive_result_type random1d(operands_type && ops) const
            {
                std::size_t dim = ops[0].dimension(0);

                blaze::Rand<blaze::DynamicVector<double>> gen{};

                return operand_type{gen.generate(dim)};
            }

            primitive_result_type random2d(operands_type && ops) const
            {
                auto dim = ops[0].dimensions();

                blaze::Rand<blaze::DynamicMatrix<double>> gen{};

                return operand_type{gen.generate(dim[0], dim[1])};
            }
        };
    }

    hpx::future<primitive_result_type> random::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::random>()->eval(args, noargs);
        }

        return std::make_shared<detail::random>()->eval(operands_, args);
    }
}}}
