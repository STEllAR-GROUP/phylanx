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

configure_eigen3()
{
    if [ ! -d ${eigen_src_dir} ] ; then
        cd ${top}/src
        if [ ! -f 3.3.4.tar.gz ] ; then
            wget http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
        fi
        tar -xzf 3.3.4.tar.gz
    fi
    echo "Removing old eigen3 build..."
    rm -rf ${eigen_build_dir}
    mkdir -p ${eigen_build_dir}
    cd ${eigen_build_dir}
    cmake \
    -DCMAKE_BUILD_TYPE=${buildtype} \
    -DCMAKE_INSTALL_PREFIX=. \
    ${eigen_src_dir}
}

build_eigen3()
{
    cd ${eigen_build_dir}
    make ${makej}
    make install
}

if [ ${step} == "all" ] || [ ${step} == "configure" ] ; then
    configure_eigen3
fi
if [ ${step} == "all" ] || [ ${step} == "compile" ] ; then
    build_eigen3
fi
