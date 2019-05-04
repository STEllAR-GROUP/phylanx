#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def fx_not(x):
    if not x:
        return "if_ case"
    else:
        return "else_ case"


assert fx_not(True) == "else_ case"
assert fx_not(False) == "if_ case"
assert fx_not(None) == "if_ case"


@Phylanx
def fx(x):
    if x:
        return "if_ case"
    else:
        return "else_ case"


assert fx(True) == "if_ case"
assert fx(False) == "else_ case"
assert fx(None) == "else_ case"
