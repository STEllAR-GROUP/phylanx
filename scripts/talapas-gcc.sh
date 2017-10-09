#module load openmpi/gcc/64/1.10.1
module load mpich/ge/gcc/64/3.2rc2
module load cmake
module load python3
module list

# special flags for some library builds
# export mycflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
# export mycxxflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
export mycflags="-fPIC -march=native"
export mycxxflags="-fPIC -march=native"
export myldflags="-fPIC -march=native"
export mycc=gcc
export mycxx=g++
export myfc=gfortran

host=talapas
arch=`arch`
uname=`uname`

export basedir=${HOME}/src/phylanx
export myarch=${host}-${arch}-${uname}-gcc
export hpxtoolchain=${basedir}/src/hpx/cmake/toolchains/Cray.cmake
export buildtype=Release
export malloc=tcmalloc
export contrib=${basedir}/build-${myarch}/contrib
export malloc_path=${contrib}
export hwloc_path=${contrib}
export activeharmony_path=${contrib}
export otf2_path=${contrib}
export papi_path=${contrib}
export boost_path=${contrib}

echo ""
echo "NB: "
echo "basedir is set to ${basedir}."
echo "  All paths are relative to that base."
echo "myarch is set to ${myarch}."
echo "  Build output will be in ${basedir}/build-${myarch}."
echo ""
