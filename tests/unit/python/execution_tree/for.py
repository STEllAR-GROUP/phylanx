#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx import Phylanx


@Phylanx
def sumn():
    n = 0
    local_sum = 0
    for n in range(4):
        local_sum += n
    return local_sum


assert sumn() == 6


@Phylanx
def sumn2():
    n = 0
    local_sum = 0
    for n in range(1, 4):
        local_sum += n
    return local_sum


assert sumn2() == 6


@Phylanx
def sumn3():
    n = 0
    local_sum = 0
    c = 0
    for n in range(3, 0, -1):
        local_sum += n
        c += 1
    return local_sum + c


result = sumn3()
assert result == 9, result
