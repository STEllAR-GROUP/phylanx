# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import phylanx
from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def f():
    a = None
    return a


assert f.__src__ == \
    "define$13$0(f$13$0, block(define$14$4(a$14$4, nil$14$8), a$15$11))"


@Phylanx
def f():
    a = True
    return a


assert f.__src__ == \
    "define$23$0(f$23$0, block(define$24$4(a$24$4, true$24$8), a$25$11))"


@Phylanx
def f():
    a = False
    return a


assert f.__src__ == \
    "define$33$0(f$33$0, block(define$34$4(a$34$4, false$34$8), a$35$11))"
