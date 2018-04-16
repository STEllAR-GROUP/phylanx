//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/access_argument.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const access_argument::match_data =
    {
        hpx::util::make_tuple("access-argument",
            std::vector<std::string>{},
            nullptr, &create_primitive<access_argument>)
    };

    ///////////////////////////////////////////////////////////////////////////
    access_argument::access_argument(
            std::vector<primitive_argument_type>&& args,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(args), name, codename, true)
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_argument::access_argument",
                generate_error_message(
                    "the access_argument primitive expects to be initialized "
                    "with exactly one argument"));
        }

        argnum_ = extract_integer_value(operands_[0]);
    }

    hpx::future<primitive_argument_type> access_argument::eval(
        std::vector<primitive_argument_type> const& params) const
    {
        if (argnum_ >= params.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                    "access_argument::eval",
                generate_error_message(hpx::util::format(
                    "argument count out of bounds, expected at least "
                        PHYLANX_FORMAT_SPEC(1) " argument(s) while only "
                        PHYLANX_FORMAT_SPEC(2) " argument(s) were supplied",
                        argnum_ + 1, params.size())));
        }
        return value_operand(params[argnum_], params, name_, codename_);
    }
}}}
