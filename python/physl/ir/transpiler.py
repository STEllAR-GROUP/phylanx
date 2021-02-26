from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast
import inspect

from abc import ABC, abstractmethod
from typing import Any, List

from physl.control import Task
from physl.ir.nodes import Argument, Attribute, Buffer, Expr
from physl.ir.nodes import Function, FunctionCall, Statement, Variable
from physl.ir.symbol_table import FnTable, Namespace, SymbolTable, TaskTable


class Transpiler(ABC, ast.NodeVisitor):
    def __init__(self, task: Task) -> None:
        self.task = task
        self.fn = task.fn
        # self.symbol_table = TaskTable(self.task)
        self.src = self.transpile(task.py_ast)

    @abstractmethod
    def transpile(self, node):
        ...

    @abstractmethod
    def __str__(self) -> str:
        ...

    def get_src(self):
        return str(self.src)

    def walk(self, node):
        target = self.visit(node)
        if target is None:
            raise NotImplementedError(
                f"Transformation rule for {node} is not implemented.")
        else:
            return target


class IR(Transpiler):
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

        return Statement(targets, value)

    def visit_Call(self, node: ast.Call) -> Any:
        fn_name = self.walk(node.func)
        fn_args = [self.walk(arg) for arg in node.args]
        namespace = Namespace.get_namespace()
        return FunctionCall(fn_name, fn_args, namespace, node.lineno,
                         node.col_offset)

    def visit_Compare(self, node: ast.Compare) -> Any:
        ops_ = {
            ast.Eq: '__eq',
            ast.NotEq: '__ne',
            ast.Lt: '__lt',
            ast.LtE: '__le',
            ast.Gt: '__gt',
            ast.GtE: '__ge',
            ast.Is: NotImplementedError(f"Phylanx: {ast.Is}"),
            ast.IsNot: NotImplementedError(f"Phylanx: {ast.IsNot}"),
            ast.In: NotImplementedError(f"Phylanx: {ast.In}"),
            ast.NotIn: NotImplementedError(f"Phylanx: {ast.NotIn}")
        }
        left = self.walk(node.left)
        comparators = [self.walk(c) for c in node.comparators]
        ops = [ops_[type(op)] for op in node.ops]
        for op in ops:
            if not isinstance(op, str):
                raise op

        # TODO
        if len(ops) > 1:
            raise NotImplementedError(
                "Multi-target assignment is not supported")

        return PhyCompare(left, ops[0], comparators[0])

    def visit_Constant(self, node: ast.Constant) -> Any:
        return node.value

    def visit_Expr(self, node: ast.Expr) -> Any:
        return self.walk(node.value)

    def visit_FunctionDef(self, node: ast.FunctionDef) -> Any:
        fn_name = node.name

        with Namespace(fn_name) as ns:
            fn = Function(fn_name, ns, node.lineno, node.col_offset)
            args = self.walk(node.args)
            for arg in args:
                fn.add_arg(arg)

            for statement in node.body:
                phy_statement = self.walk(statement)
                fn.add_statement(phy_statement)
                if isinstance(node.body, ast.Return):
                    fn.add_return(phy_statement)

        return fn

    def visit_If(self, node: ast.If) -> Any:
        predicate = self.walk(node.test)
        body = [self.walk(b) for b in node.body]
        orelse = [self.walk(o) for o in node.orelse]

        return PhyIf(predicate, body, orelse)

    def visit_Module(self, node: ast.Module) -> Any:
        file_name = inspect.getfile(self.fn).split('/')[-1].split('.')[0]
        module_name = "__phy_task+" + file_name

        with Namespace(module_name):
            return self.walk(node.body[0])

    def visit_Name(self, node: ast.Name) -> Any:
        return Variable(node.id, Namespace.get_namespace(), node.lineno,
                        node.col_offset)

    def visit_Return(self, node: ast.Return) -> Any:
        return self.walk(node.value)

    def __str__(self) -> str:
        return self.target.gen_code()

    def __call__(self, *args: Any, **kwargs: Any) -> Any:
        return self.task(*args, **kwargs)
