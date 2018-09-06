// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/plugins/booleans/equal.hpp>
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
        struct equal_op
        {
            template <typename T1, typename T2>
            HPX_FORCEINLINE bool operator()(T1 const& t1, T2 const& t2) const
            {
                return t1 == t2;
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const equal::match_data = {
        hpx::util::make_tuple("__eq",
            std::vector<std::string>{
                "_1 == _2", "__eq(_1, _2)", "__eq(_1, _2, _3)"},
            &create_equal, &create_primitive<equal>)
    };

    ///////////////////////////////////////////////////////////////////////////
    equal::equal(primitive_arguments_type&& operands,
            std::string const& name, std::string const& codename)
      : base_type(std::move(operands), name, codename)
    {}
}}}
