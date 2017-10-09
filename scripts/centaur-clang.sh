gcc_dir=/usr/local/packages/gcc/6.3.0-ppc64le
llvm_dir=/usr/local/packages/llvm/5.0.0-ppc64le
cmake_dir=/usr/local/packages/cmake/3.9.2-ppc64le
PATH=${llvm_dir}/bin:${gcc_dir}/bin:${cmake_dir}/bin:$PATH
LD_LIBRARY_PATH=${llvm_dir}/lib:${gcc_dir}/lib:${gcc_dir}/lib64:$LD_LIBRARY_PATH

# module load python/3.3.4

# special flags for some library builds
export mycflags="-fPIC"
export mycxxflags="-fPIC"
export myldflags="-fPIC"
export mycc=clang
export mycxx=clang++

export host=centaur
arch=`arch`
uname=`uname`

if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi
export basedir=$( dirname "${scriptdir}" )
export myarch=${host}-${arch}-${uname}-${mycc}
export buildtype=Release
export malloc=tcmalloc
export contrib=${basedir}/build-${myarch}/contrib
export malloc_path=/usr/local/packages/gperftools/2.5
export activeharmony_path=/usr/local/packages/activeharmony/4.6.0-${arch}
export otf2_path=/usr/local/packages/otf2/2.0-ppc64le
export papi_path=/usr/local/packages/papi/5.5.0-ppc64le
export boost_path=${basedir}/buildbot/build-${myarch}/boost-1.65.0
export BOOST_DIR=${boost_path}
export BOOST_ROOT=${boost_path}
export cmake_extras=" -DHWLOC_ROOT=/usr/local/packages/hwloc/1.8 "

echo ""
echo "NB: "
echo "basedir is set to ${basedir}."
echo "  All paths are relative to that base."
echo "myarch is set to ${myarch}."
echo "  Build output will be in ${basedir}/build-${myarch}."
echo ""
