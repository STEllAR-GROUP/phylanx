# Copyright (c) 2017 R. Tohid
# Copyright (c) 2018-2019 Parsa Amini
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

RUN apt-get update &&                                                               \
    apt-get install --yes --no-install-recommends                                   \
        python3                                                                     \
        python3-dev                                                                 \
        python3-pip                                                                 \
        libblas-dev                                                                 \
        liblapack-dev                                                               \
        libhdf5-dev                                                              && \
    apt-get purge && apt-get clean && rm -rf /var/lib/apt/lists/*                && \
                                                                                    \
    pip3 --no-cache-dir install setuptools                                       && \
    pip3 --no-cache-dir install pytest                                           && \
    pip3 --no-cache-dir install numpy                                            && \
    pip3 --no-cache-dir install astpretty                                        && \
    pip3 --no-cache-dir install flake8                                           && \
                                                                                    \
    git clone --depth=1 https://github.com/pybind/pybind11.git /pybind11-src     && \
    cmake -H/pybind11-src -B/pybind11-build -DPYBIND11_TEST=OFF                  && \
    cmake --build /pybind11-build --target install                               && \
    rm -rf /pybind11-{src,build}                                                 && \
                                                                                    \
    git clone --depth=1 https://bitbucket.org/blaze-lib/blaze.git /blaze-src     && \
    cmake -H/blaze-src -B/blaze-build -DBLAZE_SMP_THREADS=C++11                  && \
    cmake --build /blaze-build --target install                                  && \
    rm -rf /blaze-{src,build}                                                    && \
                                                                                    \
    git clone --depth=1 https://github.com/STEllAR-GROUP/BlazeIterative.git         \
        /blazeiterative-src                                                      && \
    cmake -H/blazeiterative-src -B/blazeiterative-build                             \
        -Dblaze_DIR=/blaze/share/blaze/cmake                                     && \
    cmake --build /blazeiterative-build --target install                         && \
    rm -rf /blazeiterative-{src,build}                                           && \
                                                                                    \
    git clone --depth=1 https://github.com/BlueBrain/HighFive.git /highfive-src  && \
    cmake -H/highfive-src -B/highfive-build -DHIGHFIVE_UNIT_TESTS=OFF               \
        -DHIGHFIVE_EXAMPLES=OFF                                                     \
        -DUSE_BOOST=OFF                                                          && \
    cmake --build /highfive-build --target install                               && \
    rm -rf /highfive-{src,build}
WORKDIR /
CMD /bin/bash

