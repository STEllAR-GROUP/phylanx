#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.util import *


@phyfun
def test_make_list_empty():
    return make_list()


@phyfun
def test_make_list_one():
    return make_list(42)


@phyfun
def test_make_list_literals():
    return make_list(1, 2, 3, 4)


@phyfun
def test_make_list():
    a = 1
    b = 2
    c = 3
    return make_list(a, b, c)


@phyfun
def test_make_list2():
    a = 1
    b = 2
    c = 3
    return make_list(a, make_list(a, b, c), c)


@phyfun
def test_make_list_arg(l):
    a = 1
    c = 3
    return make_list(a, l, c)


assert test_make_list_empty() == []
assert test_make_list_one() == [42]
assert test_make_list_literals() == [1, 2, 3, 4]
assert test_make_list() == [1, 2, 3]
assert test_make_list2() == [1, [1, 2, 3], 3]
assert test_make_list_arg([1, 2, 3]) == [1, [1, 2, 3], 3]
