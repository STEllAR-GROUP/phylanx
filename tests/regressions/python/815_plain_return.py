#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx


@Phylanx
def test_plain():
    return


print(test_plain())
assert(test_plain() == None) # noqa


@Phylanx
def test_none():
    return None


assert(test_None() == None) # noqa


@Phylanx
def test_scalar():
    a = 2
    return a


assert(test_scalar() == 2)


@Phylanx
def test_container():
    return (1, 2, 3)


assert(test_container() == (1, 2, 3))
