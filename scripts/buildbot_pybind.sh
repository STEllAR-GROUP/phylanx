#!/bin/bash -e

set -x

steps=all
if [ $# -eq 3 ] ; then
    buildtype=$1
    step=$2
fi
echo "Component HPX, buildtype ${buildtype}, step ${step}"

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

# load common settings
if [ -z ${eigen_src_dir} ] ; then
    . ${scriptdir}/buildbot_common.sh
fi

pythonpath=`which python3`

configure_pybind()
{
    if [ ! -d ${pybind_src_dir} ] ; then
        cd $(dirname ${pybind_src_dir})
        if [ ! -f v2.2.0.tar.gz ] ; then
            wget https://github.com/pybind/pybind11/archive/v2.2.0.tar.gz
        fi
        tar -xzf v2.2.0.tar.gz
    fi
    echo "Removing old pybind11 build..."
    rm -rf ${pybind_build_dir}
    mkdir -p ${pybind_build_dir}
    cd ${pybind_build_dir}
    set -x
    export LDFLAGS="-ldl -lutil"
    cmake \
    -DCMAKE_BUILD_TYPE=${buildtype} \
    -DDOWNLOAD_CATCH=1 \
    -DPYTHON_EXECUTABLE:FILEPATH=${pythonpath} \
    -DCMAKE_INSTALL_PREFIX=. \
    ${pybind_src_dir}
}

build_pybind()
{
    cd ${pybind_build_dir}
    make ${makej}
    make install
}

if [ ${step} == "all" ] || [ ${step} == "configure" ] ; then
    configure_pybind
fi
if [ ${step} == "all" ] || [ ${step} == "compile" ] ; then
    build_pybind
fi
