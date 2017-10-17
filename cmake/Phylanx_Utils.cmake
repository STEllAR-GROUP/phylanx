# Copyright (c) 2011 Bryce Lelbach
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

include(CMakeParseArguments)
include(Phylanx_Include)

phylanx_include(
  Message
  Option
  AddCompileFlag
  AddExecutable
  AddLibraryHeaders
  AddLibrarySources
  AddLinkFlag
  AddPseudoDependencies
  AddPseudoTarget
  AddSourceGroup
  AddTest
  AddPythonTest
  CreateSymbolicLink
  ExportTargets
  ForceOutOfTreeBuild
  GetPythonExtensionLocation
  IsTarget
  SetupCompilerFlags
  SetOutputPaths
  SetupTarget
  ShortenPseudoTarget
)

# TODO (#blazemig): Remove "SetupEigen3\n" after migration
phylanx_include(
  SetupHPX
  SetupEigen3
  SetupBlaze
  SetupPybind11
)
