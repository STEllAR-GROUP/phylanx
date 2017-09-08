#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

# load common settings
if [ -z ${eigen_src_dir} ] ; then
    . ${scriptdir}/buildbot_common.sh
fi

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

build_pybind