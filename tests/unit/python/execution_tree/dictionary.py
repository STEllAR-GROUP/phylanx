#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def mkdict():
    return {1: 2, 3: 4}


a = mkdict()

assert a[1] == 2
assert a[3] == 4


@Phylanx
def setdict():
    a = mkdict()
    a[1] = 5
    return a


a = setdict()

assert a[1] == 5


@Phylanx
def lookupdict():
    a = mkdict()
    return a[1]


assert lookupdict() == 2


@Phylanx
def newvalue():
    a = mkdict()
    a[2] = 42
    return a[2]


assert newvalue() == 42
