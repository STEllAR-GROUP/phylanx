#  Copyright (c) 2020 R. Tohid
#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1104 No arg lambdas don't work

from phylanx import Phylanx

@Phylanx
def foo():
    f = lambda : print("Hello")

    return True

assert True == foo()