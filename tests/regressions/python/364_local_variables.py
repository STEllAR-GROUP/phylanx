#  Copyright (c) 2018 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #364: Multiple Function Calls Produces Incorrect Results

from phylanx import Phylanx


@Phylanx
def foo(n):
    x = 0
    if n == 1:
        foo(2)
    elif n == 2:
        x = 3
    return x


assert(foo(1) == 0)
