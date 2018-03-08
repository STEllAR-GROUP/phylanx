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
from .utils import full_name, full_node_name

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


def rmline(a):
    return re.sub(r'\$.*', '', a)


class PhySL:
    def __init__(self):
        self.defs = {}
        self.priority = 0
        self.groupAggressively = True

    def _Num(self, a):
        return str(a.n)

    def _Str(self, a):
        return '"' + a.s + '"'

    def _Name(self, a):
        return full_node_name(a, a.id)

    def _Expr(self, a):
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = ""
        if len(args) == 1:
            s += self.recompile(args[0])
        else:
            raise Exception(
                'unexpected: expression has more than one sub-expression')
        return s

    def _Subscript(self, a):
        e = get_node(a, name="ExtSlice")
        s0 = get_node(e, name="Slice", num=0)
        s1 = get_node(e, name="Slice", num=1)
        s0alt = get_node(a, name="Slice", num=1)
        if s0 is not None and s1 is not None:
            xlo = s0.lower
            xlo_info = full_node_name(xlo)
            xhi = s0.upper
            xhi_info = full_node_name(xhi)
            ylo = s1.lower
            yhi = s1.upper
            yhi_info = full_node_name(yhi)
            sname = self.recompile(get_node(a, num=0))
            s = "slice%s(" % xlo_info
            s += sname
            s += ","
            if xlo is None:
                s += "0"
            else:
                s += self.recompile(xlo)
            s += ","
            if xhi is None:
                s += "shape%s(" % xhi_info + sname + ",0)"
            else:
                s += self.recompile(xhi)
            s += ","
            if ylo is None:
                s += "0"
            else:
                s += self.recompile(ylo)
            s += ","
            if yhi is None:
                s += "shape%s(" % yhi_info + sname + ",1)"
            else:
                s += self.recompile(yhi)
            s += ")"
            return s
        elif s0alt is not None:
            sname = self.recompile(get_node(a, num=0))
            xlo = s0alt.lower
            xlo_info = full_node_name(xlo)
            xhi = s0alt.upper
            xhi_info = full_node_name(xhi)
            s = 'slice_column%s(' % xlo_info
            s += sname
            s += ','
            if xlo is None:
                s += "0"
            else:
                s += self.recompile(xlo)
            s += ','
            if xhi is None:
                s += "shape%s(" % xhi_info + sname + ",0)"
            else:
                s += self.recompile(xhi)
            s += ")"
            return s
        else:
            raise Exception("Unsupported slicing: line=%d" % a.lineno)

    def _FunctionDef(self, a):
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

    def _BinOp(self, a):
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

    def _Call(self, a):
        symbol_info = full_node_name(a)
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

    def _Module(self, a):
        args = [arg for arg in ast.iter_child_nodes(a)]
        return self.recompile(args[0])

    def _Return(self, a, allowreturn=False):
        if not allowreturn:
            raise Exception(
                "Return only allowed at end of function: line=%d\n" % a.lineno)
        args = [arg for arg in ast.iter_child_nodes(a)]
        return " " + self.recompile(args[0])

    def _Assign(self, a):
        symbol_info = full_node_name(a)
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = ""
        if args[0].id in self.defs:
            s += "store" + symbol_info
        else:
            s += "define" + symbol_info
            self.defs[args[0].id] = 1
        a = '%s' % full_node_name(args[0], args[0].id)
        s += "(" + a + "," + self.recompile(args[1]) + ")"
        return s

    def _AugAssign(self, a):
        symbol_info = full_node_name(a)
        args = [arg for arg in ast.iter_child_nodes(a)]
        sym = "?"
        nn = args[1].__class__.__name__
        if nn == "Add":
            sym = "+"
        arg0 = '%s' % full_node_name(args[0], args[0].id)
        return "store%s(" % symbol_info + arg0 + "," + arg0 + sym + self.recompile(
            args[2]) + ")"

    def _While(self, a):
        symbol_info = full_node_name(a)
        args = [arg for arg in ast.iter_child_nodes(a)]
        s = "while(" + self.recompile(args[0]) + ","
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
        s = "if(" + self.recompile(a.test) + ","
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
        s += ","
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

    def _Compare(self, a):
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
        return self.recompile(args[0]) + sym + self.recompile(args[2])

    def _UnaryOp(self, a):
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

    def _For(self, a):
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
                    bs += ","
                bs += self.recompile(blockn)
                blocki += 1
            bs += ")"
            # The first arg of the for loop is either
            # a define() or a store(), depending on whether
            # the variable has already been defined.
            sd = "store" + symbol_info
            if rmline(ns) not in self.defs:
                sd = "define" + symbol_info
            lg = "<"
            # Determine the direction of iteration, if possible
            if not re.search(r'[a-zA-Z_]', ss):
                if eval(ss) < 0:
                    lg = ">"
                return "for(" + sd + "(" + ns + "," + ls + ")," + ns + " " + lg + " " \
                       + us + ",store%s(" % symbol_info + ns + "," + ns + \
                    "+" + ss + ")," + bs + ")"
            else:
                # if we can't determine the direction of iteration, make two for loops
                ret = "if(" + ss + " > 0,"
                ret += "for(" + sd + "(" + ns + "," + ls + ")," + ns + " < " + us + \
                    ",store%s(" % symbol_info + ns + "," + ns + "+" + ss + ")," + bs \
                    + "),"
                ret += "for(" + sd + "(" + ns + "," + ls + ")," + ns + " > " + us + \
                    ",store%s(" % symbol_info + ns + "," + ns + "+" + ss + ")," + bs \
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
        "Num": _Num,
        "Str": _Str,
        "Name": _Name,
        "Expr": _Expr,
        "Subscript": _Subscript,
        "FunctionDef": _FunctionDef,
        "BinOp": _BinOp,
        "Call": _Call,
        "Module": _Module,
        "Return": _Return,
        "Assign": _Assign,
        "AugAssign": _AugAssign,
        "While": _While,
        "If": _If,
        "Compare": _Compare,
        "UnaryOp": _UnaryOp,
        "For": _For
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


# Create the decorator
def Phylanx(target="PhySL"):
    class PhyTransformer(object):
        targets = {"PhySL": PhySL}

        def __init__(self, f):
            self.f = f
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
            transformation = self.targets[target]()

            assert len(tree.body) == 1

            if target == "PhySL":
                self.__physl_src__ = transformation.recompile(tree)
                et.eval(self.__physl_src__)
            else:
                raise Exception(
                    "Invalid target to Phylanx transformer: '%s'" % target)

        def __call__(self, *args):
            nargs = tuple(convert_to_phylanx_type(a) for a in args)
            return et.eval(self.__physl_src__, *nargs)

    return PhyTransformer
