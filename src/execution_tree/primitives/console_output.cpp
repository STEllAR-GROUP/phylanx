//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/console_output.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/util/serialization/ast.hpp>
#include <phylanx/util/serialization/execution_tree.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/components.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::console_output>
    console_output_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    console_output_type, phylanx_console_output_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(console_output_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const console_output::match_data =
    {
        hpx::util::make_tuple("cout",
            std::vector<std::string>{"cout(__1)"},
            &create<console_output>)
    };

    ///////////////////////////////////////////////////////////////////////////
    console_output::console_output(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    namespace detail
    {
        struct console_output : std::enable_shared_from_this<console_output>
        {
            console_output() = default;

        protected:
            using args_type = std::vector<primitive_result_type>;

        public:
            hpx::future<primitive_result_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::util::unwrapping(
                    [this_](args_type && args) -> primitive_result_type
                    {
                        for (auto const& arg : args)
                        {
                            hpx::cout << arg;
                        }
                        hpx::cout << std::endl;

                        return {};
                    }),
                    detail::map_operands(operands, value_operand, args)
                );
            }

        private:
            primitive_argument_type operand_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file and return content
    hpx::future<primitive_result_type> console_output::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::console_output>()->eval(args, noargs);
        }

        return std::make_shared<detail::console_output>()->eval(operands_, args);
    }

    // Never evaluate output operations while defining a function
    bool console_output::bind(std::vector<primitive_argument_type> const&)
    {
        return false;
    }
}}}
