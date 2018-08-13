#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx
def f1(x):
    local_x = x
    return local_x


c = 1
d = 2

assert (f1(c) == 1)
assert (f1(d) == 2)


@Phylanx
def f2(x):
    local_x = x
    local_x = local_x + 1
    return local_x


assert (f2(1) == 2)
assert (f2(1) == 2)
