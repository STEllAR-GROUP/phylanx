from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

from abc import ABC, abstractmethod
# from collections import ChainMap
from collections import defaultdict
from typing import Union


class Namespace:
    complete_namespace = []

    def __init__(self, namespace):
        self.namespace = namespace

    def __enter__(self):
        self.push(self.namespace)
        return self

    def __exit__(self, type, value, traceback):
        self.pop()

    def get_complete_namespace(self):
        return self._complete_namespace()

    @staticmethod
    def get_namespace():
        return Namespace.complete_namespace

    @classmethod
    def _complete_namespace(cls):
        return cls.complete_namespace

    @classmethod
    def push(cls, namespace):
        cls.complete_namespace.append(namespace)

    @classmethod
    def pop(cls):
        cls.complete_namespace.pop()

    @classmethod
    def get_complete_namespace_str(cls):
        return '+'.join(cls.complete_namespace)

    def get_current_space(self):
        return self.namespace


class SymbolTable(ABC):
    table = defaultdict
    def __init__(self) -> None:
        ...

    @classmethod
    @abstractmethod
    def add_symbol(cls, symbol):
        ...

    @classmethod
    @abstractmethod
    def check_symbol(cls, symbol_name, symbol_type, namespace=None):
        ...


class FnTable(SymbolTable):

    def __init__(self) -> None:
        super().__init__()

    @classmethod
    def add_symbol(cls, symbol):
        FnTable.table

    def check_symbol(cls, symbol_name, symbol_type, namespace=None):
        ...


class DataTable(SymbolTable):
    def __init__(self) -> None:
        super().__init__()

    @classmethod
    def add_symbol(cls, symbol):
        DataTable.table
