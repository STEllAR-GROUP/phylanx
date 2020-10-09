#  Copyright (c) 2020 Steven R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #1279: Support global keyword

from phylanx import Phylanx


# silence flake
r1 = None
r2 = None


@Phylanx
def prepare():
    global r1, r2
    r1 = 42
    r2 = '42'


@Phylanx
def access_r1():
    global r1
    return r1


@Phylanx
def access_r2():
    global r2
    return r2


prepare()

result = access_r1()
assert result == 42, result

result = access_r2()
assert result == '42', result
