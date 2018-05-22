#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #409: Cannot create lists of arbitrary objects

import phylanx
from phylanx.ast import *


@Phylanx()
def f():
    return [1, [1]]


assert f.__src__ == \
    "define$13$0(f$13$0, make_list$14$11(1,make_list$14$15(1)))"
