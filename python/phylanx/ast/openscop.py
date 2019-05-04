#  Copyright (c) 2018 R. Tohid
#  Copyright (c) 2019 Ye Fang
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast
import numpy as np
# from itertools import chain
from string import Template
from .utils import dump_to_file
from .utils import save_to_file
from .utils import print_python_ast_node
from .utils import dump_ast as python_ast_format


class Oscop:
    def __init__(self, kwargs):
        """
        openscope elements
            global
                param                        p
                number of statements
            statements                                  Assign, AugAssign
                domain                       d          For, If
                scatter                      s          Assign, AugAssign
                access                       a          Assign, AugAssign

        relation
            UNDEFINED, CONTEXT, DOMAIN, SCATTERING, READ, WRITE

        example

        # DOMAIN
          3 7 3 0 0 2

        # num_rows=3,
        # num_cols=7,
        # num_output_dim=3,        i, j, k
        # num_input_dim=0,
        # num_local_dim=0,
        # num_params=2,            M, N
        # num_cols == num_output_dim + num_input_dim + num_local_dim + num_params + 2

        # ei i  j  k  M  N  1
          1 -1  0  0  1  0  0     # -i         + M     >= 0
          1  0 -1  0  0  1  0     #     -j         + N >= 0
          1  1  1 -1  0  0  0     #  i + j - k         >= 0

        """

        self.globalinfo = self.empty_global()  # data for global
        self.statements = []  # data for statements
        """
        stmt -> For
        """
        self.domain = []  # domain stack
        self.domain_iter = []  # domain stack, only iterators
        """
        stmt -> For                 self.scatter.append(iterator)
        stmt -> Assign              self.scatter.append(0), or ++
        stmt -> AugAssign
        """
        self.scatter = [0]  # eg: [0, "i", 1, "j", 0]
        """
        expr -> BinOp
        """

    def empty_global(self):
        d = dict(
            num_rows=0,
            num_cols=0,
            num_output_dim=0,
            num_input_dim=0,
            num_local_dim=0,
            num_params=0,

            num_statements=0,
            params=[],
            relation=[],)
        return d

    def empty_statment(self):
        d = dict(
            statement_id=None,
            num_relations=0,
            domain=[],
            scatter=[],
            access=[],)
        return d

    def check_iterator_in_domain(self, iterator):
        if iterator not in self.domain_iter:
            raise Exception( \
                "Index %s is not define in this scope." % iterator)

    def fill_params_to_globalinfo(self, expr):
        """
        Filling parameter data into "self.globalinfo".
        New parameter may exist in any expressions.
        New parameter constraint may exist in "If" expressions.
        """

        expr_set = set(expr.keys()) - set(["literals"])
        domain_set = set(self.domain_iter)

        # import inspect
        # print(inspect.stack())
        print("debug fill_param_to_globalinfo", expr)
        print("debug fill_param_to_globalinfo", expr_set)
        print("debug fill_param_to_globalinfo", domain_set)

        # Finding a new parameter constraint \
        # when the expr does not have domain constraints \
        # A.K.A the intersection is empty
        if (len(expr_set & domain_set) == 0):
            # data collected, to format in "generate_global" method
            self.globalinfo["relation"].append(expr)
            self.globalinfo["num_rows"] += 1

        # Finding a new parameter.
        for a in (expr_set - domain_set):
            if (a not in self.globalinfo["params"]):
                self.globalinfo["params"].append(a)

    def new_statement(self):
        """
        Creating an empty statement, and append to the "self.statements" list.
        This statement will be modified via methods:
            fill_domain_to_statement(self)
            fill_scatter_to_statement(self)
            fill_access_to_statement(self)
        """

        self.statements.append(self.empty_statment())
        statement = self.statements[-1]
        statement["statement_id"] = len(self.statements)

    def update_domain(self, op, domaininfo=None, iterator=None):
        print("debug update_domain", op, domaininfo, iterator)

        if (op == "enter"):
            self.domain.append(domaininfo)
            self.domain_iter.append(iterator)
        elif (op == "exit"):
            self.domain.pop()
            self.domain_iter.pop()
        else:
            raise Exception("Not supported")

    def update_scatter(self, op, iterator=None):

        # finding the last index of some value in a list
        def rindex(lst, value):
            for i, v in enumerate(reversed(lst)):
                if v == value:
                    return len(lst) - i - 1
            raise Exception("iterator not found.")

        if (len(self.scatter) <= 0):
            raise Exception("the scatter stack shall always have data")

        if (op == "enter"):
            if (iterator is None):  # entering a statement
                if (not isinstance(self.scatter[-1], int)):
                    self.scatter.append(0)
                else:
                    self.scatter[-1] = self.scatter[-1] + 1
            else:  # entering a loop
                # if (not isinstance(self.scatter[-1], int)):
                #   self.scatter.append(0)
                self.scatter.append(iterator)
                # BUG: need to handel multiple loops

        elif (op == "exit"):
            current_iter = self.domain_iter[-1]
            idx = rindex(self.scatter, current_iter)
            # print("xxxxxxxxxxxxxxx idx", idx)
            # print("xxxxxxxxxxxxxxx before", self.scatter)
            del self.scatter[idx:]
            # print("xxxxxxxxxxxxxxx after", self.scatter)
            # self.scatter[-1] = self.scatter[-1] + 1

        else:
            raise Exception("Not supported")

    def fill_domain_to_statement(self):
        statement = self.statements[-1]
        statement["domain"].append(self.domain)

    def fill_scatter_to_statement(self):
        statement = self.statements[-1]
        statement["scatter"] = self.scatter

    def fill_access_to_statement(self, rw, name, expr):
        statement = self.statements[-1]
        expr["rw"] = rw  # "read", "write"
        expr["name"] = name
        statement["access"].append(expr)

    def generate_oscop_global(self):
        self.globalinfo["num_output_dim"] = 0  # always 0 in CONTEXT relation
        self.globalinfo["num_input_dim"] = 0  # always 0 in CONTEXT relation
        self.globalinfo["num_local_dim"] = 0  # always 0 in CONTEXT relation
        self.globalinfo["num_params"] = len(self.globalinfo["params"])
        self.globalinfo["num_cols"] = \
            self.globalinfo["num_output_dim"] + \
            self.globalinfo["num_input_dim"] + \
            self.globalinfo["num_local_dim"] + \
            self.globalinfo["num_params"] + \
            2
        self.globalinfo["num_statements"] = len(self.statements)

        mydata = dict(
            relation="",
            params_exist="",
            params_names="",
            num_statements="",)

        s = "CONTEXT" + "\n"
        s += str(self.globalinfo["num_rows"]) + " "
        s += str(self.globalinfo["num_cols"]) + " "
        s += str(self.globalinfo["num_output_dim"]) + " "
        s += str(self.globalinfo["num_input_dim"]) + " "
        s += str(self.globalinfo["num_local_dim"]) + " "
        s += str(self.globalinfo["num_params"]) + "\n"
        for expr in self.globalinfo["relation"]:
            s += "#TOFORMAT" + str(expr) + "\n"  # TBD
        mydata["relation"] = s

        if self.globalinfo["num_params"] > 0:
            mydata["params_exist"] = "1"
        else:
            mydata["params_exist"] = "0"

        for a in self.globalinfo["params"]:
            mydata["params_names"] += a + " "

        mydata["num_statements"] = \
            str(self.globalinfo["num_statements"])

        template = """
<OpenScop>

# =============================================== Global
# Backend Language
C

# Context
$relation

# Parameter names are provided
$params_exist

# Parameter names
$params_names

# Number of statements
$num_statements

"""
        rv = Template(template).substitute(mydata)
        return rv

    def generate_domain(self, statement):
        # s = ""
        # for key in raw.keys():
        #     s += key + ": "
        #     s += raw[key] + "\n"

        print("gen domain", statement)
        print("gen domain", statement["domain"])

        rv = str(statement["domain"])
        return rv

    def generate_scatter(self, statement):
        print("gen scatter", statement)
        print("gen scatter", statement["scatter"])
        rv = str(statement["scatter"])
        return rv

    def generate_access(self, statement):
        print("gen access ", statement)
        print("gen access ", statement["access"])
        rv = str(statement["access"])
        return rv

    def generate_oscop_statement(self, statement):
        print(statement)

        mydata = dict(
            statement_id=str(statement["statement_id"]),
            num_relations=str(statement["num_relations"]),
            domain=self.generate_domain(statement),
            scatter=self.generate_scatter(statement),
            access=self.generate_access(statement),)

        print("debug generate_statements", statement)   # DEBUG
        print("debug generate_statements", mydata)   # DEBUG
        print("")  # DEBUG

        template = """
# =============================================== Statement $statement_id
# Number of relations describing the statement
$num_relations

# ----------------------------------------------  $statement_id.1 Domain
$domain

# ----------------------------------------------  $statement_id.2 Scattering
$scatter

# ----------------------------------------------  $statement_id.3 Access
$access

        """
        rv = Template(template).substitute(mydata)
        return rv

    def generate(self):
        code = ""
        code = code + self.generate_oscop_global()
        for statement in self.statements:
            code = code + self.generate_oscop_statement(statement)
        return code

    def print1(self):
        print("self.globalinfo: ", self.globalinfo)
        print("self.statements: ", self.statements)
        print("self.domain: ", self.domain)
        print("self.domain_iter: ", self.domain_iter)
        print("self.scatter: ", self.scatter)
        # print("")


class OpenSCoP:
    def __init__(self, func, python_ast, kwargs):
        """
        expr
            {}              default
            "literals"      integer literal, the weight of const "1"
            "Para"          parameter "Para"
            "Name"          name "Name"

        coef
            1               default
            -1
        mode (reserved for multi-pass AST traversal. now obsolete)
        """

        self.oscop = Oscop(kwargs)

        kernel = python_ast.body[0].body  # BUG, only allow 1 statement
        for node in kernel:
            self.visit(node, {}, 1, 0)
        # for node in kernel:
        #    self.visit(node, {}, 1, 1)

        print("############################")
        self.__src__ = self.oscop.generate()
        save_to_file(self.__src__, "ooo_openscop.txt", True)

        # dump_to_file(self.__src__, "dump_openscop", kwargs)
        aststr = python_ast_format(python_ast)
        save_to_file(aststr, "ooo_pythonast.txt", True)

    # help function
    def debug_dev(self, node, expr, coef, mode):
        if True:
            print("############################")
            self.oscop.print1()
        if True:
            print("expr: ", expr)
            print("coef: ", coef)
            print("mode: ", mode)
            print("")
        if True:
            print_python_ast_node("node", node)
            print("")

    def visit(self, node, expr, coef, mode):
        """
        Calls the corresponding rule, based on the name of the node.
        eg:
            Add -> self._Add
        """

        if not isinstance(node, ast.AST):
            raise Exception("The type is not \"python AST node\"")

        self.debug_dev(node, expr, coef, mode)  # debug

        f = eval("self._%s" % node.__class__.__name__)
        return f(node, expr, coef, mode)

    # python AST grammar reference
    # https://docs.python.org/3/library/ast.html

    ###############################################################
    # stmt
    ###############################################################

    def _Assign(self, node, expr, coef, mode):
        """
        stmt -> Assign(expr* targets, expr value)
        """

        if len(node.targets) > 1:
            raise Exception("Does not support chain assignments.")
        if isinstance(node.targets[0], ast.Tuple):
            raise Exception("Does not support multi-target assignments.")

        self.oscop.new_statement()
        self.oscop.update_scatter("enter")
        self.oscop.fill_domain_to_statement()
        self.oscop.fill_scatter_to_statement()

        # BUGGY:    lhs.value.id only applis to B[i]
        # B[i][j] is wrong

        lhs = node.targets[0]  # supports only one target
        if isinstance(lhs, ast.Subscript):
            expr = {}
            self.visit(lhs, expr, coef, mode)  # generating expr
            self.oscop.fill_access_to_statement("write", lhs.value.id, expr)

        rhs = node.value
        if isinstance(rhs, ast.Subscript):
            expr = {}
            self.visit(rhs, expr, coef, mode)  # generating expr
            self.oscop.fill_access_to_statement("read", rhs.value.id, expr)
            # The expr is buggy

        if isinstance(rhs, ast.UnaryOp):
            # TBD
            pass

        if isinstance(rhs, ast.BinOp):
            # TBD
            pass

        return

    def _For(self, node, expr, coef, mode):
        """
        stmt -> For(expr target, expr iter, stmt* body, stmt* orelse)
        """

        # supported formats
        #
        #  for i in range(M, N):
        #       For(
        #           target=Name(id='i', ctx=Store()),
        #           iter=Call(
        #               func=Name(id='range', ctx=Load()),
        #               args=[
        #                   Name(id='M', ctx=Load()),
        #                   Name(id='N', ctx=Load()),
        #               ],
        #               keywords=[],
        #               ),
        #           body=[...]
        #           orelse=[],
        #           ),
        #
        #  for i in range(1, 9):
        #       For(
        #           target=Name(id='i', ctx=Store()),
        #           iter=Call(
        #               func=Name(id='range', ctx=Load()),
        #               args=[
        #                   Num(n=1),
        #                   Num(n=9),
        #               ],
        #               keywords=[],
        #               ),
        #           body=[...]
        #           orelse=[],
        #           ),
        #
        #  for i in range(N):
        #       For(
        #           target=Name(id='i', ctx=Store()),
        #           iter=Call(
        #               func=Name(id='range', ctx=Load()),
        #               args=[Name(id='N', ctx=Load())],
        #               keywords=[],
        #               ),
        #           body=[...]
        #           orelse=[],
        #           ),
        #
        #
        # unsupported formats
        #
        #  for i in listi:
        #       For(
        #           target=Name(id='i', ctx=Store()),
        #           iter=Name(id='listi', ctx=Load()),
        #           body=[...]
        #           orelse=[],
        #           ),
        #

        def _For_exception(self):
            raise Exception("Only support FOR loop in format\
                    for i in range(N)\
                    for i in range(M, N)\
                    Where M/N is \"number\" type or \"parameter\" type.\
                    ")

        mydomain = []
        iterator = node.target.id

        if not isinstance(node.iter, ast.Call):  # call something
            self._For_exception()

        if (node.iter.func.id != "range"):       # call range function
            self._For_exception()

        bounds = node.iter.args
        if not ((len(bounds) == 1) or (len(bounds) == 2)):  # 1 arg or 2 args
            self._For_exception()

        # lower bound
        expr_l = {}
        expr_l[iterator] = 1
        expr_l["literals"] = 0
        if (len(bounds) == 1):
            lb = 0  # debug, not used
        if (len(bounds) == 2):
            lb = node.iter.args[0]
            self.visit(lb, expr_l, 1, mode)
        mydomain.append(expr_l)  # collecting domain data

        # uppwer bound
        expr_u = {}
        expr_u[iterator] = -1
        expr_u['literals'] = -1
        ub = node.iter.args[-1]  # the last argment passed to range()
        self.visit(ub, expr_u, 1, mode)
        mydomain.append(expr_u)  # collecting domain data

        self.oscop.update_domain("enter", mydomain, iterator)  # entering fist
        self.oscop.update_scatter("enter", iterator)
        self.oscop.fill_params_to_globalinfo(expr_l)
        self.oscop.fill_params_to_globalinfo(expr_u)

        for n in node.body:
            self.visit(n, {}, coef, mode)

        self.oscop.update_scatter("exit")
        self.oscop.update_domain("exit")  # existing last
        return

    def _If(self, node, expr, coef, mode):
        """
        stmt -> If(expr test, stmt* body, stmt* orelse)
        """

        if (len(node.orelse) != 0):  # if ...: ... else: ...
            raise NotImplementedError(
                "`if` statements should not include `else` branch")

        elif isinstance(node.test, ast.BoolOp):  # if (A and B): ...
            raise NotImplementedError(
                "`if` statements may only include one expression. \
                You may want to break down this \
                statement into multiple `if` statements.")

        elif isinstance(node.test, ast.Name):  # if N: ...
            raise NotImplementedError(
                "`if %s` is not supported" % node.test.id)

        elif isinstance(node.test, ast.Num):  # if 4: ...
            if not node.test.n:  # short circuiting
                return

        elif isinstance(node.test, ast.Compare):  # if (0 < N): ...
            self.visit(node.test, expr, coef, mode)
            self.oscop.fill_params_to_globalinfo(expr)

        else:
            raise NotImplementedError

        for n in node.body:
            self.visit(n, {}, coef, mode)

        return

    def _Pass(self, node, expr, coef, mode):
        """
        stmt -> Pass
        """

        return

    ###############################################################
    # expr
    ###############################################################

    # Level mixed, bad !!!!
    def _BinOp(self, node, expr, coef, mode):
        """
        expr -> BinOp(expr left, operator op, expr right)
        """

        if (node.op.__class__.__name__ == "Add"):
            self._AddSub(node, expr, coef, mode)
        if (node.op.__class__.__name__ == "Sub"):
            self._AddSub(node, expr, coef, mode)
        if (node.op.__class__.__name__ == "Mult"):
            self._Mult(node, expr, coef, mode)
        return

    def _UnaryOp(self, node, expr, coef, mode):
        """
        expr -> UnaryOp(unaryop op, expr operand)
        """

        if isinstance(node.op, ast.USub):
            coef = -coef
            self.visit(node.operand, expr, coef, mode)
        else:
            raise NotImplementedError
        return

    def _Compare(self, node, expr, coef, mode):
        """
        expr -> Compare(expr left, cmpop* ops, expr* comparators)
        """

        if (len(node.ops) != 1):  # eg: 2 < N < 5
            raise NotImplementedError("only allow one op")
        if (len(node.comparators) != 1):  # eg: 2 < N < 5
            raise NotImplementedError("only allow one comparator")

        left = node.left
        ops = node.ops[0]
        right = node.comparators[0]
        my_cmpop = {
            "Lt": (1, -1),
            "LtE": (0, -1),
            "Gt": (-1, 1),
            "GtE": (0, 1),
        }
        expr["literals"], coef = my_cmpop[ops.__class__.__name__]
        self.visit(left, expr, coef, mode)
        self.visit(right, expr, -1 * coef, mode)
        return

    def _Call(self, node, expr, coef, mode):
        """
        expr -> Call(expr func, expr* args, keyword* keywords)
        """

        # debug, should not support call !!!!!
        for arg in node.args:
            self.visit(self, arg, expr, coef, mode)
        return

    def _Num(self, node, expr, coef, mode):
        """
        expr -> Num(object n)
        """

        if "literals" in expr:
            expr["literals"] += coef * node.n
        else:
            expr["literals"] = coef * node.n

        return

    def _Subscript(self, node, expr, coef, mode):
        """
        expr -> Subscript(expr value, slice slice, expr_context ctx)
        """

        # B[i] = ...
        #   Subscript(
        #       value=Name(id='B', ctx=Load()),
        #       slice=Index(
        #           value=Name(id='i', ctx=Load()),
        #       ),
        #       ctx=Store(),
        #   ),
        #
        #
        # B[i + 2] = ...
        #   Subscript(
        #       value=Name(id='B', ctx=Load()),
        #       slice=Index(
        #           value=BinOp(
        #               left=Name(id='i', ctx=Load()),
        #               op=Add(),
        #               right=Num(n=1),
        #           ),
        #       ),
        #       ctx=Store(),
        #   ),

        if isinstance(node.value, ast.Name):
            pass

        self.visit(node.value, expr, coef, mode)
        self.visit(node.slice, expr, coef, mode)

        if isinstance(node.slice.value, ast.Name):  # B[i]
            iterator = node.slice.value.id
            self.oscop.check_iterator_in_domain(iterator)

        if isinstance(node.slice.value, ast.BinOp):  # B[i + 2]
            raise Exception("TBD TBD")

        return

    def _Name(self, node, expr, coef, mode):
        """
        expr -> Name(identifier id, expr_context ctx)
        """

        if node.id in expr:
            expr[node.id] += coef
        else:
            expr[node.id] = coef  # N > 4:    expr["N"] = 1

        return

    ###############################################################
    # slice
    ###############################################################
    # expr -> Subscript(expr value, slice slice, expr_context ctx)

    def _Index(self, node, expr, coef, mode):
        """
        slice -> Index(expr value)
        """

        expr = {}
        self.visit(node.value, expr, coef, mode)
        self.oscop.fill_params_to_globalinfo(expr)
        return

    ###############################################################
    # operator
    ###############################################################
    # expr -> BinOp(expr left, operator op, expr right)
    # stmt -> AugAssign(expr target, operator op, expr value)

    # example input code
    # b[i] = a[i] + 7
    # b[i] = a[i + 1]
    # b[i] = a[7 * i + 1]

    def _AddSub(self, node, expr, coef, mode):
        """
        operator
        node.__class__.__name__ == BinOp
        """

        left = node.left
        right = node.right
        self.visit(left, expr, coef, mode)

        if isinstance(left, ast.Subscript):
            pass
            # self.old_fill_access(self.statements[-1], "READ", expr)

        if isinstance(right, ast.Subscript):
            expr = {}

        self.visit(right, expr, coef, mode)

        if isinstance(right, ast.Subscript):
            # self.old_fill_access(self.statements[-1], "READ", expr)
            pass

        return

    def _Mult(self, node, expr, coef, mode):
        """
        operator
        node.__class__.__name__ == BinOp
        """

        left = node.left
        right = node.right

        if isinstance(left, ast.Num):
            self.visit(right, expr, left.n * coef)
        elif isinstance(right, ast.Num):
            self.visit(left, expr, right.n * coef)
        else:
            self.visit(left, expr, coef, mode)
            self.visit(right, expr, coef, mode)

        return
