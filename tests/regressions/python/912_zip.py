#  Copyright (c) 2019 R. Tohid
#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def phy_fx(start, size):
    ind = []
    for i, j in zip(start, size):
        ind = append(ind, [i, i + j])  # noqa: F821
    return ind


def np_fx(start, size):
    ind = []
    for i, j in zip(start, size):
        ind.append([i, i + j])
    return ind


assert phy_fx([0, 1, 1], [1, 1, 3]) == np_fx([0, 1, 1], [1, 1, 3])


@Phylanx
def phy_list_comp(start, size):
    ind = [[i, i + j] for i, j in zip(start, size)]
    return ind


def np_list_comp(start, size):
    ind = [[i, i + j] for i, j in zip(start, size)]
    return ind


assert phy_list_comp([0, 1, 1], [1, 1, 3]) == np_list_comp([0, 1, 1],
                                                           [1, 1, 3])
