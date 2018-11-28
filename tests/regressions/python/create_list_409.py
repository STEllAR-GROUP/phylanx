#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #409: Cannot create lists of arbitrary objects

import phylanx
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def f():
    return [1, [1]]


assert f.__src__ == \
    "define$15$0(f$15$0, list$16$11(1, list$16$15(1)))"
