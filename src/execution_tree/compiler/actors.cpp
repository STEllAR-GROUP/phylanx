//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/compiler/actors.hpp>

#include <hpx/include/serialization.hpp>

namespace phylanx { namespace execution_tree { namespace compiler
{
    ///////////////////////////////////////////////////////////////////////////
    void function::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar & arg_ & name_;
    }

    void function::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar & arg_ & name_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void entry_point::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar & name_ & code_;
    }

    void entry_point::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar & name_ & code_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void program::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar & code_ & scratchpad_;
    }

    void program::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar & code_ & scratchpad_;
    }
}}}

