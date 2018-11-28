#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession
import numpy as np

PhylanxSession(1)


@Phylanx
def foo():
    local_a = np.array((2, 1))
    local_a[0] += 55
    return local_a


assert (np.array((57, 1)) == foo()).any()
