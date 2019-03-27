#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx import Phylanx
import numpy as np


@Phylanx
def test_make_vector_one():
    return hstack((42))  # noqa: F821


@Phylanx
def test_make_vector_literals():
    return hstack((1, 2, 3, 4))  # noqa: F821


@Phylanx
def test_make_vector():
    a = 1
    b = 2
    c = 3
    return hstack((a, b, c))  # noqa: F821


@Phylanx
def test_make_vector2():
    a = 1
    b = 2
    c = 3
    return hstack((a, hstack((a, b, c)), c))  # noqa: F821


a = test_make_vector_one()
assert (isinstance(a, np.ndarray))
assert (a == np.array(42)).all()

a = test_make_vector_literals()
assert (isinstance(a, np.ndarray))
assert (a == np.array([1, 2, 3, 4])).all()

a = test_make_vector()
assert (isinstance(a, np.ndarray))
assert (a == np.array([1, 2, 3])).all()

a = test_make_vector2()
assert (isinstance(a, np.ndarray))
assert (a == np.array([1, 1, 2, 3, 3])).all()
