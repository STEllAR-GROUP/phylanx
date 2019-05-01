# Copyright (c) 2018 R. Tohid
# Copyright (c) 2019 Ye Fang
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast
import numpy as np
import copy
# from itertools import chain
from string import Template
from .utils import dump_to_file
from .utils import save_to_file
from .utils import print_python_ast_node
from .utils import dump_ast as python_ast_format
# from .utils import dump_ast_v2 as python_ast_format
"""
def mycgen(python_ast):
    import cgen

    aststr = python_ast_format(python_ast)
    print("mycgen")
    print(aststr)

    a = cgen.Statement(aststr)
    print("mycgen")
    print(a)
"""


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

        # updating domain appears in:
        # stmt -> For
        self.domain = []  # domain stack
        self.domain_iter = []  # domain stack, only iterators

        # updating scatter appears in:
        # stmt -> For                 self.scatter.append(iterator)
        # stmt -> Assign              self.scatter.append(0), or ++
        # stmt -> AugAssign
        self.scatter = []  # eg: [0, "i", 1, "j", 0]

        self.globalinfo = self.empty_global()  # data for global
        self.statements = []  # data for statements

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
            relation=[],
        )
        return d

    def empty_statment(self):
        d = dict(
            idx=None,
            num_relations=0,
            domain=[],
            domain_iter=[],
            scatter=[],
            access=[],
        )
        return d

    """
    def check_iterator_in_domain(self, iterator):
        if iterator not in self.domain_iter:
            raise Exception( \
                "Iterator %s is not defined in this scope." % iterator)
    """

    def fill_params_to_globalinfo(self, expr):
        """
        Filling parameter data into "self.globalinfo".
        New parameter may exist in any expressions.
        New parameter constraint may exist in "If" expressions.
        """

        expr_set = set(expr.keys()) - set(["literals"])
        domain_set = set(self.domain_iter)

        # import inspect # DEBUG
        # print(inspect.stack()) # DEBUG
        print("debug fill_param_to_globalinfo", expr)
        print("debug fill_param_to_globalinfo", expr_set)
        print("debug fill_param_to_globalinfo", domain_set)

        # Finding a new parameter constraint \
        # when the expr does not have domain constraints \
        # A.K.A the intersection is empty
        if (len(expr_set & domain_set) == 0):
            # data collected, to format in "generate_global" method
            self.globalinfo["relation"].append(copy.deepcopy(expr))
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
        statement["idx"] = len(self.statements)

    def update_domain(self, op, domaininfo=None, iterator=None):
        # print("debug update_domain", op, domaininfo, iterator)

        if (op == "enter"):
            self.domain.append(copy.deepcopy(domaininfo))
            self.domain_iter.append(iterator)
        elif (op == "exit"):
            self.domain.pop()
            self.domain_iter.pop()
        else:
            raise Exception("Not supported")

    def update_scatter(self, op, iterator=None):
        def rindex(lst, value):
            """
            finding the last index of a value in a list
            """
            for i, v in enumerate(reversed(lst)):
                if v == value:
                    return len(lst) - i - 1
            raise Exception("iterator not found.")

        if (op == "enter"):
            if (len(self.scatter) == 0):
                self.scatter.append(0)
            elif (isinstance(self.scatter[-1], int)):
                self.scatter[-1] = self.scatter[-1] + 1
            else:
                self.scatter.append(0)

            if (iterator is not None):  # entering a loop
                self.scatter.append(iterator)

        elif (op == "exit"):
            current_iter = self.domain_iter[-1]
            idx = rindex(self.scatter, current_iter)
            del self.scatter[idx:]

        else:
            raise Exception("Not supported")

    def fill_domain_to_statement(self):
        statement = self.statements[-1]
        statement["domain"] = copy.deepcopy(self.domain)
        statement["domain_iter"] = copy.deepcopy(self.domain_iter)

    def fill_scatter_to_statement(self):
        statement = self.statements[-1]
        statement["scatter"] = copy.deepcopy(self.scatter)

    def fill_access_to_statement(self, expr, rw, dim):
        valid_rw = ["assign_lhs", "assign_rhs",\
                    "augassign_lhs", "augassign_rhs"]
        if (rw not in valid_rw):
            raise Exception("rw = %s is not supported", rw)

        statement = self.statements[-1]
        expr_ = copy.deepcopy(expr)
        expr_["rw"] = rw

        if (dim == "dim_one"):
            statement["access"].append([expr_])
            # print("debug dim_one", expr_)
        elif (dim == "dim_high"):
            statement["access"][-1].append(expr_)
            # print("debug dim_high", expr_)
        else:
            raise Exception("Not supported")

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
            num_statements="",
        )

        s = "CONTEXT" + "\n"
        s += str(self.globalinfo["num_rows"]) + " "
        s += str(self.globalinfo["num_cols"]) + " "
        s += str(self.globalinfo["num_output_dim"]) + " "
        s += str(self.globalinfo["num_input_dim"]) + " "
        s += str(self.globalinfo["num_local_dim"]) + " "
        s += str(self.globalinfo["num_params"]) + "\n"
        for expr in self.globalinfo["relation"]:
            s += "  TOFORMAT " + str(expr) + "\n"
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

        # print("gen domain", statement)
        # print("gen domain", statement["domain"])

        statement["num_relations"] += 1

        rv = ""
        rv += "# ----------------------------------------------  " + \
            str(statement["idx"]) + ".1 Domain\n"
        rv += "  TOFORMAT " + str(statement["domain"]) + "\n"
        rv += "\n"
        return rv

    def generate_scatter(self, statement):
        # print("gen scatter", statement)
        # print("gen scatter", statement["scatter"])
        statement["num_relations"] += 1

        rv = ""
        rv += "# ----------------------------------------------  " + \
            str(statement["idx"]) + ".2 Scattering\n"
        rv += "  TOFORMAT " + str(statement["scatter"]) + "\n"
        rv += "\n"
        return rv

    def generate_access(self, statement):
        # print("gen access ", statement)
        # print("gen access ", statement["access"])

        if len(statement["access"]) == 0:
            return ""

        rv = ""
        rv += "# ----------------------------------------------  " + \
            str(statement["idx"]) + ".3 Access\n"
        rv += "  TOFORMAT1 " + str(statement["access"]) + "\n"

        for access in statement["access"]:
            statement["num_relations"] += 1
            array_name = access[0]["array_name"]
            rv += "  TOFORMAT2 " + access[0]["rw"] + " " + array_name + "\n"
            for dim in access:
                for iterator in statement["domain_iter"]:
                    if (iterator in dim.keys()):
                        rv += " " + iterator + ":" + str(dim[iterator])
                for para in self.globalinfo["params"]:
                    if (para in dim.keys()):
                        rv += " " + para + ":" + str(dim[para])
                if ("literals" in dim.keys()):
                    rv += " " + "literals" + ":" + str(dim["literals"])
                rv += "\n"
        rv += "\n"
        return rv

    def generate_oscop_statement(self, statement):
        # print("debug generation statement")
        # print(statement)

        str_domain = self.generate_domain(statement)
        str_scatter = self.generate_scatter(statement)
        str_access = self.generate_access(statement)

        # print("debug generate_statements", statement)   # DEBUG
        # print("")  # DEBUG

        rv = ""
        rv += "# =============================================== Statement " + \
            str(statement["idx"]) + "\n"
        rv += "# Number of relations describing the statement" + "\n"
        rv += str(statement["num_relations"]) + "\n"
        rv += "\n"
        rv += str_domain + str_scatter + str_access
        return rv

    def generate(self):
        code = ""
        code = code + self.generate_oscop_global()
        for statement in self.statements:
            code = code + self.generate_oscop_statement(statement)
        return code

    # help function
    def print1(self):
        print("Oscop.domain: ", self.domain)
        print("Oscop.domain_iter: ", self.domain_iter)
        print("Oscop.scatter: ", self.scatter)
        print("Oscop.globalinfo: ", self.globalinfo)
        print("Oscop.statements: ", self.statements)
        # print("")


class OpenSCoP:
    def __init__(self, func, python_ast, kwargs):
        self.oscop = Oscop(kwargs)

        self.default_expr = {}
        self.default_coef = 1
        self.default_mode = ""
        self.expr = copy.deepcopy(self.default_expr)
        """
        expr    dict    read/write
            {}              default
            "literals"      integer literal, the weight of const "1"
            "Para"          parameter "Para"
            "Name"          name "Name"
        coef    int     readonly
            1               default
            -1
        mode    str     readonly
           ""               default
           "write"          for "_Assign node"
           "read"           for "_Assign node"
        """

        kernel = python_ast.body[0].body
        for node in kernel:
            self.expr = copy.deepcopy(self.default_expr)
            self.visit(node, self.default_coef, self.default_mode)

        print("############################")
        # dump_to_file(self.__src__, "dump_openscop", kwargs)
        aststr = python_ast_format(python_ast)
        save_to_file(aststr, "ooo_pythonast.txt", True)
        self.__src__ = self.oscop.generate()
        save_to_file(self.__src__, "ooo_openscop.txt", True)

    # help function
    def print1(self, node, coef, mode):
        if True:
            print("############################")
            self.oscop.print1()
        if True:
            print("OpenSCoP.self.expr: ", self.expr)
            print("OpenSCoP.coef: ", coef)
            print("OpenSCoP.mode: ", mode)
            # print("OpenSCoP.defaults ", self.default_expr, \
            # self.default_coef, self.default_mode)
            print("")
        if True:
            print_python_ast_node("OpenSCoP.node", node)
            print("")

    def visit(self, node, coef, mode):
        """
        Calls the corresponding rule, based on the name of the node.
        eg:
            Add -> self._Add
        """

        if not isinstance(node, ast.AST):
            raise Exception("The type is not \"python AST node\"")

        self.print1(node, coef, mode)
        f = eval("self._%s" % node.__class__.__name__)
        return f(node, coef, mode)

    # python AST grammar reference
    # https://docs.python.org/3/library/ast.html

    ###############################################################
    # stmt
    ###############################################################

    def _Assign(self, node, coef, mode):
        """
        stmt -> Assign(expr* targets, expr value)
        """

        if len(node.targets) > 1:
            raise Exception("Does not support chain assignments.")
        if isinstance(node.targets[0], ast.Tuple):
            raise Exception("Does not support multi-target assignments.")
        lhs = node.targets[0]
        rhs = node.value

        # mycgen(node)

        self.oscop.new_statement()
        self.oscop.update_scatter("enter")
        self.oscop.fill_domain_to_statement()
        self.oscop.fill_scatter_to_statement()

        self.expr = copy.deepcopy(self.default_expr)
        self.visit(lhs, self.default_coef, "assign_lhs")

        self.expr = copy.deepcopy(self.default_expr)
        self.visit(rhs, self.default_coef, "assign_rhs")
        return

    def _AugAssign(self, node, expr, coef):
        """
        stmt -> AugAssign(expr target, operator op, expr value)
        """

        lhs = node.target
        rhs = node.value

        self.oscop.new_statement()
        self.oscop.update_scatter("enter")
        self.oscop.fill_domain_to_statement()
        self.oscop.fill_scatter_to_statement()

        self.expr = copy.deepcopy(self.default_expr)
        self.visit(lhs, self.default_coef, "augassign_lhs")

        self.expr = copy.deepcopy(self.default_expr)
        self.visit(rhs, self.default_coef, "augassign_rhs")
        return

    def _For(self, node, coef, mode):
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

        iterator = node.target.id

        if not isinstance(node.iter, ast.Call):  # call something
            self._For_exception()

        if (node.iter.func.id != "range"):  # call range function
            self._For_exception()

        bounds = node.iter.args
        if not ((len(bounds) == 1) or (len(bounds) == 2)):  # 1 arg or 2 args
            self._For_exception()

        # lower bound
        self.expr = copy.deepcopy(self.default_expr)
        self.expr[iterator] = 1
        self.expr["literals"] = 0
        if (len(bounds) == 1):
            lb = 0  # debug, not used
        if (len(bounds) == 2):
            lb = node.iter.args[0]
            self.visit(lb, self.default_coef, self.default_mode)
        expr_l = self.expr

        # print("debug expr_l")
        # print(expr_l)

        # uppwer bound
        self.expr = copy.deepcopy(self.default_expr)
        self.expr[iterator] = -1
        self.expr['literals'] = -1
        ub = node.iter.args[-1]  # the last argment passed to range()
        self.visit(ub, self.default_coef, self.default_mode)
        expr_u = self.expr

        # print("debug expr_u")
        # print(expr_u)

        mydomain = []
        mydomain.append(expr_l)
        mydomain.append(expr_u)
        self.oscop.update_domain("enter", copy.deepcopy(mydomain), iterator)
        self.oscop.update_scatter("enter", iterator)
        self.oscop.fill_params_to_globalinfo(copy.deepcopy(expr_l))
        self.oscop.fill_params_to_globalinfo(copy.deepcopy(expr_u))

        for n in node.body:
            self.expr = copy.deepcopy(self.default_expr)
            self.visit(n, self.default_coef, self.default_mode)

        self.oscop.update_scatter("exit")
        self.oscop.update_domain("exit")  # existing last
        return

    def _If(self, node, coef, mode):
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
            self.expr = copy.deepcopy(self.default_expr)
            self.visit(node.test, self.default_coef, self.default_mode)
            self.oscop.fill_params_to_globalinfo(copy.deepcopy(self.expr))

        else:
            raise NotImplementedError

        for n in node.body:
            self.expr = copy.deepcopy(self.default_expr)
            self.visit(n, self.default_coef, self.default_mode)

        return

    def _Pass(self, node, coef, mode):
        """
        stmt -> Pass
        """

        return

    ###############################################################
    # expr
    ###############################################################

    def _BinOp(self, node, coef, mode):
        """
        expr -> BinOp(expr left, operator op, expr right)
        """

        left = node.left
        right = node.right

        if (isinstance(node.op, ast.Add)):
            self.visit(left, coef, mode)
            self.visit(right, coef, mode)

        elif (isinstance(node.op, ast.Sub)):
            self.visit(left, coef, mode)
            self.visit(right, coef * -1, mode)

        elif (isinstance(node.op, ast.Mult)):
            if isinstance(left, ast.Num):
                self.visit(right, left.n * coef, mode)
            elif isinstance(right, ast.Num):
                self.visit(left, right.n * coef, mode)
            else:
                self.visit(left, coef, mode)
                self.visit(right, coef, mode)

        else:
            raise NotImplementedError
        return

    def _UnaryOp(self, node, coef, mode):
        """
        expr -> UnaryOp(unaryop op, expr operand)
        """

        if isinstance(node.op, ast.USub):
            self.visit(node.operand, coef * -1, mode)
        else:
            raise NotImplementedError
        return

    def _Compare(self, node, coef, mode):
        """
        expr -> Compare(expr left, cmpop* ops, expr* comparators)
        """

        if (len(node.ops) != 1):  # eg: 2 < N < 5
            raise NotImplementedError("Accelpt only one op")
        if (len(node.comparators) != 1):  # eg: 2 < N < 5
            raise NotImplementedError("Accept only one comparator")

        left = node.left
        ops = node.ops[0]
        right = node.comparators[0]
        my_cmpop = {
            "Lt": (1, -1),
            "LtE": (0, -1),
            "Gt": (-1, 1),
            "GtE": (0, 1),
        }
        self.expr["literals"], coef_ = my_cmpop[ops.__class__.__name__]
        self.visit(left, coef_, mode)
        self.visit(right, coef_ * -1, mode)
        return

    def _Call(self, node, coef, mode):
        """
        expr -> Call(expr func, expr* args, keyword* keywords)
        """
        raise Exception("Does not support \"Call\"")
        return

    def _Num(self, node, coef, mode):
        """
        expr -> Num(object n)
        """

        if "literals" in self.expr:
            self.expr["literals"] += coef * node.n
        else:
            self.expr["literals"] = coef * node.n

        return

    def _Subscript(self, node, coef, mode):
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
        #
        # B[i][j] = ...
        #   Subscript(
        #       value=Subscript(
        #           value=Name(id='B', ctx=Load()),
        #           slice=Index(
        #               value=Name(id='i', ctx=Load()),
        #           ),
        #           ctx=Load(),
        #       ),
        #       slice=Index(
        #           value=Name(id='j', ctx=Load()),
        #       ),
        #       ctx=Store(),
        #   ),

        if (mode == "address"):
            raise Exception("SCoP does not support subscripts in address mode")
        if (not isinstance(node.slice, ast.Index)):
            raise Exception("subscript.slice must be Index type")

        # one-dimensional array: "B[i]"
        if isinstance(node.value, ast.Name):
            self.expr = copy.deepcopy(self.default_expr)
            self.visit(node.value, self.default_coef, mode)
            array_name = node.value.id

            self.expr = copy.deepcopy(self.default_expr)
            self.visit(node.slice, self.default_coef, "address")
            self.oscop.fill_params_to_globalinfo(copy.deepcopy(self.expr))

            self.expr["array_name"] = array_name
            self.oscop.fill_access_to_statement(copy.deepcopy(self.expr),\
                                                mode, "dim_one")

        # multi-dimensional array: "B[i][j]"
        elif isinstance(node.value, ast.Subscript):
            self.expr = copy.deepcopy(self.default_expr)
            self.visit(node.value, self.default_coef, mode)
            array_name = self.expr["array_name"]  # returned from lower dim

            self.expr = copy.deepcopy(self.default_expr)
            self.visit(node.slice, self.default_coef, "address")
            self.oscop.fill_params_to_globalinfo(copy.deepcopy(self.expr))

            self.expr["array_name"] = array_name
            self.oscop.fill_access_to_statement(copy.deepcopy(self.expr),\
                                                mode, "dim_high")

        else:
            raise Exception("Not supported")
        return

    def _Name(self, node, coef, mode):
        """
        expr -> Name(identifier id, expr_context ctx)
        """
        if node.id in self.expr:
            self.expr[node.id] += coef
        else:
            self.expr[node.id] = coef
        return

    ###############################################################
    # slice
    ###############################################################
    # expr -> Subscript(expr value, slice slice, expr_context ctx)

    def _Index(self, node, coef, mode):
        """
        slice -> Index(expr value)
        """
        self.visit(node.value, coef, mode)
        return

    def _Slice(self, node, coef, mode):
        raise Exception("Not supported")
        return
