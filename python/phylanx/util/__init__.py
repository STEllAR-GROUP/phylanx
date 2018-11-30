# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

try:
    from phylanx._phylanx.util import *

except Exception:
    from phylanx._phylanxd.util import *


def phyhelp(fname):
    """Python help function. When called it with the name of
a Phylanx primitive or plugin it prints the associated
help message.

Args:
    fname (string) : the name of the Phylanx primitive or plugin
    """
    # Retrieve the docstring from the Phylanx API
    ds = phyhelpex(fname)
    n = ds.find('\n')
    # The first line of the Phylanx doc string is the arglist
    args = ds[0:n]
    # We're going to use the builtin help function to display
    # the message. To make that work, however, we're going to
    # create a dummy function with a matching arg list and
    # assign the docstring and function name.
    dummy = eval("lambda %s : 1" % args)
    dummy.__doc__ = ds[n:]
    dummy.__name__ = fname
    help(dummy)
