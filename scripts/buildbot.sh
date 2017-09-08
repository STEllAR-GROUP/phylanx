#!/bin/bash -e

set -x

top=${HOME}/src/phylanx
buildprefix=${top}/hpc-gcc-build
pybind_src_dir=${top}/src/pybind11
pybind_build_dir=${buildprefix}/pybind
eigen_src_dir=${top}/src/eigen-eigen-5a0156e40feb
eigen_build_dir=${buildprefix}/eigen3
HPX_ROOT=${buildprefix}/hpx-jemalloc-RelWithDebInfo-apex

# where is this script?
scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# go up one directory
basedir=$(dirname ${scriptdir})
echo ${basedir}

build_phylanx()
{

    builddir=${basedir}/build
    rm -rf ${builddir}
    mkdir -p ${builddir}
    cd ${builddir}

    cmake \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DEigen3_DIR=${eigen_build_dir}/share/eigen3/cmake \
    -Dpybind11_DIR=${pybind_build_dir}/share/cmake/pybind11 \
    -DHPX_DIR=${HPX_ROOT}/lib/cmake/HPX \
    -DPHYLANX_WITH_PSEUDO_DEPENDENCIES=On \
    ${basedir}

    make VERBOSE=1
}

build_eigen3()
{
    if [ ! -d ${eigen_src_dir} ] ; then
        cd ${top}/src
        if [ ! -f 3.3.4.tar.gz ] ; then
            wget http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
        fi
        tar -xzf 3.3.4.tar.gz
    fi
    rm -rf ${eigen_build_dir}
    mkdir -p ${eigen_build_dir}
    cd ${eigen_build_dir}
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=. ${eigen_src_dir}
    make install
}

build_pybind()
{
    if [ ! -d ${pybind_src_dir} ] ; then
        cd $(dirname ${pybind_src_dir})
        git clone git@github.com:pybind/pybind11.git
    fi
    rm -rf ${pybind_build_dir}
    mkdir -p ${pybind_build_dir}
    cd ${pybind_build_dir}
    cmake -DCMAKE_INSTALL_PREFIX=. -DDOWNLOAD_CATCH=1 ${pybind_src_dir}
    make -j8
    make install
}

#build_eigen3
#build_pybind
build_phylanx