#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession
import numpy as np

PhylanxSession(1)


@Phylanx
def foo1():
    for i in range(3):
        print("hello")


foo1()
