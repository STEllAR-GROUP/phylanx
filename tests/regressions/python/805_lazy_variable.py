#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


PhylanxSession.init()


def variable(value, dtype=None):
    return execution_tree.variable(np.array(value, dtype), dtype)


@Phylanx
def ones_like_eager(x):
    return np.ones_like(x)


def ones_like(x):
    return ones_like_eager.lazy(x)


def eval(func):
    return func.eval()


x = variable(np.array([1, 2, 3]))
print(eval(ones_like(x)))
