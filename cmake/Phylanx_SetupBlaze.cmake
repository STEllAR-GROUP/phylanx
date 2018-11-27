# Copyright (c) 2017 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# setup Blaze as a dependency
macro(phylanx_setup_blaze)
  find_package(LAPACK REQUIRED)

  include_directories(${BLAS_INCLUDE_DIR})

  find_package(blaze NO_CMAKE_PACKAGE_REGISTRY)
  if(NOT blaze_FOUND)
    phylanx_error("Blaze could not be found. Please specify blaze_DIR to assist locating it.")
  endif()
  include("${blaze_DIR}/blaze-config-version.cmake")
  phylanx_info("Blaze version: " "${PACKAGE_VERSION}")

  # Make sure HPX is used as the parallelization target for Blaze
  phylanx_add_config_define(BLAZE_USE_HPX_THREADS 1)
  phylanx_add_config_define(BLAZE_USE_SHARED_MEMORY_PARALLELIZATION 1)
  if(MSVC)
    phylanx_add_config_cond_define(NOMINMAX)
  endif()

  # Add iterative solvers from BlazeIterative
  if(PHYLANX_WITH_ITERATIVE_SOLVERS)
    find_package(BlazeIterative)
    if(BLAZEITERATIVE_FOUND)
      phylanx_add_config_define(PHYLANX_HAVE_BLAZE_ITERATIVE)
    endif()
  endif()

  # Add tensors from BlazeTensors
  if(PHYLANX_WITH_BLAZE_TENSOR)
    find_package(BlazeTensor)
    if(NOT BlazeTensor_FOUND)
      phylanx_error("BlazeTensor could not be found. Please specify BlazeTensor_DIR to assist locating it.")
    endif()
    phylanx_add_config_define(PHYLANX_HAVE_BLAZE_TENSOR)
  endif()

endmacro()
