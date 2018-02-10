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
            std::vector<primitive_argument_type>&& args)
      : primitive_component_base(std::move(args))
    {
        if (operands_.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "access_argument::access_argument",
                "the access_argument primitive expects to be initialized with "
                    "exactly one argument");
        }

        argnum_ = extract_integer_value(operands_[0]);
    }

    primitive_argument_type access_argument::eval_direct(
        std::vector<primitive_argument_type> const& params) const
    {
        if (argnum_ >= params.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "phylanx::execution_tree::primitives::"
                "access_argument::eval_direct",
                "argument count out of bounds, expected at least " +
                    std::to_string(argnum_ + 1) + " argument(s) "
                    "while only " + std::to_string(params.size()) +
                    " argument(s) were supplied");
        }
        return value_operand_ref_sync(params[argnum_], params);
    }
}}}
