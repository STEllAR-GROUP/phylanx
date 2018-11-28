# Copyright (c) 2018 R. Tohid
# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #407: @Phylanx decorator needs parentheses

from phylanx import Phylanx, PhylanxSession
from phylanx.exceptions import InvalidDecoratorArgumentError

PhylanxSession(1)


@Phylanx
def a():
    return 0


@Phylanx()
def b():
    return 0


@Phylanx(target="PhySL")
def c():
    return 0


@Phylanx(debug=True)
def d():
    return 0


raised = False
try:

    @Phylanx("PhySL")
    def e():
        return 0
except InvalidDecoratorArgumentError:
    raised = True
assert raised
