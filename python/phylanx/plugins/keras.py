#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def in_top_k(predictions, targets, k):
    top_k = np.argsort(-predictions)[:, :k]
    target = reshape(targets, [-1, 1])  # noqa
    return np.any(target == top_k, -1)
