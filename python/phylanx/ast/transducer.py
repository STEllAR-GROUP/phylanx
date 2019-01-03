# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
# Copyright (c) 2018 Ye Fang
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import os
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

from .utils import dump_to_file



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
            self.backend = m[kwargs["target"]](func, python_ast, kwargs)
            self.__src__ = self.backend.__src__


        def get_args(self):
            """
            Build default kwargs from the list (via copying the first item).
            Overwrite the default kwargs if the key/value is passed in.
            Validate the key/value.
            """

            valid_kwargs = {
                    "target"            : ["PhySL", "OpenSCoP"],
                    "compiler_state"    : [None, "skip_validation"],
                    "performance"       : [None, "skip_validation"],
                    "debug"             : [True, False],

                    # verbos mode, printing the compilation details
                    "verbos"            : [True, False],
                    # saving python src to file
                    "dump_python_src"   : [True, False],
                    # saving python ast to file
                    "dump_python_ast"   : [True, False],
                    # saving openscop file, should combine with "target="OpenSCoP""
                    "dump_openscop"     : [True, False],
                    # saving physl file, should combine with "target="PhySL""
                    "dump_physl"        : [True, False],
                    # assigning unique uname to decorated functions
                    "python_src_tag"    : ["skip_validation"],
                    }
            default_kwargs = {}
            for key, value in valid_kwargs.items():
                default_kwargs[key] = value[0]

            # overwrite default kwargs
            default_kwargs.update(kwargs)
            kwargs.update(default_kwargs)

            # validate the keys
            for key in kwargs.keys():
                if key not in valid_kwargs:
                    raise NotImplementedError \
                        ("Unknown Phylanx kwarg key %s" % key)

            # validate the values
            for key, value in kwargs.items():
                v2 = valid_kwargs[key]
                if ((value not in v2) and ("skip_validation" not in v2)):
                    raise NotImplementedError \
                        ("Unknown Phylanx kwarg: %s = " % key, value)

            # dump kwargs
            if kwargs["verbos"] == True:
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

            # Strip off indentation if the function is not defined at top
            # level.
            src = re.sub(r'^\s*', '', src)

            if kwargs["dump_python_src"] == True:
                ofn = "./dump_python_src_" + kwargs["python_src_tag"] + ".txt"
                dump_to_file(src, ofn, kwargs["verbos"])

            return src


        def get_python_ast(self, src, func):
            """Generates the Python AST."""

            tree = ast.parse(src)
            actual_lineno = inspect.getsourcelines(func)[-1]
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1

            if kwargs["dump_python_ast"] == True:
                aststr = astpretty.pformat(tree, show_offsets=False)
                ofn = "./dump_python_ast_" + kwargs["python_src_tag"] + ".txt"
                dump_to_file(aststr, ofn, kwargs["verbos"])

            return tree



        def __call__(self, *args):
            if kwargs["target"] == "OpenSCoP":
                raise NotImplementedError \
                    ("OpenSCoP kernels are not yet callable.")

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


