#  Copyright (c) 2018 Weile Wei
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx


@Phylanx
def foo():
    a = [1, 2]
    a[0] = 55

    return a


assert (foo() == [55, 2])
