# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #429: Cannot iterate over Python lists

import phylanx
from phylanx.ast import *


@Phylanx
def f1():
    a = 0
    for i in [1]:
        a = 0
    return a


@Phylanx
def f2():
    a = 0
    for i in [1, 2, 3]:
        a = i
    return a


assert f2() == 3
