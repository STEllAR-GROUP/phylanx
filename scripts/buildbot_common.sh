#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

if [ -z ${buildtype} ] ; then
    buildtype=RelWithDebInfo
fi

if [ -z ${myarch} ] ; then
    echo "myarch is not set. Please source the appropriate settings in the ${scriptdir} directory.";
    kill -INT $$

fi

nprocs=`nproc`
let one_fourth=$nprocs/4
makej=-j${one_fourth}

top=${HOME}/src/phylanx
buildprefix=${top}/build-${myarch}

boost_src_dir=/dev/shm/src/boost
boost_build_dir=${buildprefix}/boost-1.65.0

phylanx_src_dir=${top}/src/phylanx
phylanx_build_dir=${buildprefix}/phylanx-${buildtype}

pybind_src_dir=${top}/src/pybind11-2.2.0
pybind_build_dir=${buildprefix}/pybind-${buildtype}

eigen_src_dir=${top}/src/eigen-eigen-5a0156e40feb
eigen_build_dir=${buildprefix}/eigen3-${buildtype}

hpx_src_dir=${top}/src/hpx
HPX_ROOT=${buildprefix}/hpx-${buildtype}

# go up one directory
basedir=$(dirname ${scriptdir})
echo "Basedir: ${basedir}"
