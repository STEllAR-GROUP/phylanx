#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np


def arange(start, stop=None, step=1, dtype='int32'):
    return np.arange(start, stop, step, dtype)


def eval(x):
    return x


def variable(value, dtype=None, name=None, constraint=None):
    if constraint is not None:
        raise TypeError("Constraint must be None when "
                        "using the NumPy backend.")
    return np.array(value, dtype)


def dtype(x):
    return x.dtype.name
