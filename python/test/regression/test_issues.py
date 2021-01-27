from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

from physl.decorators import Phylanx


@Phylanx
def test_364(n):
    x = 0
    if n == 1:
        test_364(2)
    elif n == 2:
        x = 3
    return x


assert (test_364(1) == 0)
