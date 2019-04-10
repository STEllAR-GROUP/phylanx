# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# Fixing #453: Bug in passing compiler state through @Phylanx()

import phylanx
from phylanx import execution_tree as et
from phylanx import Phylanx, PhylanxSession

PhylanxSession.init(1)

cs = et.compiler_state(__name__)

src_mul2 = """
define(mul2, n,
    block(
        define(a, 0),
        store(a, 2 * n),
        a
    )
)
"""
et.compile(cs, 'passing_compiler_state_453.py', 'mul2', src_mul2)


@Phylanx(compiler_state=cs)
def foo(n):
    return mul2(n)  # noqa: F821


assert (6.0 == foo(3))
