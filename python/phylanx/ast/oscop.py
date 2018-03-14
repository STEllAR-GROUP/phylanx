import re
import ast
import inspect
import numpy as np
from string import Template
from itertools import chain

from .templates import *
from .utils import dump_ast


class OpenSCoP:
    def __init__(self, root, kwargs):
        print(dump_ast(root))

        self.domain_stack = []
        self.params_stack = []
        self.memory_stack = []
        self.scatter_info = {}
        self.domain_set = set()
        self.params_set = set()
        self.memory_set = set()

        self.openscop = ''
        self.access = ''
        self.statements = []
        self.statement_idx = 0
        self.oscop_global = empty_oscop_global()

        kernel = root.body[0]
        for node in kernel.body:
            self.function[node.__class__.__name__](self, node)

        self.oscop_global['num_statements'] = self.statement_idx
        self.template = Template(oscop_global)
        self.openscop = self.template.substitute(
            self.oscop_global) + '\n' + self.openscop

        print(self.openscop)

    def _Add(self, node, expr={}, coef=1):
        left = node.left
        right = node.right
        self.function[left.__class__.__name__](self, left, expr, coef)
        if isinstance(left, ast.Subscript):
            self.memory_stack.append(expr)
            self.fill_access(self.statements[-1], 'READ', expr)

        if isinstance(right, ast.Subscript):
            expr = {}
        self.function[right.__class__.__name__](self, right, expr, coef)
        if isinstance(right, ast.Subscript):
            self.memory_stack.append(expr)
            self.fill_access(self.statements[-1], 'READ', expr)

        return

    def _Assign(self, node, expr={}, coef=1):
        self.statements.append(empty_oscop_statment())
        statement = self.statements[-1]
        self.statement_idx += 1
        statement['statement_num'] = self.statement_idx
        self.fill_domain(statement)
        expr = {}
        rhs = node.value
        self.function[rhs.__class__.__name__](self, rhs, expr, coef)
        if isinstance(rhs, ast.Subscript):
            self.memory_stack.append(expr)
            self.fill_access(self.statements[-1], 'READ', expr)
        expr = {}
        lhs = node.targets
        if len(lhs) != 1:
            raise Exception("Assignments may only have one target.")
        lhs = lhs[0]
        self.function[lhs.__class__.__name__](self, lhs, expr, coef)
        self.memory_stack.append(expr)
        self.fill_access(self.statements[-1], 'WRITE', expr)
        self.fill_scatter(statement)
        self.template = Template(oscop_statment)
        self.openscop += self.template.substitute(statement)
        self.access = ''
        self.memory_stack = []

        return

    def _BinOp(self, node, expr={}, coef=1):
        op = node.op.__class__.__name__
        self.function[op](self, node, expr, coef)
        return

    def _Call(self, node, expr={}, coef=1):
        for arg in node.args:
            self.function[arg.__class__.__name__](self, arg, expr, coef)
        return

    def _Compare(self, node, expr={}, coef=1):
        left = node.left
        right = node.comparators[0]
        ineq = node.ops[0].__class__.__name__
        lhs_sign, expr['literals'] = self.equalities[ineq]
        rhs_sign = -lhs_sign
        self.function[left.__class__.__name__](self, left, expr, lhs_sign)
        self.function[right.__class__.__name__](self, right, expr, rhs_sign)
        return

    def _Expr(self, node, expr={}, coef=1):
        self.function[node.value.__class__.__name__](self, node.value, expr,
                                                     coef)
        return

    def _For(self, node, expr={}, coef=1):
        # iteration domain
        iters = []
        iterator = node.target.id
        self.scatter_info[iterator] = 0
        self.domain_set.add(iterator)
        if len(node.iter.args) == 1:
            # lower
            expr = {}
            lower = 0
            expr[iterator] = 1
            expr['literals'] = 0
            iters.append(expr)
            # upper
            expr = {}
            upper = node.iter.args[0]
            expr[iterator] = -1
            expr['literals'] = -1
            self.function[upper.__class__.__name__](self, upper, expr, 1)
            iters.append(expr)
            for key in expr.keys():
                if not key in self.domain_set and key != 'literals':
                    self.params_set.add(key)
        else:
            # lower
            expr = {}
            lower = node.iter.args[0]
            expr[iterator] = 1
            expr['literals'] = 0
            self.function[lower.__class__.__name__](self, lower, expr, 1)
            iters.append(expr)
            # upper
            expr = {}
            upper = node.iter.args[1]
            expr[iterator] = -1
            expr['literals'] = -1
            self.function[upper.__class__.__name__](self, upper, expr, 1)
            iters.append(expr)
            for key in expr.keys():
                if not key in self.domain_set and key != 'literals':
                    self.params_set.add(key)
        # Body
        self.domain_stack.append(iters)
        expr = {}
        for n in node.body:
            if isinstance(n, ast.Pass):
                return
            node_name = n.__class__.__name__
            if node_name in self.function:
                self.function[node_name](self, n, expr, coef)
            else:
                raise NotImplementedError
        self.domain_stack.pop()
        self.scatter_info.pop(iterator, None)
        return

    def _If(self, node, expr={}, coef=1):
        condition = []
        # If the `if` statement includes more than one comparison.
        if isinstance(node.test, ast.BoolOp):
            raise NotImplementedError(
                "`if` statements may only include one "
                "`ast.Compare` expression. You may want to break down this "
                "statement into multiple `if` statements.")
        elif isinstance(node.test, ast.Num):
            if node.test.n:
                self.function[node.body.__class__.__name__](self, node.body)
            else:
                return
        # If the condition is just a variable.
        elif isinstance(node.test, ast.Name):
            _id = node.test.id
            if not _id in self.iters:
                self.params_set.add(_id)
            subexpr = {}
            subexpr[_id] = 1
            subexpr['literals'] = -1
            condition.append(subexpr)
            subexpr = {}
            subexpr[_id] = -1
            subexpr['literals'] = 1
            condition.append(subexpr)
        elif isinstance(node.test, ast.Compare):
            self._Compare(node.test, expr, coef)
            condition.append(expr)
        else:
            raise NotImplementedError

        self.fill_global(expr)

        for n in node.body:
            expr = {}
            if isinstance(n, ast.Pass):
                return
            node_name = n.__class__.__name__
            if node_name in self.function:
                self.function[node_name](self, n, expr, coef)
            else:
                raise NotImplementedError
        self.params_stack.pop()
        return

    def _Index(self, node, expr={}, coef=1):
        self.function[node.value.__class__.__name__](self, node.value, expr,
                                                     coef)
        return

    def _Mult(self, node, expr={}, coef=1):
        left_operand = node.left
        right_operand = node.right
        if isinstance(left_operand, ast.Num):
            coef = coef * left_operand.n
            operand = right_operand
            self.function[operand.__class__.__name__](self, operand, expr,
                                                      coef)
            return
        elif isinstance(right_operand, ast.Num):
            coef = coef * right_operand.n
            operand = left_operand
            self.function[operand.__class__.__name__](self, operand, expr,
                                                      coef)
            return
        else:
            self.function[left_operand.__class__.__name__](self, left_operand,
                                                           expr, coef)
            self.function[right_operand.__class__.__name__](
                self, right_operand, expr, coef)

        return

    def _Name(self, node, expr={}, coef=1):
        if node.id in expr:
            expr[node.id] += coef
        else:
            expr[node.id] = coef
        return

    def _Num(self, node, expr={}, coef=1):
        if 'literals' in expr:
            expr['literals'] += coef * node.n
        else:
            expr['literals'] = coef * node.n
        return

    def _Sub(self, node, expr={}, coef=1):
        left = node.left
        right = node.right
        self.function[left.__class__.__name__](self, left, expr, coef)
        if isinstance(left, ast.Subscript):
            self.memory_stack.append(expr)
            self.fill_access(self.statements[-1], 'READ', expr)

        if isinstance(right, ast.Subscript):
            expr = {}
        self.function[right.__class__.__name__](self, right, expr, -coef)
        if isinstance(right, ast.Subscript):
            self.memory_stack.append(expr)
            self.fill_access(self.statements[-1], 'READ', expr)
        return

    def _Subscript(self, node, expr={}, coef=1):
        if isinstance(node.value, ast.Name):
            self.memory_set.add(node.value.id)
        self.function[node.value.__class__.__name__](self, node.value, expr,
                                                     coef)
        self.function[node.slice.__class__.__name__](self, node.slice, expr,
                                                     coef)
        if isinstance(node.slice.value, ast.Name):
            _idx = node.slice.value.id
            if not _idx in self.scatter_info.keys():
                raise Exception(
                    "Index %s is not define in this scope.'" % _idx)
            self.scatter_info[_idx] += 1

        return

    def _UnaryOp(self, node, expr={}, coef=1):
        if isinstance(node.op, ast.USub):
            coef = -coef
            operand = node.operand
            self.function[operand.__class__.__name__](self, operand, expr,
                                                      coef)
        else:
            raise NotImplementedError
        return

    # source: http://bit.ly/2FFcHxj
    def list_len(self, nested_list):
        if type(nested_list) == list:
            return sum(self.list_len(sub_list) for sub_list in nested_list)
        else:
            return 1

    def fill_global(self, expr):
        self.params_stack.append([expr])
        self.oscop_global['domain_t'] = 'CONTEXT'
        self.oscop_global['params_exist'] = 1
        self.oscop_global['context'] += '## ' + str(expr) + '\n'
        for param in expr.keys():
            if param != 'literals':
                if not param in self.oscop_global['params_names']:
                    self.oscop_global['params_names'] += param + ' '
                    self.oscop_global['nrows'] += 1
                self.params_set.add(param)
        self.oscop_global['ncols'] = len(self.params_set) + 2

    def fill_domain(self, statement):
        offset = 1
        domain = list(self.domain_set)
        params = list(self.params_set)
        keys = domain + params
        idx_table = {'e/i': 0}
        for k in keys:
            idx_table[k] = offset
            offset += 1
        idx_table['literals'] = offset
        statement['domain_t'] = "DOMAIN"
        nrows = self.list_len(self.domain_stack)
        statement['num_relations'] = nrows
        ncols = len(self.domain_set) + len(self.params_set) + 2
        domain = np.zeros((nrows, ncols))
        domain[:, 0] = 1
        row = 0
        for d in self.domain_stack:
            for entry in d:
                for k in entry.keys():
                    domain[row][idx_table[k]] = entry[k]
                row += 1
        domain_string = ''
        flatten_domain = list(chain.from_iterable(self.domain_stack))
        for i, r in enumerate(domain):
            domain_string += ' '.join(str(int(e))
                                      for e in r) + "    ## " + str(
                                          flatten_domain[i]) + '\n'
        statement['domain'] = domain_string

    def fill_scatter(self, statement):
        statement['scattering'] = "SCATTERING\n"
        nscatterings = len(self.memory_stack)
        offset = 1 + nscatterings
        domain = list(self.domain_set)
        params = list(self.params_set)
        keys = domain + params
        idx_table = {'e/i': 0}
        for k in keys:
            idx_table[k] = offset
            offset += 1
        idx_table['literals'] = offset
        nrows = nscatterings
        ncols = len(keys) + nscatterings + 2
        domain = np.zeros((nrows, ncols))
        for i in range(nrows):
            domain[i][i + 1] = -1
        row = 0
        for m in self.memory_stack:
            for k in m.keys():
                if k in idx_table:
                    domain[row][idx_table[k]] = m[k]
            row += 1

        mem_string = ''
        for i, r in enumerate(domain):
            mem_string += ' '.join(str(int(e)) for e in r) + '\n'
        statement['scattering'] += mem_string

    def fill_access(self, statement, r_w, expr):
        self.access += '\n' + r_w + '\n'
        nrows = len(expr)
        offset = 1 + nrows
        domain = list(self.domain_set)
        params = list(self.params_set)
        keys = domain + params
        idx_table = {'e/i': 0}
        for k in keys:
            idx_table[k] = offset
            offset += 1
        idx_table['literals'] = offset
        ncols = len(keys) + nrows + 2
        domain = np.zeros((nrows, ncols))
        for i in range(nrows):
            domain[i][i + 1] = -1
        mems = list(self.memory_set)
        for k in expr.keys():
            if k in mems:
                domain[0][-1] = mems.index(k) + 1

        row = 1
        for k in expr.keys():
            if not k in mems:
                domain[row][idx_table[k]] = expr[k]
                row += 1

        mem_string = ''
        for i, r in enumerate(domain):
            mem_string += ' '.join(str(int(e)) for e in r) + '\n'
        self.access += mem_string
        statement['access'] = self.access

    # (sign, adjustment)
    equalities = {'Gt': (1, -1), 'GtE': (1, 0), 'Lt': (-1, 1), 'LtE': (-1, 0)}
    operators = {'USub': -1, 'Add': 1, 'Mult': 1, 'Sub': -1}
    function = {
        "Num": _Num,
        "Name": _Name,
        "Expr": _Expr,
        "Subscript": _Subscript,
        "BinOp": _BinOp,
        "Call": _Call,
        "Assign": _Assign,
        "Add": _Add,
        "Sub": _Sub,
        "Mult": _Mult,
        "If": _If,
        "Index": _Index,
        "Compare": _Compare,
        "UnaryOp": _UnaryOp,
        "For": _For
    }
