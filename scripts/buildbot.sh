#!/bin/bash -e

# where is this script?
if [ -z ${scriptdir} ] ; then
    scriptdir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
fi

# load common settings
. ${scriptdir}/buildbot_common.sh

# ${scriptdir}/build-hpx.sh
${scriptdir}/buildbot_eigen3.sh
${scriptdir}/buildbot_pybind.sh
${scriptdir}/buildbot_phylanx.sh