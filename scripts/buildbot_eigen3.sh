#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

# load common settings
if [ -z ${eigen_src_dir} ] ; then
    . ${scriptdir}/buildbot_common.sh
fi

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

build_eigen3