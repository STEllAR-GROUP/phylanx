# Copyright (c) 2011 Bryce Lelbach
#               2015 Martin Stumpf
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

set(PHYLANX_GITCOMMIT_LOADED TRUE)

# if no git commit is set, try to get it from the source directory
if(NOT PHYLANX_WITH_GIT_COMMIT OR "${PHYLANX_WITH_GIT_COMMIT}" STREQUAL "None")

  find_package(Git)

  if(GIT_FOUND)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" "log" "--pretty=%H" "-1" "${PROJECT_SOURCE_DIR}"
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      OUTPUT_VARIABLE PHYLANX_WITH_GIT_COMMIT ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  endif()

endif()

if(NOT PHYLANX_WITH_GIT_COMMIT OR "${PHYLANX_WITH_GIT_COMMIT}" STREQUAL "None")
  phylanx_warn("GIT commit not found (set to 'unknown').")
  set(PHYLANX_WITH_GIT_COMMIT "unknown")
else()
  phylanx_info("GIT commit is ${PHYLANX_WITH_GIT_COMMIT}.")
endif()

