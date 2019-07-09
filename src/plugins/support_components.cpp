//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/util/distributed_vector.hpp>

#include <hpx/collectives/all_reduce.hpp>

#include <cstdint>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

////////////////////////////////////////////////////////////////////////////////
using std_int64_t = std::int64_t;
using std_uint8_t = std::uint8_t;

REGISTER_DISTRIBUTED_VECTOR(double);
REGISTER_DISTRIBUTED_VECTOR(std_int64_t);
REGISTER_DISTRIBUTED_VECTOR(std_uint8_t);

HPX_REGISTER_ALLREDUCE(double);
HPX_REGISTER_ALLREDUCE(std_int64_t);
HPX_REGISTER_ALLREDUCE(std_uint8_t);

using blaze_vector_double = blaze::DynamicVector<double>;
using blaze_vector_std_int64_t = blaze::DynamicVector<std::int64_t>;
using blaze_vector_std_uint8_t = blaze::DynamicVector<std::uint8_t>;

HPX_REGISTER_ALLREDUCE(blaze_vector_double);
HPX_REGISTER_ALLREDUCE(blaze_vector_std_int64_t);
HPX_REGISTER_ALLREDUCE(blaze_vector_std_uint8_t);
