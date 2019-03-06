#  Copyright (c) 2019 Bita Hasheminezhad
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


def random(shape, t):   # make flake happy
    pass


@Phylanx
def random_uniform(shape, minval, maxval, dtype=None, seed=None):
    return random(shape, list("uniform", minval, maxval))


result = random_uniform(10, 0.0, 1.0)
assert result.shape == (10,)


result = random_uniform(None, 0.0, 1.0)
assert isinstance(result, float)
