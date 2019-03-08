#  Copyright (c) 2019 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #832: Phylanx Performance Data Broken

from phylanx import Phylanx


@Phylanx(performance=True)
def fib(n):
    if n < 2:
        return n
    else:
        return fib(n - 1) + fib(n - 2)


assert fib(10) == 55
