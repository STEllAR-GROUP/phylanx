#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.ast import *
import numpy as np


@Phylanx
def foo1(x):
    local_x = x
    return local_x


c = 1
d = 2

assert (foo1(c) == 1)
assert (foo1(d) == 2)
