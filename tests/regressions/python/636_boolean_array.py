#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession
import numpy as np

PhylanxSession(1)


@Phylanx
def bool_array():
    a = np.array([True, False])
    return a


assert((bool_array() == np.array([True, False])).all())
