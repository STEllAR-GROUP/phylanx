#  Copyright (c) 2019 Bita Hasheminezhad
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# #956: Cannot pass a two-part named argument


import numpy as np
from phylanx import Phylanx


# silence flake
def ctc_decode(y_pred, input_length, greedy, beam_width):
    pass


@Phylanx
def ctc_decode_eager(y_pred, input_length, greedy=True, beam_width=100):
    return ctc_decode(y_pred, input_length, greedy=greedy, beam_width=beam_width)


y_pred = np.array([[[1, 2, 3, 4, 5]]])
input_length = np.array([1, 5])

r = ctc_decode_eager(y_pred, input_length, True, 100)
assert np.allclose(r, [np.array([[4.]]), np.array([[-1.60943791]])]), r
