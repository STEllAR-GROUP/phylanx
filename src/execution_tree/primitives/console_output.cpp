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
        primitive_arguments_type&& operands,
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
            &create_console_output, &create_primitive<console_output>,
            R"(args
            Args:

                *args (list of variables) : print a string representation
                                            of the variables to stdout.

            Returns:)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    console_output::console_output(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> console_output::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
                -> primitive_argument_type
            {
                bool init = true;
                for (auto const& arg : args)
                {
                    if (valid(arg) || is_explicit_nil(arg))
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
                }
                hpx::cout << std::endl;

                return {};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
