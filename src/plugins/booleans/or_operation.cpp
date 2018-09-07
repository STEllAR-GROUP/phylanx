//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/or_operation.hpp>
#include <phylanx/plugins/booleans/logical_operation_impl.hpp>

#include <hpx/include/util.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        struct or_op
        {
            HPX_FORCEINLINE bool operator()(bool lhs, bool rhs) const
            {
                return lhs || rhs;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const or_operation::match_data =
    {
        hpx::util::make_tuple("__or",
            std::vector<std::string>{"_1 || __2", "__or(_1, __2)"},
            &create_or_operation, &create_primitive<or_operation>,
            "a,b\n"
            "Args:\n"
            "\n"
            "    a (boolean) : a boolean argument\n"
            "    b (boolean) : a boolean argument\n"
            "\n"
            "Returns:\n"
            "\n"
            "The logical or of `a` and `b`."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    or_operation::or_operation(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
