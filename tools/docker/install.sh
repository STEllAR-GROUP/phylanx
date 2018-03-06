#!/usr/bin/env bash
# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

mkdir -p /phylanx
cp -a /.data /phylanx
make -C /phylanx/build install
