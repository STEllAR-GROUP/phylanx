# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx.ast import *


@Phylanx()
def f():
    a = None
    return a


assert f.__src__ == \
    "define$11$0(f$11$0, block$11$0(define$12$4(a$12$4, nil$12$8), a$13$11))"


@Phylanx(debug=True)
def f():
    a = True
    return a


assert f.__src__ == \
    "define$21$0(f$21$0, block$21$0(define$22$4(a$22$4, true$22$8), a$23$11))"


@Phylanx(debug=True)
def f():
    a = False
    return a


assert f.__src__ == \
    "define$30$0(f$30$0, block$30$0(define$31$4(a$31$4, false$31$8), a$32$11))"
