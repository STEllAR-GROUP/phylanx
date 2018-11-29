//  Copyright (c) 2017 Hartmut Kaiser
//  Copyright (c) 2018 R. Tohid
//  Copyright (c) 2018 Steven R. Brandt
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_INIT_HPX_HPP)
#define PHYLANX_INIT_HPX_HPP

#include <vector>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace bindings
{
    ///////////////////////////////////////////////////////////////////////////
    void init_hpx_runtime(std::vector<std::string> const& cfg);
    void stop_hpx_runtime();
}}

#endif
