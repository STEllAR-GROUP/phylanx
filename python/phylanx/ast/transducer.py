# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
# Copyright (c) 2018 Ye Fang
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import re
import ast
import astpretty
import inspect
import phylanx
from .physl import PhySL
from .oscop import OpenSCoP
from phylanx import execution_tree
from phylanx.ast import generate_ast as generate_phylanx_ast
from phylanx.exceptions import InvalidDecoratorArgumentError
from phylanx.ast.utils import dump_to_file
# from phylanx.ast.utils import python_ast_format


def Phylanx(__phylanx_arg=None, **kwargs):
    class __PhylanxDecorator(object):
        def __init__(self, func):
            """
            :function:func the decorated funtion.
            """
            self.get_args()
            self.get_env(func)
            python_src = self.get_python_src(func)
            python_ast = self.get_python_ast(python_src, func)

            m = {"PhySL": PhySL, "OpenSCoP": OpenSCoP}
            t = kwargs["target"]
            if t not in m.keys():
                raise NotImplementedError(\
                    "target=%s is not available" % t)

            self.backend = m[t](func, python_ast, kwargs)
            self.__src__ = self.backend.__src__

        def get_args(self):
            """
            List of kwargs keys and valid values
                target:             "PhySL", "OpenSCoP",

                compiler_stat:      None, WHATEVER

                performance:        None, WHATEVER

                debug:              True, False, WHATEVER

                dump_python_src:    True, False, "path_to_file_you_specified",
                saving python src to file

                dump_python_ast:    True, False, "path_to_file_you_specified",
                saving python ast to file

                dump_openscop:      True, False, "path_to_file_you_specified",
                saving opnscop file, should combine with target="OpenSCoP"

                dump_physl:         True, False, "path_to_file_you_specified",
                saving physl file, should combine with target="PhySL"
            """

            default_kwargs = {
                "target": "PhySL",
                "compiler_state": None,
                "performance": None,
                "debug": False,
                "dump_python_src": False,
                "dump_python_ast": False,
                "dump_openscop": True,
                "dump_physl": False,
            }

            # overwrite default kwargs
            default_kwargs.update(kwargs)
            kwargs.update(default_kwargs)

            # validate the keys
            for key in kwargs.keys():
                if key not in default_kwargs.keys():
                    raise NotImplementedError(\
                        "Unknown Phylanx kwarg key %s" % key)

            # dump kwargs
            if kwargs["debug"] is True:
                for key, value in kwargs.items():
                    print("Phylanx kwargs:     %-20s =" % key, value)

        def get_env(self, func):

            # Obtain global environment if the object is a function.
            if inspect.isclass(func):
                kwargs["fglobals"] = None
            else:
                kwargs["fglobals"] = func.__globals__

            # Assign each decorated function a unique tag
            f1 = func.__code__.co_filename
            f2 = inspect.getsourcelines(func)[-1]
            f3 = func.__name__
            s = "%s_line%d_%s" % (f1, f2, f3)
            s = s.replace("/", "")
            kwargs["python_src_tag"] = s

        def get_python_src(self, func):
            """Gets the function's source and removes the decorator line."""

            # get the source and remove the decorator line.
            src = inspect.getsource(func)
            src = re.sub(r'^\s*@\w+.*\n', '', src)

            # Strip off indentation if the function is not defined at top level
            src = re.sub(r'^\s*', '', src)

            dump_to_file(src, "dump_python_src", kwargs)

            return src

        def get_python_ast(self, src, func):
            """Generates the Python AST."""

            tree = ast.parse(src)
            actual_lineno = inspect.getsourcelines(func)[-1]
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1

            aststr = astpretty.pformat(tree, show_offsets=False)  # prettier
#           aststr = python_ast_format(tree)                      # less pretty
            dump_to_file(aststr, "dump_python_ast", kwargs)

            return tree

        def __call__(self, *args):
            if kwargs["target"] == "OpenSCoP":
                raise NotImplementedError(\
                    "OpenSCoP kernels are not yet callable.")

            result = self.backend.call(args)
            self.__perfdata__ = self.backend.__perfdata__

            return result

        def generate_ast(self):
            return generate_phylanx_ast(self.__src__)

    if callable(__phylanx_arg):
        return __PhylanxDecorator(__phylanx_arg)
    elif __phylanx_arg is not None:
        raise InvalidDecoratorArgumentError
    else:
        return __PhylanxDecorator
