//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/util/none_manip.hpp>

#include <iostream>

namespace phylanx { namespace util
{
    namespace detail
    {
        int get_none_manip_index()
        {
            static int const index = std::ios_base::xalloc();
            return index;
        }
    }

    std::ostream& none(std::ostream& stream)
    {
        stream.iword(detail::get_none_manip_index()) = 1;
        return stream;
    }

    std::ostream& nonone(std::ostream& stream)
    {
        stream.iword(detail::get_none_manip_index()) = 0;
        return stream;
    }


    bool is_none(std::ostream& stream)
    {
        return stream.iword(detail::get_none_manip_index()) != 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    none_wrapper::none_wrapper(std::ostream& strm)
        : strm_(strm)
    {
        strm_ << none;
    }
    none_wrapper::~none_wrapper()
    {
        strm_ << nonone;
    }
}}

