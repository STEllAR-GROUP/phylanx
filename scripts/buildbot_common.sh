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
let one_half=$nprocs/2
makej="-j ${one_half} -l ${nprocs}"

tmptop=$( dirname "${scriptdir}" )
top=${tmptop}/buildbot
sourcedir=${top}/src
buildprefix=${top}/build-${myarch}

phylanx_src_dir=${tmptop}
phylanx_build_dir=${buildprefix}/phylanx-${buildtype}
phylanx_install_dir=${buildprefix}/phylanx-${buildtype}-install

pybind_src_dir=${sourcedir}/pybind11-2.2.0
pybind_build_dir=${buildprefix}/pybind-${buildtype}

eigen_src_dir=${sourcedir}/eigen-eigen-5a0156e40feb
eigen_build_dir=${buildprefix}/eigen3-${buildtype}

hpx_src_dir=${sourcedir}/hpx
HPX_ROOT=${buildprefix}/hpx-${buildtype}

# go up one directory
basedir=$(dirname ${scriptdir})
echo "Basedir: ${basedir}"
