#  Copyright (c) 2019 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import math
import numpy as np
from phylanx import Phylanx


@Phylanx
def np_inf():
    return np.inf


assert np_inf() == np.inf


@Phylanx
def np_Inf():
    return np.Inf


assert np_Inf() == np.Inf


@Phylanx
def np_Infinity():
    return np.Infinity


assert np_Infinity() == np.Infinity


@Phylanx
def np_PINF():
    return np.PINF


assert np_PINF() == np.PINF


@Phylanx
def np_NINF():
    return np.NINF


assert np_NINF() == np.NINF


@Phylanx
def np_nan():
    return np.nan


assert math.isnan(np_nan())


@Phylanx
def np_NaN():
    return np.NaN


assert math.isnan(np_NaN())


@Phylanx
def np_NAN():
    return np.NAN


assert math.isnan(np_NAN())


@Phylanx
def np_PZERO():
    return np.PZERO


assert np_PZERO() == np.PZERO


@Phylanx
def np_NZERO():
    return np.NZERO


assert np_NZERO() == np.NZERO


@Phylanx
def np_e():
    return np.e


assert np_e() == np.e


@Phylanx
def np_euler_gamma():
    return np.euler_gamma


assert np_euler_gamma() == np.euler_gamma


@Phylanx
def np_pi():
    return np.pi


assert np_pi() == np.pi
