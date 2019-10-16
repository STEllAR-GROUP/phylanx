#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #966: Named arguments don't work


from phylanx import Phylanx
import numpy as np


# make flake happy
def eye(N, M, k, dtype):
    pass


@Phylanx
def i(N, M=None, k=0, dtype=None):
    return eye(N, M=M, k=k, dtype=dtype)


assert((i(3, k=2) == np.eye(3, k=2)).all())
