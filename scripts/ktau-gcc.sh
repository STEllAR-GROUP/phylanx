# special flags for some library builds
# export mycflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
# export mycxxflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
#export mycflags="-fPIC -march=native"
#export mycxxflags="-fPIC -march=native -fpermissive"
#export mycxxflags="-fPIC -march=native"
#export myldflags="-fPIC -march=native -latomic"
export mycflags="-fPIC"
export mycxxflags="-fPIC -fpermissive"
export myldflags="-fPIC"
export mycc=gcc
export mycxx=g++
export myfc=gfortran

host=ktau
arch=`arch`
uname=`uname`

export basedir=${HOME}/src/phylanx
export myarch=${host}-${arch}-${uname}-gcc
export hpxtoolchain=${basedir}/src/hpx/cmake/toolchains/Cray.cmake
export buildtype=Release
#export malloc=jemalloc
export malloc=tcmalloc
export hwloc_path=/usr/local/hwloc/1.11.5
export contrib=${basedir}/build-${myarch}/contrib
#export malloc_path=/usr/local/jemalloc/5.0.1
export malloc_path=/usr/local/gperftools/2.5
export activeharmony_path=/usr/local/activeharmony/4.6
export otf2_path=/usr/local/otf2/2.1
export papi_path=/usr/local/papi/5.5.0/
export boost_path=/usr/local/boost/1.65.0-gcc6
export BOOST_DIR=${boost_path}
export BOOST_ROOT=${boost_path}

echo ""
echo "NB: "
echo "basedir is set to ${basedir}."
echo "  All paths are relative to that base."
echo "myarch is set to ${myarch}."
echo "  Build output will be in ${basedir}/build-${myarch}."
echo ""
