// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/string_output.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_string_output(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
    {
        static std::string type("string");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const string_output::match_data =
    {
        hpx::util::make_tuple("string",
            std::vector<std::string>{"string(__1)"},
            &create_string_output, &create_primitive<string_output>)
    };

    ///////////////////////////////////////////////////////////////////////////
    string_output::string_output(
            std::vector<primitive_argument_type>&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> string_output::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::util::unwrapping(
            [this_](args_type && args) -> primitive_argument_type
            {
                if (args.empty())
                {
                    return primitive_argument_type{std::string{}};
                }

                std::stringstream strm;
                for (auto const& arg : args)
                {
                    strm << arg;
                }

                return primitive_argument_type(strm.str());
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args));
    }

    ///////////////////////////////////////////////////////////////////////////
    // Write data to given file and return content
    hpx::future<primitive_argument_type> string_output::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
