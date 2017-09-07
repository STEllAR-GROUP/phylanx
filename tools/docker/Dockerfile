# Copyright (c) 2017 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

FROM stellargroup/hpx:dev

# Install Eigen library (http://eigen.tuxfamily.org/)
RUN apt-get install wget &&                                                      \
    wget http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2 &&                   \
    mkdir -p eigen/3.3.4 &&                                                      \
    mkdir eigen-src && tar xf 3.3.4.tar.bz2 -C eigen-src --strip-components 1 && \
    cd eigen-src &&                                                              \
    mkdir build && cd build &&                                                   \
    cmake -DCMAKE_INSTALL_PREFIX=/eigen/3.3.4 .. &&                              \
    make install &&                                                              \
    rm -r /eigen-src

