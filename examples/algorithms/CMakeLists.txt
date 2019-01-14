# Copyright (c) 2017 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(subdirs
    als
    lir
    lra
<<<<<<< HEAD
    kmeans
=======
>>>>>>> 0d4e446787bba0ddd326b883757a37e02d662fdb
    nn
   )

foreach(subdir ${subdirs})
  add_subdirectory(${subdir})
endforeach()
