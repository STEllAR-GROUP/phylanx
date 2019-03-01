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
    "define$11$0(f$11$0, lambda$11$0(block(define$12$4(a$12$4, nil$12$8), a$13$11)))", \
    f.__src__


@Phylanx
def f():
    a = True
    return a


assert f.__src__ == \
    "define$22$0(f$22$0, lambda$22$0(block(define$23$4(a$23$4, true$23$8), a$24$11)))", \
    f.__src__


@Phylanx
def f():
    a = False
    return a


assert f.__src__ == \
    "define$33$0(f$33$0, lambda$33$0(block(define$34$4(a$34$4, false$34$8), a$35$11)))", \
    f.__src__
