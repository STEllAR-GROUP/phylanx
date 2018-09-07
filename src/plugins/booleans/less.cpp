//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/less.hpp>
#include <phylanx/plugins/booleans/comparison_impl.hpp>

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
        struct less_op
        {
            template <typename T1, typename T2>
            HPX_FORCEINLINE bool operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 < t2;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const less::match_data =
    {
        hpx::util::make_tuple("__lt",
            std::vector<std::string>{
                "_1 < _2", "__lt(_1, _2)", "__lt(_1, _2, _3)"},
            &create_less, &create_primitive<less>,
                "arg1,arg2,arg3\n"
                "Args:\n"
                "\n"
                "    arg1 (number) : A value to compare\n"
                "    arg2 (number) : Another value to compare\n"
                "    arg3 (boolean,optional) : whether to use a\n"
                "                           numeric return value.\n"
                "\n"
                "Returns:\n"
                "\n"
                "    if arg3 is true\n"
                "      return 1 if arg1 < arg2, 0 otherwise.\n"
                "    else\n"
                "      return True if arg1 < arg2, False otherwise.\n"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    less::less(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
