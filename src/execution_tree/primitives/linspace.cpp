//  Copyright (c) 2018 R. Tohid
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/execution_tree/primitives/linspace.hpp>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
        phylanx::execution_tree::primitives::linspace> linspace_type ;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(linspace_type, phylanx_linspace_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(linspace_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const linspace::match_data =
    {
        hpx::util::make_tuple("linspace",
            std::vector<std::string>{"linspace(_1, _2, _3)"}, &create<linspace>)
    };

    ///////////////////////////////////////////////////////////////////////////
    linspace::linspace(std::vector<primitive_argument_type>&& args)
      : base_primitive(std::move(args))
    {}

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        using operand_type = ir::node_data<double>;
        using args_type = std::vector<operand_type>;
        using vector_type = blaze::DynamicVector<double>;
        struct linspace : std::enable_shared_from_this<linspace>
        {
            linspace() = default;

        protected:
            primitive_result_type linspace1d(args_type&& args) const
            {
                double start = extract_integer_value(args[0]);
                double stop = extract_integer_value(args[1]);
                double num_samples = extract_integer_value(args[2]);

                if (num_samples < 1)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::detail::linspace1d",
                        "the linspace primitive requires at least one interval");
                }
                else if (1 == num_samples)
                {
                    vector_type result{start};            
                    return operand_type{std::move(result)};
                }
                else
                {
                  auto result = vector_type(num_samples);
                  double dx = (stop - start) / (num_samples - 1);
                  for (std::size_t i = 0; i < num_samples; i++)
                  {
                    result[i] = start + dx * i;
                  }
                  return operand_type{std::move(result)};
                }
            }

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& args)
            {
                if (args.size() != 3)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "phylanx::execution_tree::primitives::linspace",
                        "The linspace primitive requires exactly three arguments.");
                }

                for (std::size_t i = 0; i != args.size(); ++i)
                {
                    if (!valid(args[i]))
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "linspace::eval",
                            "at least one of the arguments passed to linspace"
                               "is not valid.");
                    }
                }

                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_result_type
                    {
                        return this_->linspace1d(std::move(args));
                    }),
                    detail::map_operands(args, numeric_operand, args));
            }
        };
    }

    hpx::future<primitive_result_type> linspace::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::linspace>()->eval(args);
        }
        return std::make_shared<detail::linspace>()->eval(operands_);
    }
}}}
