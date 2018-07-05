# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import ast
import re
import phylanx
import inspect
from phylanx.exceptions import InvalidDecoratorArgumentError
from phylanx.util import prange
from .oscop import OpenSCoP
from .utils import full_name, full_node_name, physl_fmt

et = phylanx.execution_tree

# globals used during the compilation


def is_node(node, name):
    """Return the node name"""
    if node is None:
        return False
    return node.__class__.__name__ == name


def get_node(node, **kwargs):
    if node is None:
        return None
    args = [arg for arg in ast.iter_child_nodes(node)]
    params = {"name": None, "num": None}
    for k in kwargs:
        if k not in params:
            raise Exception("Invalid argument '%s'" % k)
        else:
            params[k] = kwargs[k]
    name = params["name"]
    num = params["num"]
    if name is not None:
        for i in range(len(args)):
            a = args[i]
            nm = a.__class__.__name__
            if nm == name:
                if num is None or num == i:
                    return a
    elif num is not None:
        if num < len(args):
            return args[num]

    # if we can't find what we're looking for...
    return None


def get_call_func_name(ast_call):
    if is_node(ast_call, 'Call'):
        return ast_call.func.id
    return None


def remove_line(a):
    return re.sub(r'\$.*', '', a)


class PhySL:
    def __init__(self, tree, kwargs):
        self.defs = {}
        self.priority = 0
        self.fglobals = kwargs['fglobals']
        self.groupAggressively = True
        for arg in tree.body[0].args.args:
            self.defs[arg.arg] = 1
        self.__src__ = self.recompile(tree)

    def _Arguments(self, a, allowreturn=False):
        ret = ''
        if a.args:
            for arg in a.args[:-1]:
                ret += arg.arg + ', '
            ret += a.args[-1].arg
        return ret

    def _Num(self, a, allowreturn=False):
        return str(a.n)

    def _Str(self, a, allowreturn=False):
        return '"' + a.s + '"'

    def _Name(self, a, allowreturn=False):
        return full_node_name(a, a.id)

    def _NameConstant(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        if a.value is None:
            return "nil%s" % symbol_info

        if a.value is False:
            return "false%s" % symbol_info

        if a.value is True:
            return "true%s" % symbol_info

    def _Expr(self, a, allowreturn=False):
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = "("
        if len(args) == 1:
            s += self.recompile(args[0])
        else:
            raise Exception(
                'unexpected: expression has more than one sub-expression')
        return s + ")"

    def _Subscript(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        s = 'slice%s(%s,' % (symbol_info, self.recompile(a.value))
        if isinstance(a.slice, ast.Index):
            s += '%s)' % self.recompile(a.slice.value)
            return s
        elif isinstance(a.slice, ast.Slice):
            s += self.recompile(a.slice)
            return s + ')'
        else:
            s += self.recompile(a.slice)
            return s + ')'

    def _Slice(self, a, allowreturn=False):
        if a.lower is None:
            alow = "0"
        else:
            alow = self.recompile(a.lower)
        if a.upper is None:
            aupp = "nil"
        else:
            aupp = self.recompile(a.upper)
        s = 'make_list(%s, %s' % (alow, aupp)
        if a.step:
            s += ', %s)' % self.recompile(a.step)
        else:
            s += ')'
        return s

    def _ExtSlice(self, a, allowreturn=False):
        if isinstance(a.dims[0], ast.Index):
            s = self.recompile(a.dims[0].value)
        else:
            s = self.recompile(a.dims[0])
        s += ","
        if isinstance(a.dims[1], ast.Index):
            s += self.recompile(a.dims[1].value)
        else:
            s += self.recompile(a.dims[1])
        return s

# NOTE
# this function returns comma separated entries of the tuple without the parentheses.

    def _Tuple(self, a, allowreturn=False):
        s = ''
        for i in range(len(a.elts) - 1):
            s += self.recompile(a.elts[i]) + ', '
        s += self.recompile(a.elts[-1])
        return s

    def _FunctionDef(self, a, allowreturn=False):
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = ""
        s += '%s(' % full_node_name(a, 'define')
        s += full_name(a)
        s += ", "
        for arg in ast.iter_child_nodes(args[0]):
            s += full_node_name(arg, arg.arg)
            s += ", "
        if len(args) == 2:
            s += self.recompile(args[1], True).strip(' ')
        else:
            s += '%s(' % full_node_name(a, 'block')
            sargs = args[1:]
            for i in range(len(sargs)):
                aa = sargs[i]
                if i + 1 == len(sargs):
                    s += self.recompile(aa, True)
                else:
                    s += self.recompile(aa, False)
                    s += ", "
            s += ")"
        s += ")"
        return s

    def _BinOp(self, a, allowreturn=False):
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = ""
        save = self.priority
        nm2 = args[1].__class__.__name__
        priority = 2
        if nm2 == "Add":
            op = " + "
            priority = 1
        elif nm2 == "Sub":
            op = " - "
            priority = 1
        elif nm2 == "Mult":
            op = " * "
        elif nm2 == "Div":
            op = " / "
        elif nm2 == "Mod":
            op = " % "
        elif nm2 == "Pow":
            op = " ** "
            priority = 3
        else:
            raise Exception('binary operation not supported: %s' % nm2)
        self.priority = priority
        term1 = self.recompile(args[0])
        self.priority = priority
        term2 = self.recompile(args[2])
        if priority < save or self.groupAggressively:
            s += "(" + term1 + op + term2 + ")"
        else:
            s += term1 + op + term2
        return s

    def _Call(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        s = ''
        if isinstance(a.func, ast.Attribute):
            func_name = a.func.attr
            pkg_name = get_node(a.func, num=0, name="Name")
            if pkg_name is not None and pkg_name.id not in self.fglobals:
                print(self.fglobals)
                raise Exception("package name '%s' does not exist" % pkg_name)
            if func_name == "zeros" and len(a.args) == 1:
                s += 'constant(0, make_list('
                # The code below works the same for tuples and lists
                args = [arg for arg in ast.iter_child_nodes(a.args[0])]
                for i in range(len(args) - 1):
                    if i > 0:
                        s += ','
                    s += self.recompile(args[i])
                s += '))'
                return s
            elif func_name == "array" and len(a.args) == 1:
                s += 'hstack%s(' % symbol_info
                # The code below works the same for tuples and lists
                args = [arg for arg in ast.iter_child_nodes(a.args[0])]
                for i in range(len(args) - 1):
                    if i > 0:
                        s += ','
                    s += self.recompile(args[i])
                s += ')'
                return s
            else:
                s += self._Attribute(a.func)
            for i in range(len(a.args) - 1):
                s += self.recompile(a.args[i])
                s += ', '
            s += self.recompile(a.args[-1])
            s += ')'

        else:  # a.func is ast.Name
            args = [arg for arg in ast.iter_child_nodes(a)]
            if args[0].id == "print":
                args[0].id = "cout"
            if args[0].id == "prange":
                args[0].id = "range"

            s = args[0].id + symbol_info + '('
            for n in range(1, len(args)):
                if n > 1:
                    s += ', '
                s += self.recompile(args[n])
            s += ')'
        return s

    def _Module(self, a, allowreturn=False):
        args = [arg for arg in ast.iter_child_nodes(a)]
        return self.recompile(args[0])

    def _Return(self, a, allowreturn=False):
        if not allowreturn:
            raise Exception(
                "Return only allowed at end of function: line=%d\n" % a.lineno)
        args = [arg for arg in ast.iter_child_nodes(a)]
        return self.recompile(args[0])

    def _Assign(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = ""

        if hasattr(args[0], "id"):
            # Assign to a variable by name, e.g. a = ...
            if args[0].id in self.defs:
                s += "store" + symbol_info
            else:
                s += "define" + symbol_info
                self.defs[args[0].id] = 1
            a = '%s' % full_node_name(args[0], args[0].id)
            s += "(" + a + ", " + self.recompile(args[1]) + ")"
        elif len(args) == 2 and is_node(args[0], "Subscript"):
            # Assign to a variable with a single dimension, e.g. a[ ? ] = ...
            vname = get_node(args[0], num=0, name="Name")
            indexv = get_node(args[0], num=1)
            indexv_nm = indexv.__class__.__name__
            if indexv_nm == "Slice":
                # Assign when the subscript is a slice, e.g. a[lo:hi] = ...
                s += "set("
                s += self.recompile(vname)
                s += ","
                if indexv.lower is None:
                    s += "0,"
                else:
                    s += self.recompile(indexv.lower) + ","
                if indexv.upper is None:
                    s += "0,"
                else:
                    s += self.recompile(indexv.upper) + ","
                s += "1,"
                s += "0,0,0,"
                s += self.recompile(args[1])
                s += ")"
            elif indexv_nm == "Index":
                # Assign when the subscript is a value, e.g. a[i] = ... or a[i,j] = ...
                s += "set("
                s += self.recompile(vname)
                s += ","
                indexs = get_node(indexv, num=0)
                if not is_node(indexs, "Tuple"):
                    s += self.recompile(indexs) + ","
                else:
                    s += self.recompile(get_node(indexs, num=0)) + ","
                s += "0,0,"
                if not is_node(indexs, "Tuple"):
                    s += "0,"
                else:
                    s += self.recompile(get_node(indexs, num=1)) + ","
                s += "0,0,"
                s += self.recompile(args[1])
                s += ")"
            elif indexv_nm == "ExtSlice":
                # Assign when the subscript is an extended
                # slice, e.g. a[x1:x2,y1:y2] = ...
                slice1 = get_node(indexv, num=0)
                slice2 = get_node(indexv, num=1)
                s += "set("
                s += self.recompile(vname)
                s += ","

                if slice1.lower is None:
                    s += "0,"
                else:
                    s += self.recompile(slice1.lower) + ","

                if slice1.upper is None:
                    s += "nil,"
                else:
                    s += self.recompile(slice1.upper) + ","
                s += "1,"

                if slice2.lower is None:
                    s += "0,"
                else:
                    s += self.recompile(slice2.lower) + ","

                if slice2.upper is None:
                    s += "nil,"
                else:
                    s += self.recompile(slice2.upper) + ","

                s += "1,"

                s += self.recompile(args[1])
                s += ")"
            else:
                raise Exception("Unsupported slicing in assignment: line=%d" % a.lineno)
            return s
        else:
            raise Exception("Unsupported slicing in assignment: line=%d" % a.lineno)
        return s

    def _Attribute(self, a, allowreturn=False):
        s = ""
        if isinstance(a.value, ast.Attribute):
            if a.attr in self.np_to_phylanx:
                symbol_info = full_node_name(a)
                s += self.np_to_phylanx[a.attr] + symbol_info + '('
                return s
            else:
                raise NotImplementedError

        if isinstance(a.value, ast.Name):
            if a.attr in self.np_to_phylanx:
                symbol_info = full_node_name(a)
                s += self.np_to_phylanx[a.attr] + symbol_info + '('

            if a.value.id in self.defs:
                s += a.value.id + full_node_name(a.value)
                return s
            elif a.attr == "array":
                raise Exception()
            elif a.value.id in self.fglobals and  \
                    hasattr(self.fglobals[a.value.id], a.attr):
                # Note that the above check verifies that in the extenal environment
                # a module named a.value.id contains a symbol named a.attr, e.g.
                # module "np" contains "shape"
                return s
            else:
                raise LookupError("Undefined function: %s.%s()" % (a.value.id, a.attr))
        else:
            s += self.recompile(a.value)
            return s

    def _AugAssign(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        args = [arg for arg in ast.iter_child_nodes(a)]
        sym = "?"
        nn = args[1].__class__.__name__
        if nn == "Add":
            sym = "+"
        elif nn == "Sub":
            sym = "-"
        elif nn == "Mult":
            sym = "*"
        elif nn == "Div":
            sym = "/"
        else:
            le_str = "Undefined operation discovered: %s=" % (nn,)
            raise LookupError(le_str)

        id_ = "UNKNOWN_ID"
        if type(args[0]) == ast.Subscript:
            id_ = args[0].value.id
        else:
            id_ = args[0].id

        arg0 = '%s' % full_node_name(args[0], id_)

        return "store%s(" % symbol_info + arg0 + ", " + arg0 + sym + self.recompile(
            args[2]) + ")"

    def _While(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = "while" + symbol_info + "(" + self.recompile(args[0]) + ", "
        if len(args) == 2:
            s += self.recompile(args[1])
        else:
            tw = "block" + symbol_info + "("
            for aa in args[1:]:
                s += tw
                tw = ", "
                s += self.recompile(aa)
            s += ")"
        s += ")"
        return s

    def _If(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        s = "if" + symbol_info + "(" + self.recompile(a.test) + ", "
        if len(a.body) > 1:
            s += "block" + symbol_info + "("
            for j in range(len(a.body)):
                aa = a.body[j]
                if j > 0:
                    s += ", "
                if j + 1 == len(a.body):
                    s += self.recompile(aa, allowreturn)
                else:
                    s += self.recompile(aa)
            s += ")"
        else:
            s += self.recompile(a.body[0], allowreturn)
        s += ", "
        if len(a.orelse) > 1:
            s += "block" + symbol_info + "("
            for j in range(len(a.orelse)):
                aa = a.orelse[j]
                if j > 0:
                    s += ", "
                if j + 1 == len(a.orelse):
                    s += self.recompile(aa, allowreturn)
                else:
                    s += self.recompile(aa)
            s += ")"
        elif len(a.orelse) == 1:
            s += self.recompile(a.orelse[0], allowreturn)
        else:
            s += "block" + symbol_info + "()"
        s += ")"
        return s

    def _Lambda(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        ret = "lambda%s(" % symbol_info
        ret += self.recompile(a.args)
        ret += ", block(" + self.recompile(a.body) + '))'
        return ret

    def _List(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        ret = "make_list%s(" % symbol_info
        for arg in ast.iter_child_nodes(a):
            if arg.__class__.__name__ == "Load":
                break
            if ret != "make_list%s(" % symbol_info:
                ret += ","
            ret += self.recompile(arg)
        ret += ")"
        return ret

    def _Compare(self, a, allowreturn=False):
        args = [arg for arg in ast.iter_child_nodes(a)]
        sym = "?"
        nn = args[1].__class__.__name__
        if nn == "Lt":
            sym = " < "
        elif nn == "Gt":
            sym = " > "
        elif nn == "LtE":
            sym = " <= "
        elif nn == "GtE":
            sym = " >= "
        elif nn == "NotEq":
            sym = " != "
        elif nn == "Eq":
            sym = " == "
        else:
            raise Exception('boolean operation not supported: %s' % nn)
        return '(' + self.recompile(args[0]) + sym + self.recompile(
            args[2]) + ')'

    def _UnaryOp(self, a, allowreturn=False):
        args = [arg for arg in ast.iter_child_nodes(a)]
        nm2 = args[0].__class__.__name__
        nm3 = args[1].__class__.__name__
        if nm2 == "USub":
            if nm3 == "BinOp" or self.groupAggressively:
                return "-(" + self.recompile(args[1]) + ")"
            else:
                return "-" + self.recompile(args[1])
        elif nm2 == "Not":
            if self.groupAggressively:
                return "!(" + self.recompile(args[1]) + ")"
            else:
                return "!" + self.recompile(args[1])
        else:
            raise Exception('unary operation not supported: %s' % nm2)

    def _For(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        ret = ""
        func_nom = get_call_func_name(a.iter)
        func_nom_symbol_info = full_node_name(a.iter)

        if is_node(a.iter, 'Call'):
            if func_nom == 'prange':
                ret = "parallel_map%s(lambda%s(" % (symbol_info, func_nom_symbol_info)
            else:
                ret = "map%s(lambda%s(" % (symbol_info, func_nom_symbol_info)
        else:
            ret = "map%s(lambda%s(" % (symbol_info, func_nom_symbol_info)

        ret += self.recompile(a.target) + ', block('

        # find a prange

        blocki = 2

        while True:
            blockn = get_node(a, num=blocki)
            if blockn is None:
                break
            if blocki > 2:
                ret += ", "
            ret += self.recompile(blockn)
            blocki += 1

        ret += ")), " + self.recompile(a.iter) + ')'
        return ret

    def recompile(self, a, allowreturn=False):
        nm = a.__class__.__name__
        try:
            if allowreturn:
                return self.nodes[nm](self, a, allowreturn)
            else:
                return self.nodes[nm](self, a)
        except NotImplementedError:
            raise Exception('unsupported AST node type: %s' % nm)

    nodes = {
        "arguments": _Arguments,
        "Assign": _Assign,
        "Attribute": _Attribute,
        "AugAssign": _AugAssign,
        "BinOp": _BinOp,
        "Call": _Call,
        "Compare": _Compare,
        "Expr": _Expr,
        "ExtSlice": _ExtSlice,
        "For": _For,
        "FunctionDef": _FunctionDef,
        "If": _If,
        "Lambda": _Lambda,
        "List": _List,
        "Module": _Module,
        "Name": _Name,
        "NameConstant": _NameConstant,
        "Num": _Num,
        "Return": _Return,
        "Slice": _Slice,
        "Str": _Str,
        "Subscript": _Subscript,
        "Tuple": _Tuple,
        "UnaryOp": _UnaryOp,
        "While": _While,
    }

    np_to_phylanx = {
        "argmax": "argmax",
        "argmin": "argmin",
        "cross": "cross",
        "det": "determinant",
        "diagonal": "diag",
        "dot": "dot",
        "exp": "exp",
        "hstack": "hstack",
        "identity": "identity",
        "inverse": "inverse",
        # "linearmatrix": "linearmatrix",
        "linspace": "linspace",
        "power": "power",
        "random": "random",
        "shape": "shape",
        "sqrt": "square_root",
        "transpose": "transpose",
        "vstack": "vstack",
        # slicing operations
    }


def convert_to_phylanx_type(v):
    return v


cs = phylanx.compiler_state()


# Create the decorator
def Phylanx(arg=None, target="PhySL", compiler_state=cs, **kwargs):
    class PhyTransformer(object):
        targets = {"PhySL": PhySL, "OpenSCoP": OpenSCoP}

        def __init__(self, f):
            kwargs['fglobals'] = f.__globals__

            if "debug" in kwargs:
                self.debug = kwargs['debug']
            else:
                self.debug = False

            if target not in self.targets:
                raise NotImplementedError(
                    "unknown target passed to '@Phylanx()' decorator: %s." %
                    target)

            self.f = f
            self.cs = compiler_state
            self.target = target

            # Get the source code
            actual_lineno = inspect.getsourcelines(f)[-1]
            src = inspect.getsource(f)

            # Before recompiling the code, take
            # off the decorator from the source.
            src = re.sub(r'^\s*@\w+.*\n', '', src)

            # Strip off indentation if the function
            # is not defined at top level.
            src = re.sub(r'^\s*', '', src)

            # Create the AST
            tree = ast.parse(src)
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1
            self.transformation = self.targets[target](tree, kwargs)
            self.__src__ = self.transformation.__src__
            if self.debug:
                physl_fmt(self.__src__)
                print(end="", flush="")

            if target == "PhySL":
                et.compile(self.__src__, self.cs)

        def __call__(self, *args):
            if target == "OpenSCoP":
                raise NotImplementedError(
                    "OpenSCoP kernel blocks are not yet callable.")
            nargs = tuple(convert_to_phylanx_type(a) for a in args)
            return et.eval(self.f.__name__, self.cs, *nargs)

        def generate_ast(self):
            return phylanx.ast.generate_ast(self.__src__)

    if callable(arg):
        return PhyTransformer(arg)
    elif arg is not None:
        raise InvalidDecoratorArgumentError
    else:
        return PhyTransformer

    return PhyTransformer
