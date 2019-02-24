//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/util/hashed_string.hpp>

#include <hpx/include/serialization.hpp>

#include <iostream>
#include <string>

namespace phylanx { namespace util
{
    ///////////////////////////////////////////////////////////////////////////
    void hashed_string::serialize(
        hpx::serialization::output_archive& ar, unsigned)
    {
        ar & key_ & hash_;
    }

    void hashed_string::serialize(
        hpx::serialization::input_archive& ar, unsigned)
    {
        ar & key_ & hash_;
    }

    ///////////////////////////////////////////////////////////////////////////
    std::ostream& operator<<(std::ostream& os, hashed_string const& s)
    {
        os << s.key_;
        return os;
    }
}}


