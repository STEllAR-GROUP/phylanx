export PATH=/usr/local/packages/cmake/3.9.2-ppc64le/bin:$PATH

# special flags for some library builds
# export mycflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
# export mycxxflags="-fPIC -mavx512f -mavx512cd -mavx512bw -mavx512dq -mavx512vl -mavx512ifma -mavx512vbmi"
export mycflags="-fPIC"
export mycxxflags="-fPIC -fpermissive"
export myldflags="-fPIC"
export mycc=xlc
export mycxx=xlC

host=centaur
arch=`arch`
uname=`uname`

export basedir=${HOME}/src/phylanx
export myarch=${host}-${arch}-${uname}-xlc
export buildtype=Release
export malloc=jemalloc
export contrib=${basedir}/build-${myarch}/contrib
export malloc_path=/usr/local/packages/jemalloc/5.0.1-${arch}-${mycc}
export activeharmony_path=/usr/local/packages/activeharmony/4.6.0-${arch}-${mycc}
export otf2_path=/usr/local/packages/otf2/2.0-ppc64le
export papi_path=/usr/local/packages/papi/5.5.0-ppc64le
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
