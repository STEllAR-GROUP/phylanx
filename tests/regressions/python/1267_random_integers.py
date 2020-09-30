#  Copyright (c) 2020 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1267: np.random.random_sample is not available

from phylanx import Phylanx
import numpy as np


@Phylanx
def test_random_integers1(low):
    return np.random.random_integers(low)


@Phylanx
def test_random_integers2(low, high):
    return np.random.random_integers(low, high)


@Phylanx
def test_random_integers3(low, high, size):
    return np.random.random_integers(low, high, size)


for i in range(10):
    result = test_random_integers1(10)
    assert result >= 1 and result <= 10, result

    result = test_random_integers2(10, 20)
    assert result >= 10 and result <= 20, result

    result = test_random_integers3(10, 20, 10)
    assert len(result) == 10, result
    for v in result:
        assert v >= 10 and v <= 20, v

    result = test_random_integers3(10, 20, (10,))
    assert len(result) == 10, result
    for v in result:
        assert v >= 10 and v <= 20, v
