#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1206: Improper handling of defaults
from phylanx import Phylanx


q = 57


@Phylanx
def test1(n=q):
    return n


@Phylanx
def test2(n=10):
    return n


result = test1()
assert result == 57, result

result = test2()
assert result == 10, result
