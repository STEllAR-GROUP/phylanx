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
from .oscop import OpenSCoP
from .utils import full_name, full_node_name, physl_fmt

et = phylanx.execution_tree


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


def remove_line(a):
    return re.sub(r'\$.*', '', a)


class PhySL:
    def __init__(self, tree, kwargs):
        self.defs = {}
        self.priority = 0
        self.groupAggressively = True
        for arg in tree.body[0].args.args:
            self.defs[arg.arg] = 1
        self.__src__ = self.recompile(tree)

    def _Num(self, a, allowreturn=False):
        return str(a.n)

    def _Str(self, a, allowreturn=False):
        return '"' + a.s + '"'

    def _Name(self, a, allowreturn=False):
        return full_node_name(a, a.id)

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


#        e = get_node(a, name="ExtSlice")
#        s0 = get_node(e, name="Slice", num=0)
#        s1 = get_node(e, name="Slice", num=1)
#        s0alt = get_node(a, name="Slice", num=1)
#        if s0 is not None and s1 is not None:
#            xlo = s0.lower
#            xlo_info = full_node_name(xlo)
#            xhi = s0.upper
#            xhi_info = full_node_name(xhi)
#            ylo = s1.lower
#            yhi = s1.upper
#            yhi_info = full_node_name(yhi)
#            sname = self.recompile(get_node(a, num=0))
#            s = "slice%s(" % xlo_info
#            s += sname
#            s += ","
#            if xlo is None:
#                s += "0"
#            else:
#                s += self.recompile(xlo)
#            s += ","
#            if xhi is None:
#                s += "shape%s(" % xhi_info + sname + ",0)"
#            else:
#                s += self.recompile(xhi)
#            s += ","
#            if ylo is None:
#                s += "0"
#            else:
#                s += self.recompile(ylo)
#            s += ","
#            if yhi is None:
#                s += "shape%s(" % yhi_info + sname + ",1)"
#            else:
#                s += self.recompile(yhi)
#            s += ")"
#            return s
#        elif s0alt is not None:
#            sname = self.recompile(get_node(a, num=0))
#            xlo = s0alt.lower
#            xlo_info = full_node_name(xlo)
#            xhi = s0alt.upper
#            xhi_info = full_node_name(xhi)
#            s = 'slice_column%s(' % xlo_info
#            s += sname
#            s += ','
#            if xlo is None:
#                s += "0"
#            else:
#                s += self.recompile(xlo)
#            s += ','
#            if xhi is None:
#                s += "shape%s(" % xhi_info + sname + ",0)"
#            else:
#                s += self.recompile(xhi)
#            s += ")"
#            return s
#        else:
#            raise Exception("Unsupported slicing: line=%d" % a.lineno)

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
            s += self._Attribute(a.func)
            for i in range(len(a.args) - 1):
                s += self.recompile(a.args[i])
                s += ', '
            s += self.recompile(a.args[-1])
            s += ')'

        else:
            args = [arg for arg in ast.iter_child_nodes(a)]
            if args[0].id == "print":
                args[0].id = "cout"
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
        if args[0].id in self.defs:
            s += "store" + symbol_info
        else:
            s += "define" + symbol_info
            self.defs[args[0].id] = 1
        a = '%s' % full_node_name(args[0], args[0].id)
        s += "(" + a + ", " + self.recompile(args[1]) + ")"
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
            elif a.value.id == "np" or a.value.id == "numpy":
                return s
            else:
                raise NotImplementedError
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
        arg0 = '%s' % full_node_name(args[0], args[0].id)
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

    def _List(self, a, allowreturn=False):
        ret = "["
        for arg in ast.iter_child_nodes(a):
            if arg.__class__.__name__ == "Load":
                break
            if ret != "[":
                ret += ","
            ret += self.recompile(arg)
        ret += "]"
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
        else:
            raise Exception(nm2)

    def _For(self, a, allowreturn=False):
        symbol_info = full_node_name(a)
        n = get_node(a, name="Name", num=0)
        c = get_node(a, name="Call", num=1)
        if n is not None and c is not None:
            r = get_node(c, name="Name", num=0)
            assert r.id == "range" or r.id == "xrange"
            step = get_node(c, num=3)
            if step is None:
                step = ast.Num(1)
            upper = get_node(c, num=2)
            if upper is None:
                upper = get_node(c, num=1)
                lower = ast.Num(0)
            else:
                lower = get_node(c, num=1)
            ns = self.recompile(n)
            ls = self.recompile(lower)
            us = self.recompile(upper)
            ss = self.recompile(step)
            bs = "block" + symbol_info + "("
            blocki = 2
            while True:
                blockn = get_node(a, num=blocki)
                if blockn is None:
                    break
                if blocki > 2:
                    bs += ", "
                bs += self.recompile(blockn)
                blocki += 1
            bs += ")"
            # The first arg of the for loop is either
            # a define() or a store(), depending on whether
            # the variable has already been defined.
            sd = "store" + symbol_info
            if remove_line(ns) not in self.defs:
                sd = "define" + symbol_info
            lg = "<"
            # Determine the direction of iteration, if possible
            if not re.search(r'[a-zA-Z_]', ss):
                if eval(ss) < 0:
                    lg = ">"
                return "for(" + sd + "(" + ns + ", " + ls + "), " + ns + " " + lg + " " \
                       + us + ",store%s(" % symbol_info + ns + ", " + ns + \
                    "+" + ss + "), " + bs + ")"
            else:
                # if we can't determine the direction of iteration, make two for loops
                ret = "if(" + ss + " > 0, "
                ret += "for(" + sd + "(" + ns + ", " + ls + "), " + ns + " < " + us + \
                    ",store%s(" % symbol_info + ns + ", " + ns + "+" + ss + "), " + bs \
                    + "), "
                ret += "for(" + sd + "(" + ns + ", " + ls + "), " + ns + " > " + us + \
                    ",store%s(" % symbol_info + ns + ", " + ns + "+" + ss + "), " + bs \
                    + "))"
                return ret
            raise Exception("unsupported For loop structure")

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
        "Assign": _Assign,
        "Attribute": _Attribute,
        "AugAssign": _AugAssign,
        "BinOp": _BinOp,
        "Call": _Call,
        "Compare": _Compare,
        "Expr": _Expr,
        "For": _For,
        "FunctionDef": _FunctionDef,
        "If": _If,
        "List": _List,
        "Module": _Module,
        "Name": _Name,
        "Num": _Num,
        "Return": _Return,
        "Slice": _Slice,
        "ExtSlice": _ExtSlice,
        "Str": _Str,
        "Subscript": _Subscript,
        "Tuple": _Tuple,
        "UnaryOp": _UnaryOp,
        "While": _While
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
        "vstack": "vstack"
        # slicing operations
    }


def convert_to_phylanx_type(v):
    t = type(v)
    try:
        import numpy
        if t == numpy.ndarray:
            return et.var(v)
    except NotImplementedError:
        pass
    return v


cs = phylanx.compiler_state()


# Create the decorator
def Phylanx(target="PhySL", compiler_state=cs, **kwargs):
    class PhyTransformer(object):
        targets = {"PhySL": PhySL, "OpenSCoP": OpenSCoP}

        def __init__(self, f):

            if "debug" in kwargs:
                self.debug = kwargs['debug']
            else:
                self.debug = False

            if target not in self.targets:
                raise NotImplementedError(
                    "unknown target passed to '@Phylanx()' decorator: %s." %
                    target)

            self.f = f
            self.cs = cs
            self.target = target

            # Get the source code
            actual_lineno = inspect.getsourcelines(f)[-1]
            src = inspect.getsource(f)
            # Before recompiling the code, take
            # off the decorator from the source.
            src = re.sub(r'^\s*@\w+.*\n', '', src)

            # Create the AST
            tree = ast.parse(src)
            ast.increment_lineno(tree, actual_lineno)
            assert len(tree.body) == 1
            self.transformation = self.targets[target](tree, kwargs)
            self.__src__ = self.transformation.__src__
            if self.debug:
                physl_fmt(self.__src__)

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

    return PhyTransformer
