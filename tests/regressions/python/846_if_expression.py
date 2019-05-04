#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def if_expr(n):
    return True if n else False


assert if_expr(True) is np.bool_(True)
assert if_expr(False) is np.bool_(False)
