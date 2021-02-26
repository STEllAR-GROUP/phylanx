from __future__ import absolute_import
from __future__ import annotations

__license__ = """ 
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast
import inspect

from abc import ABC
from typing import Any, List

from physl.ir.transpiler import Transpiler
from physl.ir.symbol_table import Namespace, SymbolTable


class PhyLoc:
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

    def __eq__(self, o: PhyLoc) -> bool:
        return (self.lineno, self.col_offset) == (o.lineno, o.col_offset)


class PhyExpr(ABC):
    def __init__(self,
                 name: str,
                 namespace,
                 lineno: int = None,
                 col_offset: int = None):
        self.name = name
        self.namespace = namespace
        self.loc = PhyLoc(lineno, col_offset)

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

    def __eq__(self, o: PhyVariable):
        self_ = (self.name, self.namespace, self.loc)
        other_ = (o.name, o.namespace, o.lineno, o.loc)
        return self_ == other_


class PhyArg(PhyExpr):
    def gen_code(self):
        return f'{self.name}{self.loc.gen_code()}'


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

    def __eq__(self, o: PhyArray):
        self_ = (self.name, self.namespace, self.loc, self.dimensionality,
                 self.shape)
        other_ = (o.name, o.namespace, o.loc, o.dimensionality, o.shape)
        return self_ == other_


class PhyCompare:
    def __init__(self, left, op, right) -> None:
        self.left = left
        self.op = op
        self.right = right


class PhyControl:
    def __init__(self, predicate, body):
        self.predicate = None
        self.body = list()


class PhyIf(PhyControl, PhyExpr):
    def __init__(self, predicate, body, orelse):
        super().__init__(predicate, body)
        self.else_block = orelse


class PhyFor(PhyControl, PhyExpr):
    def __init__(self):
        super().__init__()
        self.init = list()
        self.iter = None


class PhyFn(PhyExpr):
    def __init__(self, name: str, namespace, lineno: int, col_offset: int):
        super().__init__(name, namespace, lineno, col_offset)
        self.arg_list = list()
        self.body = list()
        self.returns = list()

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

    def add_statement(self, statement: PhyExpr) -> None:
        self.body.append(statement)

    def add_return(self, statement: PhyExpr) -> None:
        self.returns.append(statement)

    def get_statements(self):
        return self.body

    def gen_code(self):
        fn_name = self.name
        args = ', '.join(arg.gen_code() for arg in self.arg_list)
        new_line = '\n'
        body = f"block({f',{new_line}'.join(b.gen_code() for b in self.body)})"
        return f"define({fn_name, args, body})"

    def __eq__(self, o: PhyFn) -> bool:
        self_ = (self.name, self.namespace, self.loc, self.arg_list)
        other_ = (o.name, o.namespace, o.loc, o.args_list)
        return self_ == other_


class PhyFnCall(PhyFn):
    def __init__(self, name: str, args, namespace, lineno: int,
                 col_offset: int):
        super().__init__(name, namespace, lineno, col_offset)
        self.args = args


class PhyStatement:
    def __init__(self, targets: List, value: Any) -> None:
        self.targets = targets
        self.value = value

    def gen_code(self):
        return


class PhylanxTranspiler(Transpiler):
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

        return PhyStatement(targets, value)

    def visit_Call(self, node: ast.Call) -> Any:
        fn_name = self.walk(node.func)
        fn_args = [self.walk(arg) for arg in node.args]
        namespace = Namespace.get_namespace()
        return PhyFnCall(fn_name, fn_args, namespace, node.lineno,
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
            namespace = ns.get_complete_namespace()
            phy_fn = PhyFn(fn_name, namespace, node.lineno, node.col_offset)
            args = self.walk(node.args)
            for arg in args:
                phy_fn.add_arg(arg)

            for statement in node.body:
                phy_statement = self.walk(statement)
                phy_fn.add_statement(phy_statement)
                if isinstance(node.body, ast.Return):
                    phy_fn.add_return(phy_statement)

        return phy_fn

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
        return PhyVariable(node.id, Namespace.get_namespace(), node.lineno,
                           node.col_offset)

    def visit_Return(self, node: ast.Return) -> Any:
        return self.walk(node.value)


class PhySL(PhylanxTranspiler):
    def __str__(self) -> str:
        return self.target.gen_code()

    def __call__(self, *args: Any, **kwargs: Any) -> Any:
        return self.task(*args, **kwargs)
