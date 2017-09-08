#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

top=${HOME}/src/phylanx
buildprefix=${top}/hpc-gcc-build
pybind_src_dir=${top}/src/pybind11
pybind_build_dir=${buildprefix}/pybind
eigen_src_dir=${top}/src/eigen-eigen-5a0156e40feb
eigen_build_dir=${buildprefix}/eigen3
HPX_ROOT=${buildprefix}/hpx-jemalloc-RelWithDebInfo-apex

# go up one directory
basedir=$(dirname ${scriptdir})
echo ${basedir}
