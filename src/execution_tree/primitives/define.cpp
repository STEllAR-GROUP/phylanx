//  Copyright (c) 2017 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/define.hpp>

#include <hpx/include/util.hpp>

#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    std::vector<match_pattern_type> const define_::match_data =
    {
        // We don't need a creation function as 'define()' is explicitly
        // handled by generate_tree.
        hpx::util::make_tuple("define", "define(__1)", nullptr)
    };
}}}
