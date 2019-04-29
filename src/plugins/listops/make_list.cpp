//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/listops/make_list.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    constexpr char const* const helpstring = R"(
        args
        Args:

            *args (list of values, optional): a list of values

        Returns:

        A Phylanx list populated by the values supplied.
    )";

    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const make_list::match_data =
    {
        match_pattern_type{"list",
            std::vector<std::string>{"list(__1)"},
            &create_make_list, &create_primitive<make_list>, helpstring
        },

        match_pattern_type{"make_list",
            std::vector<std::string>{"make_list(__1)"},
            &create_make_list, &create_primitive<make_list>, helpstring
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    make_list::make_list(
            primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> make_list::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type && args)
            ->  primitive_argument_type
            {
                if (args.size() == 1 && !valid(args[0]) &&
                    is_implicit_nil(args[0]))
                {
                    return primitive_argument_type{primitive_arguments_type{}};
                }
                return primitive_argument_type{std::move(args)};
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
