#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #904: Cannot pass function as an argument

import numpy as np
from phylanx import Phylanx, PhylanxSession, execution_tree


@Phylanx
def fn():
    return lambda a, b : 2 * a - b


@Phylanx
def foldl_eager(fn, elems, initializer):
    return fold_left(fn, initializer, elems)


assert foldl_eager(fn, [1, 2, 3], 3) == 13
