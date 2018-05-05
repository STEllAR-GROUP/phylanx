# Copyright (c) 2017 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# This is the base image used for building Phylanx. It adds the following to
# the HPX image containing the debug build of HPX built during HPX's CI
# workflow:
#
#  - Python 3
#  - Python Pacakges
#      - SetupTools
#      - flake
#      - PyTest
#      - NumPy
#  - pybind11
#  - OpenBLAS and LAPACK
#  - blaze
#  - HDF5
#  - HighFive

FROM stellargroup/hpx:dev

RUN apt-get update  &&                                                           \
    apt-get -y install wget &&                                                   \
    apt-get -y install python3 python3-dev python3-pip &&                        \
    apt-get -y install libblas-dev liblapack-dev &&                              \
    apt-get -y install libhdf5-dev &&                                            \
    pip3 install setuptools &&                                                   \
    pip3 install pytest &&                                                       \
    pip3 install numpy &&                                                        \
    pip3 install flake8 &&                                                       \
    cd / &&                                                                      \
    git clone --depth 1 https://github.com/pybind/pybind11.git pybind11-src &&   \
    cd pybind11-src &&                                                           \
    mkdir build && cd build &&                                                   \
    cmake  -DPYBIND11_TEST=OFF .. &&                                             \
    make install &&                                                              \
    cd / &&                                                                      \
    rm -rf /pybind11-src &&                                                      \
    git clone --depth 1 https://bitbucket.org/blaze-lib/blaze.git blaze-src &&   \
    cd blaze-src &&                                                              \
    mkdir build && cd build &&                                                   \
    cmake -DBLAZE_SMP_THREADS=C++11 .. &&                                        \
    make install &&                                                              \
    cd / &&                                                                      \
    rm -rf /blaze-src  &&                                                        \
    git clone --depth 1 https://github.com/BlueBrain/HighFive.git highfive-src &&\
    cd highfive-src &&                                                           \
    mkdir build && cd build &&                                                   \
    cmake -DUSE_BOOST=Off .. &&                                                  \
    make install &&                                                              \
    cd / &&                                                                      \
    rm -rf /highfive-src
WORKDIR /
CMD /bin/bash
