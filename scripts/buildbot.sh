#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

myhost=`hostname`
# load common settings
if [ ${myhost} == "ktau" ] ; then
    . ${scriptdir}/${myhost}-gcc.sh
elif [ ${myhost} == "delphi" ] ; then
    . ${scriptdir}/${myhost}-gcc.sh
elif [ ${myhost} == "grover" ] ; then
    . ${scriptdir}/${myhost}-intel.sh
elif [ ${myhost} == "centaur" ] ; then
    . ${scriptdir}/${myhost}-clang.sh
fi
. ${scriptdir}/buildbot_common.sh

args=$(getopt -l "searchpath:" -o "c:t:s:h" -- "$@")
component_in="all"
buildtype_in="all"
step_in="all"

eval set -- "${args}"

while [ $# -ge 1 ]; do
    case "$1" in
        --)
            # No more options left.
            break
            ;;
        -c|--component)
            component_in="$2"
            ;;
        -t|--type)
            buildtype_in="$2"
            ;;
        -s|--step)
            step_in="$2"
            ;;
        -h)
            echo "$0 -c,--component [all|hpx|eigen3|pybind|phylanx] -t,--type [all|Release|RelWithDebInfo|Debug] -s,--step [all|configure|compile|test|install] -h,--help"
            exit 0
            ;;
    esac
    shift
done

echo "component = ${component_in}"
echo "buildtype = ${buildtype_in}"
echo "step = ${step_in}"

build_type_component_step()
{
    # re-load common settings
    . ${scriptdir}/buildbot_common.sh

    # Build the project
    ${scriptdir}/buildbot_${component}.sh ${buildtype} ${step}
}

loop_steps() {
    if [ ${step_in} == "all" ] ; then
        for step in "configure" "compile" "test" ; do
            export step
            build_type_component_step ${buildtype} ${component} ${step}
        done
    else
        export step=${step_in}
        build_type_component_step ${buildtype} ${component} ${step}
    fi
}

loop_components() {
    if [ ${component_in} == "all" ] ; then
        for component in "hpx" "eigen3" "pybind" "phylanx" ; do
            export component
            loop_steps
        done
    else
        export component=${component_in}
        loop_steps
    fi
}

loop_buildtypes() {
    if [ ${buildtype_in} == "all" ] ; then
        for buildtype in "Debug" "RelWithDebInfo" "Release" ; do
            export buildtype
            loop_components
        done
    else
        export buildtype=${buildtype_in}
        loop_components
    fi
}

# make the builddir if it doesn't exist
if [ ! -d ${buildprefix} ] ; then
    mkdir -p ${buildprefix}
fi

# make the sourcedir if it doesn't exist
if [ ! -d ${sourcedir} ] ; then
    mkdir -p ${sourcedir}
fi

# if necessary, build boost
if [ ${myhost} == "delphi" ] || [ ${myhost} == "centaur" ] ; then
    if [ ! -d ${boost_path} ] ; then
        ${scriptdir}/buildbot_boost.sh
    fi
fi
# do all the requested combinations
loop_buildtypes
