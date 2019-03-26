#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #874: Shape for 0D arrays

import numpy as np
from phylanx import execution_tree, PhylanxSession


PhylanxSession.init(1)


def variable(value):
    return execution_tree.var(np.float64(value))


val = variable(42.0).eval()
assert isinstance(val, np.float64)
assert val.shape == ()
