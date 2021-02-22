from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import inspect
import symtable

from abc import ABC, abstractmethod
from collections import defaultdict, deque
from typing import Any, List

from physl.control import Task


class Namespace:
    complete_namespace = deque()

    def __init__(self, namespace):
        self.namespace = namespace

    def __enter__(self):
        self.push(self.namespace)
        return self

    def __exit__(self, type, value, traceback):
        self.pop()

    def get_complete_namespace(self):
        return tuple(self._complete_namespace())

    @classmethod
    def get_namespace(cls):
        return tuple(cls.complete_namespace)

    @classmethod
    def push(cls, namespace):
        cls.complete_namespace.append(namespace)

    @classmethod
    def pop(cls):
        cls.complete_namespace.pop()


class Symbol:
    def __init__(self, name: str, namespace: Namespace) -> None:
        self.name = name
        self.namespace = namespace


class FnSymbol(Symbol):
    def __init__(self, name: str, namespace: Namespace) -> None:
        super().__init__(name, namespace)


class DataSymbol(Symbol):
    def __init__(self, name: str, namespace: Namespace) -> None:
        super().__init__(name, namespace)


class SymbolTable:
    table = defaultdict()

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


class TaskTable(SymbolTable):
    """Table of symbols within a task.

    Holds symbols' information that'll be used during transpilation.
    """
    def __init__(self, task: Task) -> None:
        src = task.py_src
        fn = task.fn
        file_name = inspect.getfile(fn).split('/')[-1].split('.')[0]
        table_name = "__phy_task+" + file_name
        self.py_table = symtable.symtable(src, file_name,
                                          "exec").get_children()[0]

        # self.table = defaultdict(lambda: list())
        self.build_table(table_name, self.py_table)
        self.register_table(self)

    def build_table(self, table_name: str, table: symtable.SymbolTable):
        with Namespace(table_name) as ns:
            symbols = table.get_symbols()
            namespace = ns.get_namespace()
            for sym in symbols:
                self.table[namespace].append(sym)

            children = table.get_children()
            for child in children:
                self.build_table(child.get_name(), child)

    @classmethod
    def register_table(cls, self: TaskTable):
        cls.global_table[self.py_table.get_id()] = self.table


class FnTable(SymbolTable):
    def __init__(self, task: Task) -> None:
        ...

    def create_table(self):
        ...

    @classmethod
    def add_symbol(cls, symbol):
        FnTable.table

    def check_symbol(cls, symbol_name, symbol_type, namespace=None):
        ...


class ClassTable(SymbolTable):
    ...


class DataTable(SymbolTable):
    def __init__(self) -> None:
        super().__init__()

    @classmethod
    def add_symbol(cls, symbol):
        DataTable.table


class ClassTable(SymbolTable):
    ...


_tables = {'function': FnTable, 'class': ClassTable}
