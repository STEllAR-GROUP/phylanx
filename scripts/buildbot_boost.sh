#!/bin/bash -e
set -x

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

# load common settings
if [ -z ${hpx_src_dir} ] ; then
    . ${scriptdir}/buildbot_common.sh
fi

mkdir -p /dev/shm/src
cd /dev/shm/src
if [ ! -d boost_1_65_0 ] ; then
    if [ ! -f boost_1_65_0.tar.bz2 ] ; then
        wget https://sourceforge.net/projects/boost/files/boost/1.65.0/boost_1_65_0.tar.bz2
    fi
    tar -xjf boost_1_65_0.tar.bz2
fi


cd /dev/shm/src/boost_1_65_0
./bootstrap.sh --prefix=${boost_build_dir}
if [ ${mycc} == "icc" ] ; then
    ./b2 ${makej} install toolset=intel address-model=64
else
    ./b2 ${makej} install cxxflags="-march=native" linkflags="-march=native"
fi
