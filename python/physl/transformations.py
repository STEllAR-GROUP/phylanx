from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast

from abc import ABC, abstractmethod


class Transpiler(ABC, ast.NodeVisitor):
    def __init__(self, task: Task) -> None:
        self.task = task
        self.py_ast = task.py_ast
        self.target = self.transpile(self.py_ast)

    @abstractmethod
    def transpile(self, node):
        returned = self.visit(node)
        if returned is None:
            raise NotImplementedError(
                f"Transformation rule for {node} is not implemented.")
        else:
            return returned