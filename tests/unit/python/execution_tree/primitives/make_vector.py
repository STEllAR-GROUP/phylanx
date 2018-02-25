#  Copyright (c) 2018 Hartmut Kaiser
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.util import *


@phyfun
def test_make_vector_empty():
    return make_vector()


@phyfun
def test_make_vector_one():
    return make_vector(42)


@phyfun
def test_make_vector_literals():
    returnmake_vector(1, 2, 3, 4)


@phyfun
def test_make_vector():
    a = 1
    b = 2
    c = 3
    return test_make_vector(a, b, c)


assert test_make_vector_empty() == []
assert test_make_vector_one() == [42]
assert test_make_vector_literals() == [1, 2, 3, 4]
assert test_make_vector() == [1, 2, 3]
