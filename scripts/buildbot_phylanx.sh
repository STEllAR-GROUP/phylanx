#!/bin/bash -e

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

configure_phylanx()
{
    echo "Removing old phylanx build..."
    rm -rf ${phylanx_build_dir}
    mkdir -p ${phylanx_build_dir}
    cd ${phylanx_build_dir}

    export CC=${mycc}
    export CXX=${mycxx}
    export FC=${myfc}
    export CFLAGS=${mycflags}
    export CXXFLAGS=${mycxxflags}
    export LDFLAGS=${myldflags}

    set -x
    cmake \
    -DCMAKE_BUILD_TYPE=${buildtype} \
    -DEigen3_DIR=${eigen_build_dir}/share/eigen3/cmake \
    -Dpybind11_DIR=${pybind_build_dir}/share/cmake/pybind11 \
    -DHPX_DIR=${HPX_ROOT}/lib/cmake/HPX \
    -DPHYLANX_WITH_PSEUDO_DEPENDENCIES=On \
    -DPHYLANX_WITH_MALLOC=${malloc} \
    -DCMAKE_INSTALL_PREFIX=${phylanx_install_dir} \
    -DPYTHON_EXECUTABLE:FILEPATH=${pythonpath} \
    ${phylanx_src_dir}
}

build_phylanx()
{
    cd ${phylanx_build_dir}
    make ${makej}
}

test_phylanx()
{
    cd ${phylanx_build_dir}
    make tests
    make test
}

if [ ${step} == "all" ] || [ ${step} == "configure" ] ; then
    configure_phylanx
fi
if [ ${step} == "all" ] || [ ${step} == "compile" ] ; then
    build_phylanx
fi
if [ ${step} == "all" ] || [ ${step} == "test" ] ; then
    test_phylanx
fi
