//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/debug_output.hpp>

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
    primitive create_debug_output(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("debug");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const debug_output::match_data =
    {
        hpx::util::make_tuple("debug",
            std::vector<std::string>{"debug(__1)"},
            &create_debug_output, &create_primitive<debug_output>)
    };

    ///////////////////////////////////////////////////////////////////////////
    debug_output::debug_output(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> debug_output::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](args_type && args) -> primitive_argument_type
        {
            for (auto const& arg : args)
            {
                hpx::consolestream << arg;
            }
            hpx::consolestream << std::endl;

            return {};
        }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
    ///////////////////////////////////////////////////////////////////////////
    // write data to given file and return content
    hpx::future<primitive_argument_type> debug_output::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
