# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# This image is used in Phylanx's CI process to copy the files of
# the built and installed Phylanx setup to an image

FROM stellargroup/phylanx_base:prerequisites

ADD . /phylanx
RUN cmake --build /phylanx/build -- -j2 install && rm -rf /phylanx/build
RUN ldconfig
WORKDIR /
CMD /bin/bash
