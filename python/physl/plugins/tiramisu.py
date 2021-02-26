from __future__ import absolute_import
from __future__ import annotations

__license__ = """
Copyright (c) 2021 R. Tohid (@rtohid)

Distributed under the Boost Software License, Version 1.0. (See accompanying
file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
"""

import ast
import inspect

from collections import OrderedDict, defaultdict
from typing import Any, Callable, List, Union

from physl.ir.symbol_table import Namespace, SymbolTable
from physl.ir.transpiler import IR, Transpiler
# from pytiramisu import buffer, computation, constant, expr, function
# from pytiramisu import init_physl, input, var
# from pytiramisu import uint32_expr





class Index:
    def __init__(self, name: str, index: List) -> None:
        self.name = name
        self.index = index

    def variables(self):
        vars = list()
        for entry in self.index:
            if not isinstance(entry, int):
                vars.append(entry)
        return vars


# PyTiramisu objects
# ---------------------------------------------------------------------------- #
class Buffer:
    def __init__(self,
                 name: str,
                 id: int,
                 indices: List,
                 context: Union[ast.Load, ast.Store] = None) -> None:
        """Represents memory buffers."""

        self.id = id
        self.name = name
        self.indices = OrderedDict()

        self._read_write = {'loads': OrderedDict(), 'stores': OrderedDict()}
        if context:
            self.set_context(name, id, context)

        self.isl = None

    def build(self):
        raise NotImplementedError

    def dimension(self):
        return len(self.indices)

    def loads(self):
        return self._read_write['loads']

    def get_context(self, name: str, id: int) -> Union[Load, Store]:
        return self._read_write[name][id]

    def set_context(self, name: str, id: int,
                    context: Union[ast.Load, ast.Store]) -> None:

        context: Union[ast.Load, ast.Store]
        if isinstance(context, ast.Load):
            self._read_write['loads'][name] = id
        else:
            self._read_write['stores'][name] = id

    def stores(self):
        return self.get_context()['stores']


class Call:
    stack = OrderedDict()

    def __init__(self, fn_name: str, args: Any, id: int, dtype=None):
        self.name = fn_name
        self.id = id
        self.dtype = dtype
        self.args = args
        self.isl = None

        call_stack = Call.stack
        if call_stack.get(fn_name):
            call_stack[fn_name].append(self)
        else:
            call_stack[fn_name] = [self]

    def build(self):
        print(self.args)
        # raise NotImplementedError


class Computation:
    statements = OrderedDict()

    def __init__(self, lhs=None, rhs=None, id=None):
        """A computation has an expression (class:`Expression`) and 
        iteration domain defined using an :class:`Iterator`."""
        self._lhs = lhs
        self._rhs = rhs
        self.id = id
        self.iter_domain = None
        self.name = 'S' + str(len(Computation.statements))
        self.isl = None
        Computation.statements[self.name] = self

    @property
    def lhs(self):
        return self._lhs

    @lhs.setter
    def lhs(self, targets):
        self._lhs = targets

    @property
    def rhs(self):
        return self._rhs

    @rhs.setter
    def rhs(self, expr):
        self._rhs = expr

    def build(self):
        rhs_ = Computation.statements[self.name].rhs
        lhs_ = Computation.statements[self.name].lhs
        if hasattr(rhs_, 'build'):
            rhs_.gen_code()
        elif isinstance(rhs_, int):
            value = Constant(rhs_)

        if hasattr(lhs_, 'build'): lhs_.gen_code()


class Constant:
    def __init__(self, name: str, value: int):
        """Designed to represent constants that are supposed to be declared at
        the beginning of a Tiramisu function. This can be used only to declare
        constant scalars."""
        self.name = name
        self.value = value


class Expr:
    tree = OrderedDict()

    def __init__(self, value: ast.Expr) -> None:
        """Represnets expressions, e.g., 4, 4 + 4, 4 * i, A[i, j], ..."""
        self.id = hash(value)
        self.value = value
        self.add_to_tree()

    def add_to_tree(self) -> None:
        Expr.tree[self.id] = self

    def build(self):
        raise NotImplementedError


class Function:
    defined = OrderedDict()

    def __init__(self, name: str, id: int, params: Any, dtype=None) -> None:
        """Equivalent to a function in C; composed of multiple computations and
        possibly Vars."""

        self.name = name
        self.dtype = dtype

        self.body = OrderedDict()

        self.num_returns = 0
        self.returns = OrderedDict()

    def add_statement(self, statement: Any, id: int):
        self.body[statement.id] = statement

    def add_return(self, return_val):
        self.num_returns += 1
        self.returns[self.num_returns] = return_val

    def build(self):
        # for arg in self.-+
        # init_physl(self.name)
        body_ = self.body
        for value in body_.values():
            value.gen_code()
            self.add_statement(value, value.id)
        for return_ in self.returns:
            if hasattr(return_, 'build'): return_.gen_code()

    def define(self):
        defined_ = Function.defined.get(self.name)
        self.id = self.task.id

        if defined_:
            Function.defined[self.name].append(self.id)
        else:
            Function.defined[self.name] = [self.id]


class Input:
    def __init__(self):
        """An input can represent a buffer or a scalar"""
        pass


class Return:
    returns = OrderedDict()

    def __init__(self, value, id) -> None:
        self.id = id
        self.value = value

    def build(self):
        print(self.id, self.value)

    @classmethod
    def add(cls, return_):
        Return.returns[return_.id] = return_

    def num_return(self):
        return len(Return.returns.keys())

    @classmethod
    def reset(cls):
        Return.returns = OrderedDict()


class Var:
    iters = OrderedDict()

    def __init__(self, id: int, iterator=None):
        """Defines the range of the loop around the computation (its iteration
        domain). When used to declare a buffer it defines the buffer size, and
        when used with an input it defines the input size."""

        self.id = id
        self.iterator = iterator
        self.bounds = {'lower': None, 'upper': None, 'stride': None}
        self.body = OrderedDict()

    def set_bounds(self, lower=None, upper=None, stride=1):
        if lower:
            self.bounds['lower'] = lower
        else:
            self.bounds['lower'] = 0

        self.bounds['upper'] = upper
        self.bounds['stride'] = stride

    def build(self):
        for statement in self.body.values():

            if hasattr(statement, 'build'):
                self.body[self.id] = statement.gen_code()


class PolytopeTranspiler(Transpiler):
    def transpile(self, node):
        self.target = super().walk(node)

    def visit_Add(self, node: ast.Add) -> str:
        return '__Add__'

    def visit_Assign(self, node: ast.Assign) -> Computation:
        id = hash(node)
        target = node.targets
        value = node.value

        rhs = self.walk(value)

        if 1 < (len(target)):
            raise NotImplementedError(
                "Multi-target assignments are not yet supported.")
        lhs = self.walk(target[0])
        s = Computation(lhs, rhs, id)

        return s

    def visit_Attribute(self, node: ast.Attribute) -> None:
        return node.attr

    def visit_BinOp(self, node: ast.BinOp) -> Call:
        fn_name = self.walk(node.op)

        lhs = self.walk(node.left)
        rhs = self.walk(node.right)
        args = [lhs, rhs]

        fn = Call(fn_name, hash(node), args)

        return fn

    def visit_Call(self, node: ast.Call) -> Call:
        if isinstance(node.func, str):
            fn_name = node.func
        else:
            fn_name = self.walk(node.func)

        if isinstance(node.args, list):
            args = [self.walk(arg) for arg in node.args]
        else:
            args = self.walk(node.args)

        if not isinstance(fn_name, str):
            raise TypeError(f"Expected {str}, received {fn_name}")

        fn = Call(fn_name, args, self.task.id, self.task.dtype)

        for attr in node.keywords:
            value = self.walk(attr.value)
            setattr(fn, attr.arg, value)

        return fn

    def visit_Constant(self, node: ast.Constant) -> Union[int, str]:
        return node.value

    def visit_Expr(self, node: ast.Expr) -> Expr:
        return Expr(self.walk(node.value))

    def visit_For(self, node: ast.For) -> Var:
        loop = Var(hash(node.target))
        loop.iterator = self.walk(node.target)

        if isinstance(node.iter, ast.Call) and 'range' == node.iter.func.id:
            bounds = self.walk(node.iter)
            iter_space = bounds.args

            if not isinstance(iter_space, type(None)):
                if 1 == len(iter_space):
                    loop.set_bounds(lower=0, upper=iter_space[0])
                if 2 == len(iter_space):
                    loop.set_bounds(lower=iter_space[0], upper=iter_space[1])
                if 3 == len(iter_space):
                    loop.set_bounds(lower=iter_space[0],
                                    upper=iter_space[1],
                                    stride=iter_space[2])

        if isinstance(node.iter, ast.List):
            raise TypeError(":class:`ast.List` might not be an affine space.")

        for statement in node.body:
            statement_isl = self.walk(statement)
            if not isinstance(statement_isl, str):
                loop.body[hash((statement))] = statement_isl

        return loop

    def visit_FunctionDef(self, node: ast.FunctionDef) -> Function:

        # Return.reset()
        parameters = inspect.getfullargspec(self.task.fn).args
        dtype = self.task.dtype
        fn = Function(node.name, self.task.id, parameters, dtype)

        for statement in node.body:
            if not isinstance(statement, str):
                if isinstance(statement, ast.Expr):
                    if isinstance(statement.value, ast.Constant):
                        continue
            else:
                continue
            visited_statement = self.walk(statement)
            if not isinstance(visited_statement, str):
                if isinstance(visited_statement, Return):
                    fn.add_return(visited_statement)
                fn.add_statement(visited_statement, fn.id)

        return fn

    def visit_Index(self, node: ast.Index) -> Any:
        return self.walk(node.value)

    def visit_Module(self, node: ast.Module) -> Any:
        file_name = inspect.getfile(self.fn).split('/')[-1].split('.')[0]
        module_name = "__phy_task+" + file_name

        with Namespace(module_name):
            return self.walk(node.body[0])

    def visit_Mult(self, node: ast.Mult) -> str:
        return '__Mult__'

    def visit_Name(self, node: ast.Name) -> str:
        return node.id

    def visit_Return(self, node):
        return_ = Return(self.walk(node.value), hash(node))
        Return.add(return_)

        return return_

    def visit_Slice(self, node: ast.Slice) -> Any:
        lower = self.walk(node.lower)
        upper = self.walk(node.upper)
        step = self.walk(node.step)

        return (lower, upper, step)

    def visit_Subscript(self, node: ast.Subscript) -> tuple:
        def _NestedSubscript(node):
            value_slice = self.walk(node.value)
            slice_ = [self.walk(node.slice)]
            if isinstance(node.value, ast.Subscript):
                value_slice = _NestedSubscript(node.value)

            if isinstance(value_slice, list):
                value = value_slice + slice_
            else:
                value = [value_slice] + slice_

            return value

        slice_ = [self.walk(node.slice)]
        if isinstance(node.value, ast.Subscript):
            value_slice = _NestedSubscript(node.value)
        else:
            value_slice = self.walk(node.value)

        if isinstance(value_slice, list):
            value = value_slice + slice_
        else:
            value = [value_slice] + slice_
        print(value)
        return value

    def visit_Tuple(self, node: ast.Tuple) -> None:
        return tuple([self.walk(expr) for expr in node.elts])


class Polytope(IR):
    def __str__(self) -> str:
        self.target.gen_code()
        # self.isl_tree.gencode()

    # def isl_gencode(self):
    #     pass
    #     return self.target.gen_code()

    def __call__(self, *args, **kwargs):
        # self.isl_tree(*args, **kwargs)
        return self.task(*args, **kwargs)