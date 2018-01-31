//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/string_output.hpp>
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
#include <vector>
#include <string>
#include <sstream>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
typedef hpx::components::component<
    phylanx::execution_tree::primitives::string_output>
    string_output_type;
HPX_REGISTER_DERIVED_COMPONENT_FACTORY(
    string_output_type, phylanx_string_output_component,
    "phylanx_primitive_component", hpx::components::factory_enabled)
HPX_DEFINE_GET_COMPONENT_TYPE(string_output_type::wrapped_type)

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const string_output::match_data =
    {
        hpx::util::make_tuple("string",
            std::vector<std::string>{"string(__1)"},
            &create<string_output>)
    };

    ///////////////////////////////////////////////////////////////////////////
    string_output::string_output(
            std::vector<primitive_argument_type>&& operands)
      : base_primitive(std::move(operands))
    {}

    namespace detail
    {
        struct string_output : std::enable_shared_from_this<string_output>
        {
            string_output() = default;

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
                        if (args.empty())
                        {
                            return primitive_result_type{std::string{}};
                        }

                        std::stringstream strm;
                        for (auto const& arg : args)
                        {
                            strm << arg;
                        }

                        return primitive_result_type(strm.str());
                    }),
                    detail::map_operands(
                        operands, functional::value_operand{}, args));
            }

        private:
            primitive_argument_type operand_;
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    // write data to given file and return content
    hpx::future<primitive_result_type> string_output::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::string_output>()->eval(args, noargs);
        }

        return std::make_shared<detail::string_output>()->eval(operands_, args);
    }
}}}
