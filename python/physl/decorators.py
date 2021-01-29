from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

from abc import ABC

from types import FunctionType
from typing import Any, Union

from physl.control import Task
from physl.plugins.phylanx import PhySL


class Decorator(ABC):
    def __init__(self, fn: Union[FunctionType, Task]) -> None:
        if isinstance(fn, FunctionType):
            self.task: Task = Task(fn)
        elif isinstance(fn, Task):
            self.task: Task = fn
        else:
            raise TypeError


class Phylanx(Decorator):
    def __init__(self,
                 fn: Union[FunctionType, Task],
                 debug=None,
                 compiler_state=None,
                 profile=None,
                 verbose=None) -> None:
        super().__init__(fn)

        self.physl: PhySL = PhySL(self.task)

    def __call__(self, *args: Any, **kwargs: Any) -> Any:
        return self.physl()


