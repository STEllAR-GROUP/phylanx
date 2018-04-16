//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/console_output.hpp>

#include <hpx/include/iostreams.hpp>
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
    primitive create_console_output(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("cout");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const console_output::match_data =
    {
        hpx::util::make_tuple("cout",
            std::vector<std::string>{"cout(__1)"},
            &create_console_output, &create_primitive<console_output>)
    };

    ///////////////////////////////////////////////////////////////////////////
    console_output::console_output(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    namespace detail
    {
        struct console_output : std::enable_shared_from_this<console_output>
        {
            console_output() = default;

        protected:
            using args_type = std::vector<primitive_argument_type>;

        public:
            hpx::future<primitive_argument_type> eval(
                std::vector<primitive_argument_type> const& operands,
                std::vector<primitive_argument_type> const& args)
            {
                auto this_ = this->shared_from_this();
                return hpx::dataflow(hpx::launch::sync,
                    hpx::util::unwrapping(
                    [this_](args_type&& args) -> primitive_argument_type
                    {
                        bool init = true;
                        for (auto const& arg : args)
                        {
                            if (init)
                            {
                                init = false;
                            }
                            else
                            {
                                // Put spaces in the output to match Python
                                hpx::cout << ' ';
                            }
                            hpx::cout << arg;
                        }
                        hpx::cout << std::endl;

                        return {};
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
    hpx::future<primitive_argument_type> console_output::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return std::make_shared<detail::console_output>()->eval(args, noargs);
        }
        return std::make_shared<detail::console_output>()->eval(operands_, args);
    }
}}}
