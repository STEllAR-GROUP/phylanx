# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx

expected = np.array([[1, 2], [3, 4]])


@Phylanx
def foo(m):
    a = np.array([[m, 2], [3, 4]])
    return a


lazy_foo = foo.lazy(1)
evaluated_foo = lazy_foo.eval()
assert (evaluated_foo == expected).all()


@Phylanx
def foo(m):
    a = np.array([[m, 2], [3, 4]])
    return a


evaluated_foo = foo(1)
assert (evaluated_foo == expected).all()


@Phylanx
def eye_eager(size):
    return np.eye(size)


def eye(size):
    return eye_eager.lazy(size)


assert (eye_eager(3) == np.eye(3)).all()
assert (eye(3).eval() == np.eye(3)).all()
