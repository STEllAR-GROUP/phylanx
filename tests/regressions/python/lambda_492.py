#  Copyright (c) 2018 Stevn R. Brandt
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession

PhylanxSession(1)


@Phylanx
def foo():
    f = lambda a: a  # noqa: E731
    f(42)


assert (foo() == 42)
