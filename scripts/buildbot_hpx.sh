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

if [ -z ${basedir+x} ] ; then
    echo "basedir is not set. Please source the appropriate settings in the ${scriptdir} directory.";
    kill -INT $$
fi

# get the common settings
. ${scriptdir}/buildbot_common.sh

if [ -z ${buildtype} ] ; then
    echo "buildtype not set."
    kill -INT $$
fi

if [ -z ${HPX_ROOT} ] ; then
    echo "HPX_ROOT not set."
    kill -INT $$
fi

if [ ! -d $(dirname ${HPX_ROOT}) ] ; then
    echo "$(dirname ${HPX_ROOT}) does not exist."
    kill -INT $$
fi

get_source()
{
    if [ ! -d ${hpx_src_dir} ] ; then
        cd $(dirname ${hpx_src_dir})
        git clone https://github.com/STEllAR-GROUP/hpx.git
    fi
    cd ${hpx_src_dir}
    git reset --hard
    git checkout master
    git pull
    # git checkout a7da13ca4a48b08db379534d8f2cbfd7265ca6fa
    patch -N CMakeLists.txt ${scriptdir}/hpx.patch
}

configure_it()
{
    rm -rf ${HPX_ROOT}
    mkdir -p ${HPX_ROOT}
    cd ${HPX_ROOT}

    if [ ${malloc} == "jemalloc" ] ; then
        alloc_opts="-DJEMALLOC_ROOT=${malloc_path} -DHPX_WITH_MALLOC=jemalloc"
    elif [ ${malloc} == "tcmalloc" ] ; then
        alloc_opts="-DTCMALLOC_ROOT=${malloc_path} -DHPX_WITH_MALLOC=tcmalloc"
    else
        alloc_opts="-DHPX_WITH_MALLOC=system"
    fi
    apex_opts="-DHPX_WITH_APEX=TRUE -DAPEX_WITH_ACTIVEHARMONY=TRUE -DACTIVEHARMONY_ROOT=${activeharmony_path} -DAPEX_WITH_OTF2=TRUE -DOTF2_ROOT=${otf2_path} -DAPEX_WITH_PAPI=TRUE -DPAPI_ROOT=${papi_path} -DHPX_WITH_APEX_NO_UPDATE=TRUE "
    export CC=${mycc}
    export CXX=${mycxx}
    export FC=${myfc}
    export CFLAGS=${mycflags}
    export CXXFLAGS=${mycxxflags}
    export LDFLAGS=${myldflags}

    if [[ ${myarch} =~ .*knl.* ]] ; then
        cmake_extras="-DHPX_WITH_MAX_CPU_COUNT=512"
    fi

    set -x
    cmake \
    -DCMAKE_BUILD_TYPE=${buildtype} \
    -DBOOST_ROOT=${boost_path} \
    ${alloc_opts} \
    -DHWLOC_ROOT=${hwloc_path} \
    -DCMAKE_INSTALL_PREFIX=. \
    -DHPX_WITH_TOOLS=OFF \
    -DHPX_WITH_THREAD_IDLE_RATES=ON \
    -DHPX_WITH_PARCELPORT_MPI=OFF \
    -DHPX_WITH_PARCEL_COALESCING=OFF \
    ${apex_opts} \
    ${cmake_extras} \
    ${hpx_src_dir}
}

build_it()
{
    cd ${HPX_ROOT}
    make ${makej} core
}

if [ ${step} == "all" ] || [ ${step} == "configure" ] ; then
    get_source
    configure_it
fi
if [ ${step} == "all" ] || [ ${step} == "compile" ] ; then
    build_it
fi
