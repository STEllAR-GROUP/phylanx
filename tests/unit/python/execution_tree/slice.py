#  Copyright (c) 2017-2018 Hartmut Kaiser
#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
from phylanx import Phylanx


def hstack():
    pass


@Phylanx
def test1():
    a = hstack((0, 1, 2, 3, 4, 5))
    b = hstack((True, True, False, False, True, False))
    a[b] = 9
    return a


assert ([9, 9, 2, 3, 9, 5] == test1()).all()


@Phylanx
def test2():
    a = hstack((0, 1, 2, 3, 4, 5))
    b = hstack((1, 3, 5))
    a[b] = 7
    return a


assert([0, 7, 2, 7, 4, 7] == test2()).all()
