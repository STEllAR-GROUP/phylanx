from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

from abc import ABC
from collections import OrderedDict
from typing import Any, List


class Load:
    ...


class Store:
    ...


class Del:
    ...


class Attribute:
    def __init__(self, lineno, col_offset) -> None:
        self._lineno = lineno
        self._col_offset = col_offset

    def gen_code(self):
        return f"${self.lineno}${self.col_offset}"

    @property
    def lineno(self):
        return self._lineno

    @lineno.setter
    def lineno(self, lineno):
        self._lineno = lineno

    @property
    def col_offset(self):
        return self._col_offset

    @col_offset.setter
    def col_offset(self, col_offset):
        self._col_offset = col_offset

    def __eq__(self, o: Attribute) -> bool:
        return (self.lineno, self.col_offset) == (o.lineno, o.col_offset)


class Expr(ABC):
    def __init__(self,
                 name: str,
                 namespace,
                 lineno: int = None,
                 col_offset: int = None):
        self.name = name
        self.namespace = namespace
        self.loc = Attribute(lineno, col_offset)

    def namespace_str(self):
        return str(self.namespace)


class Variable(Expr):
    def __init__(self,
                 name: str,
                 namespace,
                 lineno: int,
                 col_offset: int,
                 dtype=None):
        super().__init__(name, namespace, lineno, col_offset)
        self.type = dtype

    def __eq__(self, o: Variable):
        self_ = (self.name, self.namespace, self.loc)
        other_ = (o.name, o.namespace, o.lineno, o.loc)
        return self_ == other_


class Argument(Expr):
    def gen_code(self):
        return f'{self.name}{self.loc.gen_code()}'


class Buffer(Variable):
    def __init__(self,
                 name: str,
                 namespace,
                 lineno: int,
                 col_offset: int,
                 dimension: int = None,
                 shape: int = None):
        super().__init__(name, namespace, lineno, col_offset)

        if dimension and dimension < 1:
            raise ValueError(
                f"Arrays must have 1 or more dimension(s). {dimension} given.")
        self.dimensionality = dimension

        if shape and not len(shape) == dimension:
            raise ValueError(
                f"Array dimensionality({dimension}) does not match the shape({shape})."
            )
        self.shape = shape

    def __eq__(self, o: Buffer):
        self_ = (self.name, self.namespace, self.loc, self.dimensionality,
                 self.shape)
        other_ = (o.name, o.namespace, o.loc, o.dimensionality, o.shape)
        return self_ == other_



class PhyControl:
    def __init__(self, predicate, body):
        self.predicate = None
        self.body = list()


class PhyIf(PhyControl, Expr):
    def __init__(self, predicate, body, orelse):
        super().__init__(predicate, body)
        self.else_block = orelse


class PhyFor(PhyControl, Expr):
    def __init__(self):
        super().__init__()
        self.init = list()
        self.iter = None


class Function(Expr):
    def __init__(self, name: str, namespace, lineno: int, col_offset: int):
        super().__init__(name, namespace, lineno, col_offset)
        self.arg_list = list()
        self.body = OrderedDict()
        self.returns = OrderedDict()

    def add_arg(self, argument):
        self.arg_list.append(argument)

    def add_args(self, arguments: List):
        self.arg_list.extend(arguments)

    def insert_arg(self, argument, position):
        self.arg_list.insert(position, argument)

    def prepend_arg(self, argument):
        self.insert_arg(argument, 0)

    def get_arguments(self):
        return self.arg_list

    def add_statement(self, statement: Expr) -> None:
        self.body.append(statement)

    def add_return(self, statement: Expr) -> None:
        self.returns.append(statement)

    def get_statements(self):
        return self.body

    def gen_code(self):
        fn_name = self.name
        args = ', '.join(arg.gen_code() for arg in self.arg_list)
        new_line = '\n'
        body = f"block({f',{new_line}'.join(b.gen_code() for b in self.body)})"
        return f"define({fn_name, args, body})"

    def __eq__(self, o: Function) -> bool:
        self_ = (self.name, self.namespace, self.loc, self.arg_list)
        other_ = (o.name, o.namespace, o.loc, o.args_list)
        return self_ == other_


class FunctionCall(Function):
    def __init__(self, name: str, args, namespace, lineno: int,
                 col_offset: int):
        super().__init__(name, namespace, lineno, col_offset)
        self.args = args


class Statement:
    def __init__(self, targets: List, value: Any) -> None:
        self.targets = targets
        self.value = value

    def gen_code(self):
        return

class Variable:
    pass