#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *
from phylanx.util import *


@Phylanx
def test_make_vector_empty():
    return make_vector()


@Phylanx
def test_make_vector_one():
    return make_vector(42)


@Phylanx
def test_make_vector_literals():
    return make_vector(1, 2, 3, 4)


@Phylanx
def test_make_vector():
    a = 1
    b = 2
    c = 3
    return make_vector(a, b, c)


@Phylanx
def test_make_vector2():
    a = 1
    b = 2
    c = 3
    return make_vector(a, make_vector(a, b, c), c)


assert test_make_vector_empty() == []
assert test_make_vector_one() == [42]
assert test_make_vector_literals() == [1, 2, 3, 4]
assert test_make_vector() == [1, 2, 3]
assert test_make_vector2() == [1, 1, 2, 3, 3]
