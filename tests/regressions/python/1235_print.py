#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1235: Can't print array

from phylanx import Phylanx
from random import random


@Phylanx
def pr(a):
    print(a)


im = 31
jm = 284
arr = [[random() for i in range(im)] for j in range(jm)]

pr(arr)
