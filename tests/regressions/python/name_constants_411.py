# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx import Phylanx


@Phylanx
def f():
    a = None
    return a


assert f.__src__ == \
    "define$11$0(f$11$0, block(define$12$4(a$12$4, nil$12$8), a$13$11))"


@Phylanx
def f():
    a = True
    return a


assert f.__src__ == \
    "define$21$0(f$21$0, block(define$22$4(a$22$4, true$22$8), a$23$11))"


@Phylanx
def f():
    a = False
    return a


assert f.__src__ == \
    "define$31$0(f$31$0, block(define$32$4(a$32$4, false$32$8), a$33$11))"
