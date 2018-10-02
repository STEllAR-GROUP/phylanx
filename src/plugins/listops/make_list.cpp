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
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const make_list::match_data =
    {
        hpx::util::make_tuple("list",
            std::vector<std::string>{"list(__1)"},
            &create_make_list, &create_primitive<make_list>,
            "args\n"
            "Args:\n"
            "\n"
            "    *args (list of values, optional): a list of values\n"
            "\n"
            "Returns:\n"
            "\n"
            "A Phylanx list populated by the values supplied."
            ),

        hpx::util::make_tuple("make_list",
            std::vector<std::string>{"make_list(__1)"},
            &create_make_list, &create_primitive<make_list>,
            "args\n"
            "Args:\n"
            "\n"
            "    *args (list of values, optional): a list of values\n"
            "\n"
            "Returns:\n"
            "\n"
            "A Phylanx list populated by the values supplied."
            )
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
        primitive_arguments_type const& args) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type && args)
            ->  primitive_argument_type
            {
                return primitive_argument_type{std::move(args)};
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    hpx::future<primitive_argument_type> make_list::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
}}}
