//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/debug_output.hpp>
#include <phylanx/execution_tree/primitives/generic_function.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    primitive create_debug_output(hpx::id_type const& locality,
        primitive_arguments_type&& operands,
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
            &create_debug_output, &create_primitive<debug_output>,
            R"(args
            Args:

                *args (list of variables) : print a string representation
                                            of the variables to stderr.

            Returns:)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    debug_output::debug_output(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    hpx::future<primitive_argument_type> debug_output::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](args_type && args)
            -> primitive_argument_type
            {
                for (auto const& arg : args)
                {
                    if (valid(arg) || is_explicit_nil(arg))
                    {
                        hpx::consolestream << arg;
                    }
                }
                hpx::consolestream << std::endl << hpx::flush;

                return {};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_,
                std::move(ctx)));
    }
}}}

///////////////////////////////////////////////////////////////////////////////
HPX_PLAIN_ACTION(phylanx::execution_tree::primitives::locality_id, locality);

namespace phylanx { namespace execution_tree { namespace primitives
{
    match_pattern_type const locality_match_data =
    {
        hpx::util::make_tuple(
            "locality", std::vector<std::string>{"locality()"},
            &create_generic_function< ::locality>,
            &create_primitive<generic_function< ::locality>>, R"(
            locality

            Args:

                none

            Returns:

            The locality id of the currently executing code)")
    };

    hpx::future<primitive_argument_type> locality_id(
        phylanx::execution_tree::primitive_arguments_type const&,
        phylanx::execution_tree::primitive_arguments_type const&,
        std::string const&, std::string const&, eval_context)
    {
        std::int64_t locality_ =
            hpx::naming::get_locality_id_from_id(hpx::find_here());
        return hpx::make_ready_future(primitive_argument_type(locality_));
    }
}}}
