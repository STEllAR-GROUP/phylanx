#Buildbot scripts for Phylanx project

The scripts in this directory are used for launching regular, automated builds
of the Phylanx project.  All dependencies are included, such as HPX, pybind11,
eigen3.

The top level script is buildbot.sh. The other relevant scripts:

* buildbot\_boost.sh: for building boost 1.65
* buildbot\_hpx.sh: for building HPX master
* buildbot\_common.sh: for setting common build paths
* buildbot\_eigen3.sh: for "building" the eigen3 headers
* buildbot\_pybind.sh: for "building" the pybind11 headers
* buildbot\_phylanx.sh: for building phylanx
* launch-build-talapas.sh: for launching the HPX build on a compute node
* delphi-gcc.sh: for loading modules, etc. on delphi x86\_64 server at UO
* grover-intel.sh: for loading modules, etc. on grover KNL server at UO
* ktau-gcc.sh: for loading modules, etc. on buildbot server at UO

Working machine scripts:

* Delphi (x86\_64-Linux RedHat, with 18 cores, hyperthreading, gcc 7.1, boost 1.65)
* ktau (x86\_64-Linux Ubuntu, with 8 cores, gcc 7.1, boost 1.65)
* grover (KNL-Linux with 68 cores, 4x threads per core, Intel 18, boost 1.65)
* centaur (IBM Power8-Linux with 40 cores, 4x threads per core, Clang 5.0, boost 1.65)

