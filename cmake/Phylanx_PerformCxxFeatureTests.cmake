# Copyright (c) 2020 Hartmut Kaiser
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include(Phylanx_AddConfigTests)

# ##############################################################################
# C++ feature tests
# ##############################################################################
function(phylanx_perform_cxx_feature_tests)
  phylanx_check_for_cxx17_shared_ptr_array(
    DEFINITIONS PHYLANX_HAVE_CXX17_SHARED_PTR_ARRAY
  )
endfunction()
