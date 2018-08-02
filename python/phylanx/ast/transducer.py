# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import re
import ast
import inspect
import phylanx
from .physl import PhySL
from .oscop import OpenSCoP
from phylanx import execution_tree
from phylanx.ast import generate_ast as generate_phylanx_ast
from phylanx.exceptions import InvalidDecoratorArgumentError


def Phylanx(__phylanx_arg=None, **kwargs):
    class __PhylanxDecorator(object):
        def __init__(self, f):
            """
            :function:f the decorated funtion.
            """

            self.backends_map = {'PhySL': PhySL, 'OpenSCoP': OpenSCoP}
            self.backend = self.get_backend(kwargs.get('target'))

            # Obtain global environment if the object is a function.
            if inspect.isclass(f):
                kwargs['fglobals'] = None
            else:
                kwargs['fglobals'] = f.__globals__

            python_src = self.get_python_src(f)
            python_ast = self.get_python_ast(python_src, f)

            self.backend = self.backends_map[self.backend](f, python_ast, kwargs)
            self.__src__ = self.backend.__src__

        def get_backend(self, target):
            """Set the target backend. By default it is set to PhySL."""

            if target:
                if target not in self.backends_map:
                    raise NotImplementedError("Unknown target: %s." % target)
                return target
            else:
                return 'PhySL'

        def get_python_src(self, f):
            """Gets the function's source and removes the decorator line."""

            # get the source and remove the decorator line.
            src = inspect.getsource(f)
            src = re.sub(r'^\s*@\w+.*\n', '', src)

            # Strip off indentation if the function is not defined at top
            # level.
            src = re.sub(r'^\s*', '', src)
            return src

        def get_python_ast(self, src, f):
            """Generates the Python AST."""

            tree = ast.parse(src)
            actual_lineno = inspect.getsourcelines(f)[-1]
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1
            return tree

        def __call__(self, *args):
            if self.backend == 'OpenSCoP':
                raise NotImplementedError(
                    "OpenSCoP kernels are not yet callable.")
            return self.backend.call(args)

        def generate_ast(self):
            return generate_phylanx_ast(self.__src__)

    if callable(__phylanx_arg):
        return __PhylanxDecorator(__phylanx_arg)
    elif __phylanx_arg is not None:
        raise InvalidDecoratorArgumentError
    else:
        return __PhylanxDecorator
