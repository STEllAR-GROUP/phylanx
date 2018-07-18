#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.ast import *
import numpy as np


@Phylanx()
def test_empty_list(a, b):
    c = []
    return c


a = [1, 2, 3]
b = [4, 5]
c = test_empty_list(a, b)
assert (c == [])
