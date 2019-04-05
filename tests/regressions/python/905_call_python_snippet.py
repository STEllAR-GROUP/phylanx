#  Copyright (c) 2019 R. Tohid
#
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #905: Enable calling Python snippet from inside the Phylanx execution tree

import numpy as np
from phylanx import Phylanx


@Phylanx
def foo(func, a, b):
    return func(a, b)


@Phylanx
def phy_bar(x, y):
    return x + y


assert 5 == foo(phy_bar, 2, 3)


def bar(x, y):
    return x + y


assert 5 == foo(bar, 2, 3)

assert 5 == foo(lambda a, b: a + b, 2, 3)
