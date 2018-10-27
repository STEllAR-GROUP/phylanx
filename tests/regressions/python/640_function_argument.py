#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def test_assign(x):
    x = 1
    return x


assert(test_assign.__src__ ==
        'define$11$0(test_assign$11$0, x$11$16, ' + \       # noqa: W504
        'block(store$12$4(x$12$4, 1), x$13$11))')
