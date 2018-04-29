# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

if(PHYLANX_WITH_VIM_YCM)
  SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(build_dir_file ${CMAKE_BINARY_DIR}/.ycm_extra_conf.py)
  set(source_dir_file ${CMAKE_SOURCE_DIR}/.ycm_extra_conf.py)
  configure_file(${CMAKE_SOURCE_DIR}/tools/vim/.ycm_extra_conf.py
               ${build_dir_file} @ONLY)
  add_custom_target(
    configure_ycm
    COMMAND ${CMAKE_COMMAND} -E copy ${build_dir_file} ${source_dir_file}
    COMMENT "Copying YCM config file to source directory"
    VERBATIM
  )
  phylanx_info("VIM YouCompleteMe: run 'make configure_ycm' to copy config file to source directory and enable support in YCM. To enable automatic loading of configure file, add to your .vimrc option: \"let g:ycm_extra_conf_globlist = ['${CMAKE_SOURCE_DIR}/*']\"")
endif()

