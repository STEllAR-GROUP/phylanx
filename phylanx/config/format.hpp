//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_CONFIG_FORMAT_HPP)
#define PHYLANX_CONFIG_FORMAT_HPP

#include <hpx/config.hpp>

// the HPX string formatting has changed after V1.1 was released
#if HPX_VERSION_FULL <= 0x010100
#define PHYLANX_FORMAT_SPEC(n)  "%" #n "%"
#else
#define PHYLANX_FORMAT_SPEC(n)  "{" #n "}"
#endif

#endif
