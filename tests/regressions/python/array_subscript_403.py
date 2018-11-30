#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx


@Phylanx
def foo():
    v1 = [0, 0, 0]
    v1[1] = 1
    return v1


print(foo())
assert (foo() == [0, 1, 0])
