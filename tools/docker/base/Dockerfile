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

RUN apt-get update &&                                                               \
    apt-get install --yes --no-install-recommends                                   \
        python3                                                                     \
        python3-dev                                                                 \
        python3-pip                                                                 \
        libblas-dev                                                                 \
        liblapack-dev                                                               \
        libhdf5-dev &&                                                              \
    apt-get purge && apt-get clean && rm -rf /var/lib/apt/lists/*                && \
                                                                                    \
    pip3 --no-cache-dir install setuptools                                       && \
    pip3 --no-cache-dir install pytest                                           && \
    pip3 --no-cache-dir install numpy                                            && \
    pip3 --no-cache-dir install astpretty                                        && \
    pip3 --no-cache-dir install flake8                                           && \
                                                                                    \
    git clone --depth=1 https://github.com/pybind/pybind11.git /pybind11-src     && \
    (                                                                               \
    cd /pybind11-src                                                             && \
    cmake -H. -Bbuild -DPYBIND11_TEST=OFF                                        && \
    cmake --build build -- install                                                  \
    )                                                                            && \
    rm -rf /pybind11-src                                                         && \
                                                                                    \
    git clone --depth=1 https://bitbucket.org/blaze-lib/blaze.git /blaze-src     && \
    (                                                                               \
    cd /blaze-src                                                                && \
    cmake -H. -Bbuild -DBLAZE_SMP_THREADS=C++11                                  && \
    cmake --build build -- install                                                  \
    )                                                                            && \
    rm -rf /blaze-src                                                            && \

    git clone --depth=1 https://github.com/STEllAR-GROUP/BlazeIterative.git         \
        /blazeiterative-src                                                      && \
    (                                                                               \
    cd /blazeiterative-src                                                       && \
    cmake -H. -Bbuild -Dblaze_DIR=/blaze/share/blaze/cmake                       && \
    cmake --build build -- install                                                  \
    )                                                                            && \
    rm -rf /blaze-iterative-src                                                  && \
                                                                                    \
    git clone --depth=1 https://github.com/BlueBrain/HighFive.git /highfive-src  && \
    (                                                                               \
    cd /highfive-src                                                             && \
    cmake -H. -Bbuild -DUSE_BOOST=Off                                            && \
    cmake --build build -- install                                                  \
    )                                                                            && \
    rm -rf /highfive-src
WORKDIR /
CMD /bin/bash
