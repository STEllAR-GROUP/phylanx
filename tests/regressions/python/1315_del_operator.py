#  Copyright (c) 2021 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# 1315: Missing del operator for dictionaries

from phylanx import Phylanx


@Phylanx
def test():
    a = {"a": 1, "b": 2, "c": 3}
    del a["a"]
    return a


result = test()
assert result == {"b": 2, "c": 3}, result
