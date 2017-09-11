#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

# load common settings
if [ -z ${eigen_src_dir} ] ; then
    . ${scriptdir}/buildbot_common.sh
fi

pythonpath=`which python3`

build_phylanx()
{
    echo "Removing old phylanx build..."
    rm -rf ${phylanx_build_dir}
    mkdir -p ${phylanx_build_dir}
    cd ${phylanx_build_dir}

    set -x
    cmake \
    -DCMAKE_BUILD_TYPE=${buildtype} \
    -DEigen3_DIR=${eigen_build_dir}/share/eigen3/cmake \
    -Dpybind11_DIR=${pybind_build_dir}/share/cmake/pybind11 \
    -DHPX_DIR=${HPX_ROOT}/lib/cmake/HPX \
    -DPHYLANX_WITH_PSEUDO_DEPENDENCIES=On \
    -DPYTHON_EXECUTABLE:FILEPATH=${pythonpath} \
    ${phylanx_src_dir}

    make ${makej}
}

build_phylanx