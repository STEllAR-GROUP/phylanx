//  Copyright (c) 2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/util/repr_manip.hpp>

#include <iostream>

namespace phylanx { namespace util
{
    namespace detail
    {
        int get_repr_manip_index()
        {
            static int const index = std::ios_base::xalloc();
            return index;
        }
    }

    std::ostream& repr(std::ostream& stream)
    {
        stream.iword(detail::get_repr_manip_index()) = 1;
        return stream;
    }

    std::ostream& norepr(std::ostream& stream)
    {
        stream.iword(detail::get_repr_manip_index()) = 0;
        return stream;
    }


    bool is_repr(std::ostream& stream)
    {
        return stream.iword(detail::get_repr_manip_index()) != 0;
    }
}}

