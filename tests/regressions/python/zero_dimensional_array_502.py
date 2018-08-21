# Copyright (c) 2018 Weile Wei
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


@Phylanx
def zero_dim_array(input_para):
    return input_para


a = np.array(123456)
assert ((a == zero_dim_array(a)).all())
