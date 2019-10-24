# Copyright (c) 2018 R. Tohid
# Copyright (c) 2018 Parsa Amini
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


class InvalidDecoratorArgumentError(Exception):
    """Phylanx decorator or Phylanx.lazy used with invalid arguments"""
    pass


class RuntimeNotInitializedError(Exception):
    """PhylanxSession must be initialized before using the Phylanx decorator."""
    pass


class RuntimeAlreadyInitializedError(Exception):
    """PhylanxSession can initialized only once."""
    pass
