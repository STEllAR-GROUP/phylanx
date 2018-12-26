# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# This image is used in Phylanx's CI process to copy the files of
# the built and installed Phylanx setup to an image

FROM stellargroup/phylanx_base:prerequisites

# docker build needs to be run in /usr/local
ADD . /usr/local
RUN ldconfig
WORKDIR /
CMD /bin/bash
