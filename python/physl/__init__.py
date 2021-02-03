from __future__ import absolute_import

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import sys
if (3, 8) > sys.version_info:
    raise RuntimeError(
        f"PhySL requires Python 3.8 or newer. Running with {sys.version_info}")
