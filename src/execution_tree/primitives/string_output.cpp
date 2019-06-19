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
        primitive_arguments_type&& operands,
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
            &create_string_output, &create_primitive<string_output>,
            R"(args
            Args:

                *args (object list) : any objects

            Returns:

            A string created by concatenating the string
            representations of the objects in `args`.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    string_output::string_output(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> string_output::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](args_type&& args)
            -> primitive_argument_type
            {
                if (args.empty())
                {
                    return primitive_argument_type{std::string{}};
                }

                std::stringstream strm;
                for (auto const& arg : args)
                {
                    if (valid(arg) || is_explicit_nil(arg))
                    {
                        strm << arg;
                    }
                }

                return primitive_argument_type(strm.str());
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}
