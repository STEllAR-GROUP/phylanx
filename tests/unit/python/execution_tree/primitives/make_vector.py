#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *

@Phylanx("PhySL")
def test_make_vector_one():
    return hstack(42)

@Phylanx("PhySL")
def test_make_vector_literals():
    return hstack(1, 2, 3, 4)


@Phylanx("PhySL")
def test_make_vector():
    a = 1
    b = 2
    c = 3
    return hstack(a, b, c)


@Phylanx("PhySL")
def test_make_vector2():
    a = 1
    b = 2
    c = 3
    return hstack(a, hstack(a, b, c), c)

assert test_make_vector_one() == [42]
assert test_make_vector_literals() == [1, 2, 3, 4]
assert test_make_vector() == [1, 2, 3]
assert test_make_vector2() == [1, 1, 2, 3, 3]