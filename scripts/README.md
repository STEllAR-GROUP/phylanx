#Buildbot scripts for Phylanx project

The scripts in this directory are used for launching regular, automated builds
of the Phylanx project.  All dependencies are included, such as HPX, pybind11,
eigen3.

The top level script is buildbot.sh. The other scripts:
* build-hpx.sh: for building HPX
* buildbot_common.sh: for setting common build paths
* buildbot_eigen3.sh: for "building" the eigen3 headers
* buildbot_phylanx.sh: for building phylanx
* buildbot_pybind.sh: for "building" the pybind11 headers
* launch-build-talapas.sh: for launching the HPX build on a compute node
* talapas-gcc.sh: for loading modules, etc. on talapas cluster at UO
