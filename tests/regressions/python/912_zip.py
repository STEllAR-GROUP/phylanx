#  Copyright (c) 2019 R. Tohid
#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def phy_fx(start, size):
    indices = []
    for i, j in zip(start, size):
        indices = append(indices, [i, i + j])  # noqa: F821
    return indices


def np_fx(start, size):
    indices = []
    for i, j in zip(start, size):
        indices.append([i, i + j])
    return indices


assert phy_fx([0, 1, 1], [1, 1, 3]) == np_fx([0, 1, 1], [1, 1, 3])
