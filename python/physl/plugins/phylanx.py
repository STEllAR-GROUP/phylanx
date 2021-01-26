from __future__ import absolute_import
from __future__ import annotations

__license__ = """ 
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast

from types import FunctionType
from typing import Union

from physl.task import Transpiler


class Transpilation(ast.NodeVisitor):
    def __init__(self, py_ast) -> None:
        self.py_ast = py_ast

    def visit_FunctionDef(self, node: ast.FunctionDef) -> None:
        return 1

    def visit_Module(self, node: ast.Module) -> str:
        return node.body


class PhySL:
    def __init__(self, fn: FunctionType) -> None:
        self.transpile = Transpiler(self.py_ast, Transpilation)