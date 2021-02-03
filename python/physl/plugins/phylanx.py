from __future__ import absolute_import
from __future__ import annotations
from physl.symbol_table import Namespace, SymbolTable

__license__ = """ 
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast
import inspect

from abc import ABC
from collections import namedtuple
from typing import Any

from physl.control import Task
from physl.symbol_table import SymbolTable
from physl.transformations import Transpiler


class PhyExpr(ABC):
    def __init__(self,
                 name: str,
                 namespace,
                 lineno: int = None,
                 col_offset: int = None):
        self.name = name
        self.namespace = namespace
        self.lineno = lineno
        self.col_offset = col_offset

    def namespace_str(self):
        return '+'.join(self.namespace)


class PhyVariable(PhyExpr):
    def __init__(self,
                 name: str,
                 namespace,
                 lineno: int,
                 col_offset: int,
                 dtype=None):
        super().__init__(name, namespace, lineno, col_offset)
        self.type = dtype

    def __eq__(self, other):
        self_ = (self.name, self.namespace, self.lineno, self.col_offset,
                 self.type)
        other_ = (other.name, other.namespace, other.lineno, other.col_offset,
                  self.type)
        return self_ == other_


class PhyArg(PhyExpr):
    ...


class PhyArray(PhyVariable):
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

    def __eq__(self, other):
        self_ = (self.name, self.namespace, self.lineno, self.col_offset,
                 self.dimensionality, self.shape)
        other_ = (other.name, other.namespace, other.lineno, other.col_offset,
                  other.dimensionality, other.shape)
        return self_ == other_


class PhyFn(PhyExpr):
    def __init__(self, name: str, namespace, lineno: int, col_offset: int):
        super().__init__(name, namespace, lineno, col_offset)
        self.arg_list = list()
        self.body = list()
        self.returns = list()

    def add_arg(self, argument):
        self.arg_list.append(argument)

    def add_args(self, arguments):
        self.arg_list.extend(arguments)

    def insert_arg(self, argument, position):
        self.arg_list.insert(position, argument)

    def prepend_arg(self, argument):
        self.insert_arg(argument, 0)

    def get_arguments(self):
        return self.arg_list

    def add_statement(self, statement: PhyExpr) -> None:
        self.body.append(statement)

    def __eq__(self, other):
        self_ = (self.name, self.namespace, self.lineno, self.col_offset,
                 self.arg_list)
        other_ = (other.name, other.namespace, other.lineno, other.col_offset,
                  other.args_list)
        return self_ == other_


class PhyFnCall(PhyFn):
    pass


class PhySL(Transpiler):
    def transpile(self, node):
        self.target = super().walk(node)

    def visit_arg(self, node: ast.arg) -> Any:
        namespace = Namespace.get_namespace()
        arg_name = node.arg
        return PhyArg(arg_name, namespace, node.col_offset, node.col_offset)

    def visit_arguments(self, node: ast.arguments) -> Any:
        args = [self.walk(n) for n in node.args]
        return args

    def visit_Assign(self, node: ast.Assign) -> Any:
        targets = [self.walk(target) for target in node.targets]
        value = self.walk(node.value)
        return super().visit_Assign(node)

    def visit_Constant(self, node: ast.Constant) -> Any:
        return node.value

    def visit_FunctionDef(self, node: ast.FunctionDef) -> Any:
        fn = self.get_fn()
        fn_name = node.name

        with Namespace(fn_name) as ns:
            namespace = ns.get_compelete_namespace()
            phy_fn = PhyFn(fn_name, namespace, node.lineno, node.col_offset)
            args = self.walk(node.args)
            for arg in args:
                phy_fn.add_arg(arg)

        [phy_fn.add_statement(self.walk(statement)) for statement in node.body]

        fn_args_specs = inspect.getfullargspec(fn)
        fn_params = fn_args_specs.args

    def visit_Module(self, node: ast.Module) -> Any:
        fn = self.get_fn()
        module_name = "__phy_module+" + inspect.getfile(fn).split(
            '/')[-1].split('.')[0]

        with Namespace(module_name):
            return self.walk(node.body[0])

    def visit_Name(self, node: ast.Name) -> Any:
        return (
            node.id,
            node.ctx,
            node._attributes,
        )

    def __call__(self, *args: Any, **kwargs: Any) -> Any:
        return self.task(*args, **kwargs)
