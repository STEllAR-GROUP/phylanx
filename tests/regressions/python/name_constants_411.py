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


assert f.get_physl_source() == \
    "define$11$0(f$11$0, lambda$11$0(block(define$12$4(a$12$4, nil), a$13$11)))", \
    f.get_physl_source()


@Phylanx
def f1():
    a = True
    return a


assert f1.get_physl_source() == \
    "define$22$0(f1$22$0, lambda$22$0(block(define$23$4(a$23$4, true), a$24$11)))", \
    f1.get_physl_source()


@Phylanx
def f2():
    a = False
    return a


assert f2.get_physl_source() == \
    "define$33$0(f2$33$0, lambda$33$0(" + \
    "block(define$34$4(a$34$4, false), a$35$11)))", \
    f2.get_physl_source()
