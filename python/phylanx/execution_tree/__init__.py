# Copyright (c) 2017-2019 Hartmut Kaiser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import inspect
try:
    from phylanx._phylanx.execution_tree import *

except Exception:
    from phylanx._phylanxd.execution_tree import *


class parallel:

    @staticmethod
    def is_parallel_block():
        return True


# find name of file that imported this file
_name_of_importing_file = None

if __name__ != '__main__':
    if _name_of_importing_file is None:
        for frame in inspect.stack()[1:]:
            if frame.filename[0] != '<':
                _name_of_importing_file = frame.filename


# global compiler state is initialized only once
_compiler_state = None


def global_compiler_state(file_name=None):

    global _compiler_state
    if _compiler_state is None:
        from phylanx import PhylanxSession
        if not PhylanxSession.is_initialized:
            PhylanxSession.init(1)

        if file_name is None:
            file_name = _name_of_importing_file

        _compiler_state = compiler_state(file_name)

    return _compiler_state


# wrap variable such that user does not need to supply compiler_state
class variable(variable_impl):

    def __init__(self, *args, **kwargs):
        """initialize variable instance"""

        if len(args) == 1 and len(kwargs) == 0:
            if isinstance(args[0], variable):
                super(variable, self).__init__(args[0])
                return

        super(variable, self).__init__(global_compiler_state(), *args, **kwargs)
