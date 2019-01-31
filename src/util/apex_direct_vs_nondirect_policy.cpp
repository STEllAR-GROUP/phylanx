//  Copyright (c) 2019 M. A. H. Monil 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include <hpx/config.hpp>
#include <phylanx/util/apex_direct_vs_nondirect_policy.hpp>
#include <iostream>


namespace phylanx { namespace util
{
#if defined(HPX_HAVE_APEX) && defined(PHYLANX_HAVE_DIRECT_VS_NONDIRECT_POLICY)
	//apex_direct_vs_nondirect_policy* apex_direct_vs_nondirect_policy::instance = nullptr;

#endif //HPX_HAVE_APEX && HPX_HAVE_DIRECT_VS_NONDIRECT_POLICY
}}

