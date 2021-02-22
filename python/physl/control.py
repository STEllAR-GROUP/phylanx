from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast
import inspect

from types import FunctionType
from typing import Any


class Profile:
    def __init__(self) -> None:
        self.called = 0

    def __call__(self, *args: Any, **kwargs: Any) -> Any:
        self.called += 1


class Task:
    def __init__(self, fn: FunctionType) -> None:

        self.fn = fn
        self.dtype = None
        self.id = self.fn.__hash__()

        self.py_src = inspect.getsource(fn)
        self.py_ast = ast.parse(self.py_src)

        self.profile = Profile()

    def __call__(self, *args: Any, **kwargs: Any) -> Any:
        self.profile()
        return self.fn(*args, **kwargs)

    def __hash__(self) -> int:
        return self.id
