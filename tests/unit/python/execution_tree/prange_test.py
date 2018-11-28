#  Copyright (c) 2018 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx import Phylanx, PhylanxSession
from phylanx.util import prange

import numpy as np

PhylanxSession(1)

# @Phylanx(debug=True)
# def test_prange():
#     for x in prange(0,10):
#         print (x)

# test_prange()


@Phylanx
def test_prange_list():

    # TODO: originaly just a list which does not work.
    # replacing the list with numpy array resolves the issue
    arr = np.array([0, 0, 0, 0, 0])
    # arr = [0, 0, 0, 0, 0]
    arrlen = 5

    # TODO: add 'len' support
    # arrlen = len(arr)
    for i in prange(0, arrlen):
        arr[i] = i


test_prange_list()
