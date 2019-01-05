#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx import Phylanx, PhylanxSession
from phylanx.exceptions import RuntimeAlreadyInitializedError

try:
    PhylanxSession.init(1)
    PhylanxSession.init(1)
    raise Exception()
except RuntimeAlreadyInitializedError:
    pass
