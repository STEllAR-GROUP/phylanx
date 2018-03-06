#!/usr/bin/env bash
# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# This script is to be used on CircleCI for the purpose of installing Phylanx
# to the Docker image
set -x

mkdir -p /phylanx
cd /.data
cp -a . /phylanx
make -C /phylanx/build install
