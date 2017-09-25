module load gcc/7.1
module load cmake
module load python/3.3.4
module list

# special flags for some library builds
# export mycflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
# export mycxxflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
export mycflags="-fPIC -march=native"
#export mycxxflags="-fPIC -march=native -fpermissive"
export mycxxflags="-fPIC -march=native"
export myldflags="-fPIC -march=native -latomic"
#export mycflags="-fPIC"
#export mycxxflags="-fPIC -fpermissive"
#export myldflags="-fPIC"
export mycc=gcc
export mycxx=g++
export myfc=gfortran

host=delphi
arch=`arch`
uname=`uname`

export basedir=${HOME}/src/phylanx
export myarch=${host}-${arch}-${uname}-gcc
export hpxtoolchain=${basedir}/src/hpx/cmake/toolchains/Cray.cmake
export buildtype=Release
export malloc=jemalloc
export contrib=${basedir}/build-${myarch}/contrib
export malloc_path=/usr/local/packages/jemalloc/5.0.1-gcc
export activeharmony_path=/usr/local/packages/activeharmony/4.6.0-knl-gcc-6.1
export otf2_path=/usr/local/packages/otf2-2.1
export papi_path=/usr/local/packages/papi/papi-knl/5.5.0/
export boost_path=${basedir}/build-${myarch}/boost-1.65.0
export BOOST_DIR=${boost_path}
export BOOST_ROOT=${boost_path}

echo ""
echo "NB: "
echo "basedir is set to ${basedir}."
echo "  All paths are relative to that base."
echo "myarch is set to ${myarch}."
echo "  Build output will be in ${basedir}/build-${myarch}."
echo ""
