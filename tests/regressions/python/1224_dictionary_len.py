#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1224: Can't get length of a dictionary

from phylanx import Phylanx


@Phylanx
def foo():
    a = {"a": 1, "b": 2, "c": 3}
    return len(a)


result = foo()
assert result == 3, result
