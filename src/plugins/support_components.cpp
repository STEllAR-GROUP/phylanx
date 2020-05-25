//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/util/distributed_vector.hpp>
#include <phylanx/util/distributed_matrix.hpp>

#include <hpx/collectives/all_reduce.hpp>

#include <cstddef>
#include <cstdint>
#include <utility>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

////////////////////////////////////////////////////////////////////////////////
using std_int64_t = std::int64_t;
using std_uint8_t = std::uint8_t;

REGISTER_DISTRIBUTED_VECTOR(double);
REGISTER_DISTRIBUTED_VECTOR(std_int64_t);
REGISTER_DISTRIBUTED_VECTOR(std_uint8_t);

REGISTER_DISTRIBUTED_MATRIX(double);
REGISTER_DISTRIBUTED_MATRIX(std_int64_t);
REGISTER_DISTRIBUTED_MATRIX(std_uint8_t);

HPX_REGISTER_ALLREDUCE(double);
HPX_REGISTER_ALLREDUCE(std_int64_t);
HPX_REGISTER_ALLREDUCE(std_uint8_t);

using blaze_vector_double = blaze::DynamicVector<double>;
using blaze_vector_std_int64_t = blaze::DynamicVector<std::int64_t>;
using blaze_vector_std_uint8_t = blaze::DynamicVector<std::uint8_t>;

HPX_REGISTER_ALLREDUCE(blaze_vector_double);
HPX_REGISTER_ALLREDUCE(blaze_vector_std_int64_t);
HPX_REGISTER_ALLREDUCE(blaze_vector_std_uint8_t);

using blaze_matrix_double =      blaze::DynamicMatrix<double>;
using blaze_matrix_std_int64_t = blaze::DynamicMatrix<std::int64_t>;
using blaze_matrix_std_uint8_t = blaze::DynamicMatrix<std::uint8_t>;

HPX_REGISTER_ALLREDUCE(blaze_matrix_double);
HPX_REGISTER_ALLREDUCE(blaze_matrix_std_int64_t);
HPX_REGISTER_ALLREDUCE(blaze_matrix_std_uint8_t);

///////////////////////////////////////////////////////////////////////////////
using std_pair_double_size_t = std::pair<double, std::size_t>;
using std_pair_int64_t_size_t = std::pair<std::int64_t, std::size_t>;
using std_pair_uint8_t_size_t = std::pair<std::uint8_t, std::size_t>;

HPX_REGISTER_ALLREDUCE(std_pair_double_size_t);
HPX_REGISTER_ALLREDUCE(std_pair_int64_t_size_t);
HPX_REGISTER_ALLREDUCE(std_pair_uint8_t_size_t);
