#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1281: Phylanx Minimum Viable Product

from phylanx import Phylanx
import socket


def hostname():     # silence flake
    pass


@Phylanx
def get_hostname():
    return hostname()


result = get_hostname()
assert result == socket.gethostname(), result
