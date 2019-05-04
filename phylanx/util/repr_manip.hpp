//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_UTIL_REPR_MANIP_HPP)
#define PHYLANX_UTIL_REPR_MANIP_HPP

#include <phylanx/config.hpp>

#include <iosfwd>

namespace phylanx { namespace util
{
    PHYLANX_EXPORT std::ostream& repr(std::ostream& stream);
    PHYLANX_EXPORT std::ostream& norepr(std::ostream& stream);
    PHYLANX_EXPORT bool is_repr(std::ostream& stream);

    struct PHYLANX_EXPORT repr_wrapper
    {
        repr_wrapper(std::ostream& strm);
        ~repr_wrapper();

        std::ostream& strm_;
    };
}}

#endif

