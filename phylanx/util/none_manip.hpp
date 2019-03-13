//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_NONE_MANIP_HPP)
#define PHYLANX_UTIL_NONE_MANIP_HPP

#include <phylanx/config.hpp>

#include <iosfwd>

namespace phylanx { namespace util
{
    PHYLANX_EXPORT std::ostream& none(std::ostream& stream);
    PHYLANX_EXPORT std::ostream& nonone(std::ostream& stream);
    PHYLANX_EXPORT bool is_none(std::ostream& stream);

    struct PHYLANX_EXPORT none_wrapper
    {
        none_wrapper(std::ostream& strm);
        ~none_wrapper();

        std::ostream& strm_;
    };
}}

#endif

