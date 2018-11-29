#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx
import numpy as np


# 0d tests
@Phylanx
def expand_dims_0d_default_d():
    return np.expand_dims(42.0)


result = expand_dims_0d_default_d()
assert(result.dtype.name == 'float64')
assert((result == np.array([42.0])).all())


@Phylanx
def expand_dims_0d_d():
    return np.expand_dims(42, dtype=float)


result = expand_dims_0d_default_d()
assert(result.dtype.name == 'float64')
assert((result == np.array([42.0])).all())


@Phylanx
def expand_dims_0d_default_i():
    return np.expand_dims(42)


result = expand_dims_0d_default_i()
assert(result.dtype.name == 'int64')
assert((result == np.array([42])).all())


@Phylanx
def expand_dims_0d_i():
    return np.expand_dims(42.0, dtype=int)


result = expand_dims_0d_i()
assert(result.dtype.name == 'int64')
assert((result == np.array([42])).all())


# 1d tests
@Phylanx
def expand_dims_1d_default_d():
    return np.expand_dims(np.array([42.0]))


result = expand_dims_1d_default_d()
assert(result.dtype.name == 'float64')
assert((result == np.array([[42.0]])).all())


@Phylanx
def expand_dims_1d_d():
    return np.expand_dims(np.array([42]), dtype=float)


result = expand_dims_1d_default_d()
assert(result.dtype.name == 'float64')
assert((result == np.array([[42.0]])).all())


@Phylanx
def expand_dims_1d_default_i():
    return np.expand_dims(np.array([42]))


result = expand_dims_1d_default_i()
assert(result.dtype.name == 'int64')
assert((result == np.array([[42]])).all())


@Phylanx
def expand_dims_1d_i():
    return np.expand_dims(np.array([42.0]), dtype=int)


result = expand_dims_1d_i()
assert(result.dtype.name == 'int64')
assert((result == np.array([[42]])).all())
