//  Copyright (c) 2018 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/linspace.hpp>
#include <phylanx/ir/node_data.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_linspace(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands, std::string const& name)
    {
        static std::string type("linspace");
        return create_primitive_component(
            locality, type, std::move(operands), name);
    }

    match_pattern_type const linspace::match_data =
    {
        hpx::util::make_tuple("linspace",
            std::vector<std::string>{"linspace(_1, _2, _3)"},
            &create_linspace, &create_primitive<linspace>)
    };

    ///////////////////////////////////////////////////////////////////////////
    linspace::linspace(std::vector<primitive_argument_type>&& args)
      : primitive_component_base(std::move(args))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct linspace : std::enable_shared_from_this<linspace>
        {
            linspace() = default;

        protected:
            using arg_type = ir::node_data<double>;
            using args_type = std::vector<arg_type>;
            using vector_type = blaze::DynamicVector<double>;

            primitive_argument_type linspace1d(args_type&& args) const
            {
                double start = args[0][0];
                double stop = args[1][0];
                double num_samples = args[2][0];

                if (num_samples < 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::detail::linspace1d",
                        "the linspace primitive requires at least one interval");
                }

                if (1 == num_samples)
                {
                    vector_type result{start};
                    return primitive_argument_type{arg_type{std::move(result)}};
                }
                else
                {
                    auto result = vector_type(num_samples);
                    double dx = (stop - start) / (num_samples - 1);
                    for (std::size_t i = 0; i < num_samples; i++)
                    {
                        result[i] = start + dx * i;
                    }
                    return primitive_argument_type{arg_type{std::move(result)}};
                }
            }

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                if (operands.size() != 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::linspace",
                        "The linspace primitive requires exactly three arguments.");
                }

                for (std::size_t i = 0; i != operands.size(); ++i)
                {
                    if (!valid(operands[i]))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linspace::eval",
                            "at least one of the arguments passed to linspace"
                               "is not valid.");
                    }
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_argument_type
                    {
                        return this_->linspace1d(std::move(args));
                    }),
                    detail::map_operands(operands, functional::numeric_operand{}, args));
            }
        };
    }

    hpx::future<primitive_argument_type> linspace::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::linspace>()->eval(args, noargs);
        }
        return std::make_shared<detail::linspace>()->eval(operands_, args);
    }
}}}
