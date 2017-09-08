#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

build_project_type()
{
    project=$1
    export buildtype=$2

    # load common settings
    . ${scriptdir}/buildbot_common.sh

    # Build the project
    ${scriptdir}/buildbot_${project}.sh
    unset buildtype
}

build_project()
{
    project=$1
    build_project_type ${project} Debug 
    build_project_type ${project} RelWithDebInfo
    build_project_type ${project} Release 
}

#build_project hpx
build_project eigen3
build_project pybind
build_project phylanx
