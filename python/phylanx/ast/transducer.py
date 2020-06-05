# Copyright (c) 2017-2019 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018-2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import re
import ast
import inspect
import types
import phylanx
from .physl import PhySL
from .openscop import OpenSCoP
from phylanx import execution_tree
from phylanx.ast import generate_ast as generate_phylanx_ast
from phylanx.exceptions import InvalidDecoratorArgumentError


class LambdaExtractor(ast.NodeVisitor):
    _ast = None

    def visit_Lambda(self, node):
        LambdaExtractor._ast = node


def lambda_counter(init=[0]):
    init[0] += 1
    return init[0]


def Phylanx(__phylanx_arg=None, **kwargs):
    class _PhylanxDecorator(object):
        def __init__(self, f):
            """
            :function:f the decorated funtion.
            """
            valid_kwargs = [
                'debug',
                'target',
                'compiler_state',
                'doc_src',
                'performance',
                'localities',
                'startatlineone'
            ]

            self.backends_map = {'PhySL': PhySL, 'OpenSCoP': OpenSCoP}
            self.backend = self.get_backend(kwargs.get('target'))
            self.startatlineone = kwargs.get("startatlineone", False)
            for key in kwargs.keys():
                if key not in valid_kwargs:
                    raise NotImplementedError("Unknown Phylanx argument '%s'." % key)

            # Obtain global environment if the object is a function.
            if inspect.isclass(f):
                kwargs['fglobals'] = None
            else:
                kwargs['fglobals'] = f.__globals__

            python_src = self.get_python_src(f)
            python_ast = self.get_python_ast(python_src, f)

            self.kwargs = kwargs
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
            if self.startatlineone:
                actual_lineno = 0
            else:
                actual_lineno = inspect.getsourcelines(f)[-1]
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1
            return tree

        def get_physl_source(self):
            """Return generated PhySL source string"""

            return self.backend.get_physl_source()

        def compile_function(self, func):
            fn_src = inspect.getsource(func).strip()
            fn_ast = ast.parse(fn_src)
            if func.__name__ == '<lambda>':
                func.__name__ = "__Physl_lambda_%d" % lambda_counter()
                LambdaExtractor().visit(fn_ast)
                lambda_ast = LambdaExtractor._ast
                fn_body = ast.FunctionDef(
                    name=func.__name__,
                    args=lambda_ast.args,
                    body=lambda_ast.body,
                    lineno=lambda_ast.lineno,
                    col_offset=lambda_ast.col_offset)
                fn_ast = ast.Module(body=[fn_body])
            return PhySL(func, fn_ast, self.kwargs)

        def map_decorated(self, val):
            """If a PhylanxDecorator is passed as an argument to an
               invocation of a Phylanx function we need to extract the
               compiled execution tree and pass along that instead.
               If a Python lambda function is passed as an argument to an
               an invocation of Phylanx function we need to compile the
               lambda into Physl and pass along the compiled tree.
            """

            typename = type(val).__name__
            if typename == "_PhylanxDecorator":
                return val.backend.lazy()

            if typename == "_PhylanxLazyDecorator":
                return val()

            if isinstance(val, types.FunctionType):
                fn_physl = self.compile_function(val)
                return fn_physl.lazy()

            return val

        def lazy(self, *args, **kwargs):
            """Compile this decorator, return wrapper binding the function to
               arguments"""

            if self.backend == 'OpenSCoP':
                raise NotImplementedError(
                    "OpenSCoP kernels are not yet callable.")

            mapped_args = tuple(map(self.map_decorated, args))
            kwitems = kwargs.items()
            mapped_kwargs = {k: self.map_decorated(v) for k, v in kwitems}
            return self.backend.lazy(*mapped_args, **mapped_kwargs)

        def __call__(self, *args, **kwargs):
            """Invoke this decorator using the given arguments"""

            if self.backend == 'OpenSCoP':
                raise NotImplementedError(
                    "OpenSCoP kernels are not yet callable.")

            mapped_args = tuple(map(self.map_decorated, args))
            kwitems = kwargs.items()
            mapped_kwargs = {k: self.map_decorated(v) for k, v in kwitems}
            result = self.backend.call(*mapped_args, **mapped_kwargs)

            self.__perfdata__ = self.backend.__perfdata__

            return result

        def generate_ast(self):
            return generate_phylanx_ast(self.__src__)

        def tree(self):
            return self.backend.tree()

    class _PhylanxLazyDecorator:
        def __init__(self, decorator):
            """
            :function:decorator the decorated funtion.
            """
            self.decorator = decorator

        def __call__(self, *args, **kwargs):
            """Invoke this decorator"""
            return self.decorator.lazy(*args, **kwargs)

        def tree(self):
            """Return the tree data for this decorator"""
            return self.decorator.tree()

    # expose lazy version of the decorator
    def lazy(decorator):
        if type(decorator).__name__ != "_PhylanxDecorator":
            raise InvalidDecoratorArgumentError
        return _PhylanxLazyDecorator(decorator)

    setattr(Phylanx, 'lazy', lazy)

    if callable(__phylanx_arg):
        return _PhylanxDecorator(__phylanx_arg)
    elif __phylanx_arg is not None:
        raise InvalidDecoratorArgumentError
    else:
        return _PhylanxDecorator
