#!/bin/bash -e
set -x

if [ -z ${basedir+x} ] ; then
    echo "basedir is not set. Please source sourceme.sh";
    kill -INT $$
fi

# Parse the arguments
args=$(getopt -l "searchpath:" -o "cdht" -- "$@")
clean=0

eval set -- "${args}"

while [ $# -ge 1 ]; do
    case "$1" in
        --)
            # No more options left.
            break
            ;;
        -c)
            clean=1
            echo "doing clean configure"
            ;;
        -h)
            echo "$0 [-c]"
            echo "-c : Clean"
            kill -INT $$
            ;;
        esac
    shift
done

# just in case - it gets set later
builddir=${basedir}/${myarch}-build/hpx-$buildtype

get_source()
{
    cd ${basedir}/src
    if [ ! -d hpx ] ; then
        git clone git@github.com:STEllAR-GROUP/hpx.git
    fi
    cd hpx
    git checkout master
    git pull
    cd ${basedir}
}

configure_it()
{
    rm -rf ${builddir}
    mkdir -p ${builddir}
    cd ${builddir}

    if [ ${malloc} == "jemalloc" ] ; then
        alloc_opts="-DJEMALLOC_ROOT=${malloc_path} -DHPX_WITH_MALLOC=jemalloc"
    else
        alloc_opts="-DTCMALLOC_ROOT=${malloc_path} -DHPX_WITH_MALLOC=tcmalloc"
    fi
    export CC=${mycc}
    export CXX=${mycxx}
    export FC=${myfc}
    cmake \
    -DCMAKE_BUILD_TYPE=${buildtype} \
    -DBOOST_ROOT=${boost_path} \
    ${alloc_opts} \
    -DHWLOC_ROOT=${hwloc_path} \
    -DCMAKE_INSTALL_PREFIX=. \
    -DHPX_WITH_TOOLS=ON \
    -DHPX_WITH_THREAD_IDLE_RATES=ON \
    -DHPX_WITH_PARCELPORT_MPI=ON \
    -DHPX_WITH_PARCEL_COALESCING=OFF \
    ${apex_opts} \
    ${basedir}/src/hpx
    cd ${basedir}
#-DHPX_WITH_DATAPAR_VC=ON \
#-DVc_ROOT=${basedir}/${myarch}-build/Vc \
#-DHPX_WITH_THREAD_STACK_MMAP=ON \
#-DHPX_WITH_THREAD_MANAGER_IDLE_BACKOFF=OFF \
#-DHPX_WITH_THREAD_BACKTRACE_ON_SUSPENSION=OFF \
#-DHPX_WITH_THREAD_TARGET_ADDRESS=OFF \
#-DHPX_WITH_THREAD_QUEUE_WAITTIME=OFF \
#-DHPX_WITH_THREAD_CUMULATIVE_COUNTS=OFF \
#-DHPX_WITH_THREAD_STEALING_COUNTS=OFF \
#-DHPX_WITH_THREAD_LOCAL_STORAGE=OFF \
#-DHPX_WITH_SCHEDULER_LOCAL_STORAGE=OFF \
#-DHPX_WITH_THREAD_GUARD_PAGE=OFF \
#-DHPX_WITH_PARCELPORT_MPI=ON \
#-DHPX_WITH_PARCELPORT_LIBFABRIC=OFF \
#-DHPX_WITH_PARCEL_COALESCING=ON \
#-DHPX_WITH_PARCELPORT_MPI_MULTITHREADED=ON \
#-DHPX_WITH_DATAPAR_VC_NO_LIBRARY=ON \
#-DCMAKE_SKIP_INSTALL_RPATH=ON \
}

build_it()
{
    cd ${builddir}
    #make -j20 core tools.inspect
    make -j20 core examples
    cd ${basedir}
}

if [ ${clean} -eq 1 ] ; then
    get_source
fi

for t in Release RelWithDebInfo Debug ; do
    buildtype=${t}
    malloc=jemalloc
    builddir=${basedir}/${myarch}-build/hpx-${malloc}-${buildtype}-apex
    if [ ${clean} -eq 1 ] ; then
        apex_opts="-DHPX_WITH_APEX=TRUE -DAPEX_WITH_ACTIVEHARMONY=TRUE -DACTIVEHARMONY_ROOT=${activeharmony_path} -DAPEX_WITH_OTF2=TRUE -DOTF2_ROOT=${otf2_path} -DAPEX_WITH_PAPI=TRUE -DPAPI_ROOT=${papi_path} -DHPX_WITH_APEX_NO_UPDATE=TRUE "
        configure_it
    fi
    build_it
    # builddir=${basedir}/${myarch}-build/hpx-${malloc}-${buildtype}
    # if [ ${clean} -eq 1 ] ; then
    #     apex_opts="-DHPX_WITH_APEX=FALSE"
    #     configure_it
    # fi
    # build_it
done
cd ${basedir}
