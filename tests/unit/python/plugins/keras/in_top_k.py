#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx.plugins.keras import in_top_k


# source: https://bit.ly/2ZvM9Fx
def np_in_top_k(predictions, targets, k):
    top_k = np.argsort(-predictions)[:, :k]
    targets = targets.reshape(-1, 1)
    return np.any(targets == top_k, axis=-1)


prediction = np.array([[0, 1], [3, 2]])
target = np.array([0, 1])

assert (in_top_k(prediction, target, 2) == np_in_top_k(prediction, target, 2)).all()
