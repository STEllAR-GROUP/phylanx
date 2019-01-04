// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/primitive_argument_type.hpp>

#include <hpx/include/serialization.hpp>

#include <cstdint>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    void topology::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        ar & children_ & name_;
    }

    void topology::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        ar & children_ & name_;
    }

    ///////////////////////////////////////////////////////////////////////////
    void eval_context::serialize(hpx::serialization::output_archive& ar, unsigned)
    {
        std::int32_t mode = mode_;
        ar & mode & variables_;
    }

    void eval_context::serialize(hpx::serialization::input_archive& ar, unsigned)
    {
        std::int32_t mode = 0;
        ar & mode & variables_;
        mode_ = eval_mode(mode);
    }

    ///////////////////////////////////////////////////////////////////////////
    void variable_frame::serialize(
        hpx::serialization::output_archive& ar, unsigned)
    {
        ar & variables_;
    }

    void variable_frame::serialize(
        hpx::serialization::input_archive& ar, unsigned)
    {
        ar & variables_;
    }
}}
