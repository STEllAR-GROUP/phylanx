# Copyright (c) 2018 Adrian Serio
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.3.2 FATAL_ERROR)
cmake_policy(VERSION 3.3.2)

project (phylanx_external)
find_package (Phylanx REQUIRED)
add_executable (phylanx_external phylanx_external.cpp)
phylanx_setup_target(phylanx_external)
