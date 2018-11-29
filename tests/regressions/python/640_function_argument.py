#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# flake8: noqa

import numpy as np
from phylanx import Phylanx


@Phylanx
def test_assign(x):
    x = 1
    return x


assert(test_assign.__src__ ==
        'define$13$0(test_assign$13$0, x$13$16, ' + \
        'block(store$14$4(x$14$4, 1), x$15$11))')
