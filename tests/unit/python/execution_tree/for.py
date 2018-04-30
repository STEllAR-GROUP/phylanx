#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *


@Phylanx("PhySL")
def sumn():
    n = 0
    sum = 0
    for n in range(4):
        sum += n
    return sum


assert sumn() == 6


@Phylanx("PhySL")
def sumn2():
    n = 0
    sum = 0
    for n in range(1, 4):
        sum += n
    return sum


assert sumn2() == 6


@Phylanx("PhySL")
def sumn3():
    n = 0
    sum = 0
    c = 0
    for n in range(3, 0, -1):
        sum += n
        c += 1
    return sum + c


assert sumn3() == 9

