#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 866: The problem with the `dtype` as an argument

import numpy as np
from phylanx import Phylanx


@Phylanx
def fx2(x, d):
    return np.eye(x, dtype=d)


assert (fx2(5, 'float') == np.eye(5, dtype='float')).all()
