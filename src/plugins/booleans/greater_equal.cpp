//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/greater_equal.hpp>
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
        struct greater_equal_op
        {
            template <typename T1, typename T2>
            HPX_FORCEINLINE bool operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 >= t2;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const greater_equal::match_data =
    {
        hpx::util::make_tuple("__ge",
            std::vector<std::string>{
                "_1 >= _2", "__ge(_1, _2)", "__ge(_1, _2, _3)"},
            &create_greater_equal, &create_primitive<greater_equal>,
                R"(arg1, arg2, arg3
                Args:

                    arg1 (number) : A value to compare
                    arg2 (number) : Another value to compare
                    arg3 (boolean, optional) : whether to use a
                                           numeric return value.

                Returns:

                    if arg3 is true
                      return 1 if arg1 >= arg2, 0 otherwise.
                    else
                      return True if arg1 >= arg2, False otherwise.)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    greater_equal::greater_equal(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
