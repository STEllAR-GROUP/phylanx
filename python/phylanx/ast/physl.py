# Copyright (c) 2017 Hartmut Kaiser
# Copyright (c) 2018 Steven R. Brandt
# Copyright (c) 2018-2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import re
import ast
import inspect
import numpy as np
import phylanx.execution_tree
from phylanx import PhylanxSession


def physl_zip(loop):
    def define(i, idx):
        return ['define', (i, ['slice', ('__physl_iterator', str(idx))])]

    if isinstance(loop, ast.For):
        targets = [it.id for it in loop.target.elts]
        args = [arg.id for arg in loop.iter.args]
    elif isinstance(loop, list):
        targets = loop[0]
        args = loop[1][1]

    lambda_ = ['lambda', (*targets, ['list', (*targets, )])]
    fmap = ['fmap', (lambda_, *args)]
    iterators = tuple(define(i, idx) for idx, i in enumerate(targets))
    return (fmap, iterators)


mapped_methods = {
    "add": "__add",
    "array": "hstack",
    "det": "determinant",
    "diagonal": "diag",
    "divide": "__div",
    "matmul": "__mul",
    "multiply": "__mul",
    "negative": "__minus",
    "print": "cout",
    "subtract": "__sub"
}

numpy_constants = {
    "inf": 'inf',
    "Inf": 'inf',
    "Infinity": 'inf',
    "PINF": 'inf',
    "infty": 'inf',
    "NINF": 'ninf',
    "nan": 'nan',
    "NaN": 'nan',
    "NAN": 'nan',
    "PZERO": 'PZERO',
    "NZERO": 'NZERO',
    "e": 'euler',
    "euler_gamma": 'euler_gamma',
    "pi": 'pi',
    "float": 'float',
    "int": 'int',
    "bool": 'bool'
}

methods_supporting_dtype = [
    'absolute',
    'arccos',
    'arccosh',
    'arcsin',
    'arcsinh',
    'arctan',
    'arctanh',
    'cbrt',
    'ceil',
    'conj',
    'cos',
    'cosh',
    'erf',
    'erfc',
    'square',
    'sign',
    'exp',
    'exp2',
    'exp10',
    'floor',
    'identity',
    'imag',
    'invcbrt',
    'invsqrt',
    'linearmatrix',
    'linspace',
    'log',
    'log2',
    'log10',
    'mean',
    'normalize',
    'power',
    'prod',
    'real',
    'rint',
    'sin',
    'sinh',
    'sqrt',
    'tan',
    'tanh',
    'trace'
    'trunc',
]


def create_array(array_tree, kwargs):
    symbol_info = []

    hstack_symbol = 'hstack'
    vstack_symbol = 'vstack'
    dstack_symbol = 'dstack'

    def extract_data(arr):
        if isinstance(arr, tuple):
            if not arr:
                return []
            elif isinstance(arr[0], str):
                return [i for i in arr]
            else:
                current_dim = []
                for entry in arr:
                    current_dim.append(extract_data(entry))
                return current_dim
        elif isinstance(arr, list):
            symbol_info.append('$' + arr[0].split('$', 1)[1])
            return extract_data(arr[1])

    data = extract_data(array_tree)

    if not symbol_info:
        if kwargs:
            args = (['list', tuple(data)], kwargs)
        else:
            args = (['list', tuple(data)], )
        return [hstack_symbol, args]

    data = np.array(*extract_data(array_tree))
    num_dim = len(data.shape)

    if 3 == num_dim:
        columns = [data[:, :, i] for i in range(data.shape[-1])]
        dstacks = []
        for i, column in enumerate(columns):
            dstacks.append([])
            if kwargs:
                [dstacks[i].append((['list', tuple(data)], kwargs)) for data in column]
            else:
                [dstacks[i].append((['list', tuple(data)], )) for data in column]

        outer_symbol = '' if not symbol_info else symbol_info.pop(0)
        arr = []
        for d in dstacks:
            vstack = []
            for hstacks in d:
                vstack.append([hstack_symbol + symbol_info.pop(0), hstacks])
            sym_info = '' if not symbol_info else symbol_info.pop(0)
            if kwargs:
                args = (['list', tuple(vstack)], kwargs)
            else:
                args = (['list', tuple(vstack)], )
            vstack = [vstack_symbol + sym_info, args]
            arr.append(vstack)
        if kwargs:
            args = (['list', tuple(arr)], kwargs, )
        else:
            args = (['list', tuple(arr)], )
        arr = [dstack_symbol + outer_symbol, args]
    elif 2 == num_dim:
        arr = []
        for hstacks in data:
            sym_info = '' if not symbol_info else symbol_info.pop(0)
            if kwargs:
                args = (['list', tuple(hstacks)], kwargs)
            else:
                args = (['list', tuple(hstacks)], )
            arr.append([hstack_symbol + sym_info, args])
        sym_info = '' if not symbol_info else symbol_info.pop(0)
        if kwargs:
            args = (['list', tuple(arr)], kwargs)
        else:
            args = (['list', tuple(arr)], )
        arr = [vstack_symbol + sym_info, args]
    elif 1 == num_dim:
        sym_info = '' if not symbol_info else symbol_info.pop(0)
        if kwargs:
            args = (['list', tuple(data)], kwargs)
        else:
            args = (['list', tuple(data)], )
        arr = [hstack_symbol + sym_info, args]
    else:
        ValueError("Phylanx supports arrays with 3 dimensions or less.")
    return (arr,)


def primitive_name(method_name):
    """Given a method_name, returns the corresponding Phylanx primitive.

    This primarily used for mapping NumPy mapped_methods to Phylanx primitives,
    but there are also other functions in python that would map to primitives
    with different name in Phylanx, e.g., `print` is mapped to `cout`.
    """

    primitive_name = mapped_methods.get(method_name)
    if primitive_name:
        return primitive_name

    constant_name = numpy_constants.get(method_name)
    if constant_name:
        return constant_name

    return method_name


def print_physl_src(src, with_symbol_info=False, tag=4):
    """Pretty print PhySL source code."""

    # Remove line number info
    src = re.sub(r'\$\d+', '', src)

    if with_symbol_info:
        print(src)
        return

    # The regex below matches one of the following three
    # things in order of priority:
    # 1: a quoted string, with possible \" or \\ embedded
    # 2: a set of balanced parenthesis
    # 3: a single character
    pat = re.compile(r'"(?:\\.|[^"\\])*"|\([^()]*\)|.')
    indent = 0
    tab = 4
    for s in re.findall(pat, src):
        if s in " \t\r\b\n":
            pass
        elif s == '(':
            print(s)
            indent += 1
            print(" " * indent * tab, end="")
        elif s == ')':
            indent -= 1
            print("", sep="")
            print(" " * indent * tab, end="")
            print(s, end="")
        elif s == ',':
            print(s)
            print(" " * indent * tab, end="")
        else:
            print(s, end="", sep="")
    print("", sep="")


def get_symbol_info(symbol, name):
    """Adds symbol info (line and column number) to the symbol."""

    if name in numpy_constants.keys():
        return name
    else:
        return '%s$%d$%d' % (name, symbol.lineno, symbol.col_offset)


def remove_line(a):
    return re.sub(r'\$.*', '', a)


def is_fun(func, ir):
    """
    Check that the intermediate representation (ir) describes
    a function with name func.
    """
    return type(ir) == list and type(ir[0]) == str and re.match(func + r'\b', ir[0])


def check_noreturn(ir):
    """
    Check that the intermediate representation (ir) passed
    to this routine does not contain ir return statement.
    """
    if type(ir) not in [list, tuple]:
        return
    if len(ir) == 0:
        return
    elif len(ir) == 1:
        check_noreturn(ir[0])
    elif is_fun('define', ir):
        check_hasreturn(ir)
    elif is_fun('return', ir):
        msg = "Illegal return"
        g = re.match(r'.*\$(\d+)\$(\d+)$', str(ir[0]))
        if g:
            msg += ": line=%s, col=%s" % (g.group(1), g.group(2))
        raise NotImplementedError(msg)
    elif is_fun('.*', ir):
        check_noreturn(ir[1])
    elif type(ir) in [list, tuple]:
        for s in ir:
            check_noreturn(s)


def check_hasreturn(ir):
    """
    Process the intermediate representation (ir) passed
    and ensure that if it has ir return statement, it is
    at the end.
    """
    if type(ir) not in [list, tuple]:
        return
    if len(ir) == 0:
        return
    elif len(ir) == 1:
        check_hasreturn(ir[0])
    elif is_fun('for_each', ir):
        check_noreturn(ir[1])
    elif is_fun('while', ir):
        check_noreturn(ir[1])
    elif is_fun('if', ir):
        for k in ir[1][1:]:
            check_hasreturn(k)
    elif is_fun('.*', ir):
        check_hasreturn(ir[1])
    else:
        if len(ir) == 0:
            return
        check_noreturn(ir[:-1])
        check_hasreturn([ir[-1]])


def check_return(ir):
    """
    Process the intermediate representation (ir) passed
    and check that return statements are only used where
    allowed.
    """
    if type(ir) not in [list, tuple]:
        return
    if len(ir) == 0:
        return
    elif len(ir) == 1:
        check_return(ir[0])
    elif is_fun('block', ir):
        check_hasreturn(ir[1])
    elif is_fun('while', ir):
        check_noreturn(ir[1])
    elif is_fun('if', ir):
        for k in ir[1][1:]:
            check_hasreturn(k)
    elif is_fun('.*', ir):
        check_return(ir[1])
    else:
        for s in ir:
            check_return(s)


class PhySLFunction:

    functions = []

    def __init__(self, file_name, function_name, src):
        self.file_name = file_name
        self.func_name = function_name
        self.src = src

    def compile_function(self):
        if PhySL.compiler_state is None:
            PhySL.compiler_state = phylanx.execution_tree.compiler_state(
                self.file_name)
        phylanx.execution_tree.compile(PhySL.compiler_state, self.file_name,
                                       self.func_name, self.src)

    @staticmethod
    def compile():
        for func in PhySLFunction.functions:
            func.compile_function()
        PhySLFunction.functions = []


class PhySL:
    """Python AST to PhySL Transducer."""

    compiler_state = None

    def __init__(self, func, tree, kwargs):
        self.defined = set()
        self.numpy_aliases = {'numpy'}
        self.wrapped_function = func
        self.kwargs = kwargs
        self.is_compiled = False
        self.file_name = None
        if self.kwargs.get('fglobals'):
            self.fglobals = self.kwargs['fglobals']
            for key, val in self.fglobals.items():
                if type(val).__name__ == 'module' and val.__name__ == 'numpy':
                    self.numpy_aliases.add(key)
            self.file_name = self.fglobals.get('__file__')
        if not self.file_name:
            self.file_name = "<none>"

        self.performance = self.kwargs.get('performance', False)
        self.localities = self.kwargs.get('localities')
        self.__perfdata__ = (None, None, None)

        # Add arguments of the function to the list of discovered variables.
        if inspect.isfunction(tree.body[0]):
            for arg in tree.body[0].args.args:
                self.defined.add(arg.arg)
        else:
            PhySL.defined_classes = {}

        self.ir = self.apply_rule(tree.body[0])
        check_return(self.ir)
        self.__src__ = self.generate_physl(self.ir)

        if self.kwargs.get("debug"):
            print_physl_src(self.__src__)
            print(end="", flush="")

        if PhylanxSession.is_initialized:
            if "compiler_state" in self.kwargs:
                PhySL.compiler_state = self.kwargs['compiler_state']
            # the static method compiler_state is constructed only once
            elif PhySL.compiler_state is None:
                PhySL.compiler_state = phylanx.execution_tree.compiler_state(
                    self.file_name)

            PhySLFunction.compile()

            phylanx.execution_tree.compile(
                PhySL.compiler_state, self.file_name,
                self.wrapped_function.__name__, self.__src__)

            self.is_compiled = True
        else:
            func_name = self.wrapped_function.__name__
            func = PhySLFunction(self.file_name, func_name, self.__src__)
            PhySLFunction.functions.append(func)

    def generate_physl(self, ir):
        if len(ir) == 2 and isinstance(ir[0], str) and isinstance(
                ir[1], tuple):
            result = [self.generate_physl(i) for i in ir[1]]
            # Remove return statements when generating physl
            if re.match(r'return\$.*', ir[0]):
                if len(result) == 1:
                    return result[0]
                else:
                    return '(' + ', '.join(result) + ')'
            else:
                return ir[0] + '(' + ', '.join(result) + ')'
        elif isinstance(ir, list):
            return ', '.join([self.generate_physl(i) for i in ir])
        elif isinstance(ir, tuple):
            # NOTE Phylanx does not support tuples at this point, therefore, we
            # unpack all tuples for now!
            return ', '.join([self.generate_physl(i) for i in ir])
        else:
            return ir

    def apply_rule(self, node):
        """Calls the corresponding rule, based on the name of the node."""
        if node is not None:
            node_name = node.__class__.__name__
            return eval('self._%s' % node_name)(node)

    def block(self, node):
        """Returns a map representation of a PhySL block."""

        if isinstance(node, list):
            block = tuple(map(self.apply_rule, node))
            if len(node) == 1:
                return block
            else:
                return ['block', block]
        else:
            block = (self.apply_rule(node), )
            return block

    class eval_wrapper:
        """evaluation wrapper binding arguments to a compiled function"""

        def __init__(self, outer, *args, **kwargs):
            """initialize evaluation wrapper"""
            self.outer = outer
            self.args = args
            self.kwargs = kwargs
            self.func_name = self.outer.wrapped_function.__name__

        def eval(self):
            """evaluate given compiled function using the bound arguments"""

            self.outer.__perfdata__ = (None, None, None)
            self.outer.performance_primitives = None

            if self.outer.performance:
                self.outer.performance_primitives = \
                    phylanx.execution_tree.enable_measurements(
                        PhySL.compiler_state, True)

            result = phylanx.execution_tree.eval(
                PhySL.compiler_state, self.outer.file_name, self.func_name,
                *self.args, **self.kwargs)

            if self.outer.performance:
                treedata = phylanx.execution_tree.retrieve_tree_topology(
                    PhySL.compiler_state, self.outer.file_name, self.func_name)
                self.outer.__perfdata__ = (
                    phylanx.execution_tree.retrieve_counter_data(
                        PhySL.compiler_state),
                    treedata[0], treedata[1]
                )

            return result

        def code(self):
            """Expose the wrapped Phylanx primitive, either directly or
               with its arguments bound"""

            return self.outer.map_wrapped(self)

    def map_wrapped(self, val):
        """If a eval_wrapper is passed as an argument to an
            invocation of a Phylanx function we need to extract the
            compiled execution tree and pass along that instead"""

        if isinstance(val, self.eval_wrapper):
            if len(val.args) == 0:
                return phylanx.execution_tree.code_for(
                    PhySL.compiler_state, self.file_name, val.func_name)

            args = tuple(map(self.map_wrapped, val.args))
            return phylanx.execution_tree.bound_code_for(
                PhySL.compiler_state, self.file_name, val.func_name, *args)

        return val

    def lazy(self, *args, **kwargs):
        """Compile a given function, return wrapper binding the function to
           arguments"""

        if not PhylanxSession.is_initialized:
            PhylanxSession.init(1)

        if not self.is_compiled:
            PhySLFunction.compile()
            if "compiler_state" in self.kwargs:
                PhySL.compiler_state = self.kwargs['compiler_state']
            elif PhySL.compiler_state is None:
                PhySL.compiler_state = phylanx.execution_tree.compiler_state(
                    self.file_name)

            phylanx.execution_tree.compile(
                PhySL.compiler_state, self.file_name,
                self.wrapped_function.__name__, self.__src__)

            self.is_compiled = True

        mapped_args = tuple(map(self.map_wrapped, args))
        kwitems = kwargs.items()
        mapped_kwargs = {k: self.map_wrapped(v) for k, v in kwitems}
        return self.eval_wrapper(self, *mapped_args, **mapped_kwargs)

    def call(self, *args, **kwargs):
        """Invoke this Phylanx function, pass along the given arguments"""

        return self.lazy(*args, **kwargs).eval()

# #############################################################################
# Transducer rules

    def _Add(self, node):
        """Leaf node, returning raw string of the `add` operation."""

        return '__add'

    def _And(self, node):
        """Leaf node, returning raw string of the `and` operation."""

        return '__and'

    def _arg(self, node):
        """class arg(arg, annotation)

        A single argument in a list.

        `arg` is a raw string of the argument name.
        `annotation` is its annotation, such as a `Str` or `Name` node.

        TODO:
            add support for `annotation` which is ignored at this time. Maybe
            we can use this to let user provide type information!?!
        """

        arg = get_symbol_info(node, node.arg)
        return arg

    def _arguments(self, node):
        """class arguments(args, vararg, kwonlyargs, kwarg, defaults, kw_defaults)

        The arguments for a function.
        `args` and `kwonlyargs` are lists of arg nodes.
        `vararg` and `kwarg` are single arg nodes, referring to the *args,
        **kwargs parameters.
        `defaults` is a list of default values for arguments that can be passed
        positionally. If there are fewer defaults, they correspond to the last
        n arguments.
        `kw_defaults` is a list of default values for keyword-only arguments. If
        one is None, the corresponding argument is required.
        """
        if node.vararg or node.kwarg:
            raise (Exception("Phylanx does not support *args and **kwargs"))

        defaults = tuple(map(self.apply_rule, node.defaults))
        result = tuple()
        padded_defaults = (None,) * (len(node.args) - len(defaults)) + defaults
        for arg, default in zip(node.args, padded_defaults):
            a = self.apply_rule(arg)
            symbol_name = re.sub(r'\$\d+', '', a)
            self.defined.add(symbol_name)
            if default is None:
                result = (*result, a)
            else:
                op = get_symbol_info(arg, '__arg')
                result = (*result, [op, (a, default)])
        return result

    def _Assign(self, node):
        """class Assign(targets, value)

        `targets` is a list of nodes which are assigned a value.
        `value` is a single node which gets assigned to `targets`.

        TODO:
            Add support for multi-target (a,b, ... = iterable) and chain (a = b = ... )
            assignments.
        """

        if len(node.targets) > 1:
            raise Exception("Phylanx does not support chain assignments.")
        if isinstance(node.targets[0], ast.Tuple):
            raise Exception(
                "Phylanx does not support multi-target assignments.")

        symbol = self.apply_rule(node.targets[0])
        # if lhs is not indexed.
        if isinstance(symbol, str):
            symbol_name = re.sub(r'\$\d+', '', symbol)
            if symbol_name in self.defined:
                op = get_symbol_info(node.targets[0], "store")
            else:
                op = get_symbol_info(node.targets[0], "define")
                # TODO:
                # For now `self.defined` is a set containing names of symbols
                # with no extra information. We may want to make it a
                # dictionary with the symbol names as keys and list of
                # symbol_infos to keep track of the symbol.
                self.defined.add(symbol_name)
        # lhs is a subscript.
        else:
            op = get_symbol_info(node.targets[0], "store")

        target = self.apply_rule(node.targets[0])
        value = self.apply_rule(node.value)
        return [op, (target, value)]

    def _Attribute(self, node):
        """class Attribute(value, attr, ctx)

        `value` is an AST node.
        `attr` is a bare string giving the name of the attribute.
        """

        method_name = get_symbol_info(node, primitive_name(node.attr))

        namespace = [node.attr]
        current_node = node.value
        while isinstance(current_node, ast.Attribute):
            namespace.insert(0, current_node.attr)
            current_node = current_node.value
        namespace.insert(0, current_node.id)

        if isinstance(current_node, ast.Name):
            if namespace[0] in self.numpy_aliases:
                return method_name
            else:
                attr = '.'.join(namespace)
                raise NotImplementedError(
                    'Phylanx does not support non-NumPy member functions.'
                    'Cannot transform: %s' % attr)

    def _AugAssign(self, node):
        """class AugAssign(target, op, value)"""

        symbol = get_symbol_info(node, 'store')
        op = get_symbol_info(node, self.apply_rule(node.op))
        target = self.apply_rule(node.target)
        value = self.apply_rule(node.value)

        return [symbol, (target, (op, (target, value)))]

    def _BinOp(self, node):
        """class BinOp(left, op, right)"""

        op = get_symbol_info(node, self.apply_rule(node.op))
        left = self.apply_rule(node.left)
        right = self.apply_rule(node.right)
        return [op, (left, right)]

    def _BoolOp(self, node):
        """class BoolOp(left, op, right)"""

        op = get_symbol_info(node, self.apply_rule(node.op))
        values = list(map(self.apply_rule, node.values))

        return [op, (values, )]

    def _Call(self, node):
        """class Call(func, args, keywords, starargs, kwargs)

        TODO(?):
            Add support for keywords, starargs, and kwargs
        """

        def __apply(self, k):
            kw = self.apply_rule(k.value)
            return (k.arg, kw)

        symbol = self.apply_rule(node.func)
        args = tuple(self.apply_rule(arg) for arg in node.args)
        op = get_symbol_info(node.func, '__arg')
        kwargs = tuple([op, __apply(self, k)] for k in node.keywords)

        dtype = ''

        # TODO: these are workarounds for the cases that Phylanx does not
        # follow NumPy functions' signatures.
        # +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if 'hstack' in symbol:
            return create_array(args, kwargs)
        elif 'vstack' in symbol:
            if args and isinstance(args[0], tuple):
                args = (['list', (tuple(args), )],)
        elif 'dstack' in symbol:
            if args and isinstance(args[0], tuple):
                args = (['list', (tuple(args), )],)

        elif 'zeros_like' in symbol:
            symbol = symbol.replace('zeros_like', 'constant_like')
            return [symbol, ('0', args + kwargs)]
        elif 'ones_like' in symbol:
            symbol = symbol.replace('ones_like', 'constant_like')
            return [symbol, ('1', args + kwargs)]
        elif 'full_like' in symbol:
            symbol = symbol.replace('full_like', 'constant_like')
            return [symbol, (args[1], (args[0], ) + kwargs)]
        elif 'empty_like' in symbol:
            symbol = symbol.replace('empty_like', 'constant_like')
            return [symbol, (None, args + kwargs)]

        elif 'zeros' in symbol:
            symbol = symbol.replace('zeros', 'constant')
            if isinstance(args[0], tuple):
                op = get_symbol_info(node.func, 'list')
                return [symbol, ('0', [op, args[0]]) + kwargs]
            else:
                return [symbol, ('0', args + kwargs)]
        elif 'ones' in symbol:
            symbol = symbol.replace('ones', 'constant')
            if isinstance(args[0], tuple):
                op = get_symbol_info(node.func, 'list')
                return [symbol, ('1', [op, args[0]]) + kwargs]
            else:
                return [symbol, ('1', args + kwargs)]
        elif 'full' in symbol:
            symbol = symbol.replace('full', 'constant')
            if isinstance(args[0], tuple):
                op = get_symbol_info(node.func, 'list')
                return [symbol, (args[1], [op, args[0]]) + kwargs]
            else:
                return [symbol, (args[1], (args[0], ) + kwargs)]
        elif 'empty' in symbol:
            symbol = symbol.replace('empty', 'constant')
            if isinstance(args[0], tuple):
                op = get_symbol_info(node.func, 'list')
                return [symbol, (None, [op, args[0]]) + kwargs]
            else:
                return [symbol, (None, args + kwargs)]

        else:
            method = [m for m in methods_supporting_dtype if symbol.find(m, 0) == 0]
            if len(method) == 1:
                symbol = symbol.replace(method[0], method[0] + dtype)

        # +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        return [symbol, args + kwargs]

    # def _ClassDef(self, node):
    #     """class ClassDef(name, bases, keywords, starargs, kwargs, body,
    #                       decorator_list)

    #     `name` is a raw string for the class name.
    #     `bases` is a list of nodes for explicitly specified base classes.
    #     `keywords` is a list of keyword nodes, principally for `metaclass`.
    #         Other keywords will be passed to the metaclass, as per PEP-3115.
    #     `starargs` removed in python 3.5.
    #     `kwargs`   removed in Python 3.5.
    #     `body` is a list of nodes representing the code within the class
    #         definition.
    #     `decorator_list` is the list of decorators to be applied, stored
    #         outermost first (i.e. the first in the list will be applied last).
    #     """
    #     PhySL.defined_classes[node.name] = {}
    #     if node.bases:
    #         raise NotImplementedError("Phylanx does not support inheritance.")
    #     class_body = list(self.apply_rule(m) for m in node.body)

    #     return class_body

    def _Compare(self, node):
        """class Compare(left, ops, comparators)

        A comparison of two or more values.
        `left` is the first value in the comparison
        `ops` is the list of operators
        `comparators` is the list of values after the first (`left`).
        """

        if (len(node.ops) == 1):
            left = self.apply_rule(node.left)
            op = get_symbol_info(node, self.apply_rule(node.ops[0]))
            right = self.apply_rule(node.comparators[0])

            return [op, (left, right)]
        else:
            # if we're dealing with more than one comparison, we canonicalize the
            # comparisons in to the form of chained logical ands. e.g., a < b < c
            # becomes: ([__and ((__lt b, c), (__lt a, b))])
            # TODO: Make sure to respect Python operator precedence.
            comparison = []
            for i in range(len(node.ops)):
                op = self.apply_rule(node.ops[-i])
                left = self.apply_rule(node.comparators[-i - 1])
                right = self.apply_rule(node.comparators[-i])
                if comparison:
                    comparison = ['__and', (*comparison, (op, (left, right)))]
                else:
                    comparison = [*comparison, (op, (left, right))]

            op = self.apply_rule(node.ops[0])
            left = self.apply_rule(node.left)
            right = self.apply_rule(node.comparators[0])
            if comparison:
                comparison = ['__and', (*comparison, (op, (left, right)))]
            else:
                comparison = [op, (left, right)]

            return comparison

    def _comprehension(self, node):
        """class comprehension(target, iter, ifs, is_async)

        1 for clause in a comprehension.
        `target` is the reference to use in each element- a `name` or `Tuple`.
        `iter` is the object to iterate over.
        `ifs` is a list of test expressions (a for clause may have multiple ifs).
        `is_async` indicates a comprehension is asynchronous.
        """

        target = self.apply_rule(node.target)
        iteration_space = self.apply_rule(node.iter)

        comprehension = {
            'target': target,
            'iter': iteration_space
        }

        return comprehension

    def _Div(self, node):
        """Leaf node, returning raw string of the 'division' operation."""

        return '__div'

    def _Eq(self, node):
        """Leaf node, returning raw string of the 'equality' operation."""

        return '__eq'

    def _Expr(self, node):
        """class Expr(value)

        `value` holds one of the other nodes (rules).
        """

        return self.apply_rule(node.value)

    def _ExtSlice(self, node):
        """class ExtSlice(dims)

        Advanced slicing.
        `dims` holds a list of `Slice` and `Index` nodes.
        """
        slicing = list(map(self.apply_rule, node.dims))
        return slicing

    def _For(self, node):
        """class For(target, iter, body, orelse)

        A for loop.
        `target` holds the variable(s) the loop assigns to, as a single Name,
            Tuple or List node.
        `iter` holds the item to be looped over, again as a single node.
        `body` contain lists of nodes to execute.
        `orelse` same as `body`, however, those in orelse are executed if the
            loop finishes normally, rather than via a break statement.
        """

        # this lookup table helps us to choose the right mapping function based on the
        # type of the iteration space (list, range, or prange).
        mapping_function = {
            'list': 'for_each',
            'slice': 'for_each',
            'range': 'for_each',
            'prange': 'parallel_map'
        }

        target = self.apply_rule(node.target)
        # TODO: **MAP**
        # target_name = target.split('$', 1)[0]
        # self.defined.add(target_name)
        iteration_space = self.apply_rule(node.iter)
        if isinstance(iteration_space, list) and iteration_space[0].startswith('zip'):
            iter_space, indices = physl_zip(node)
            symbol = get_symbol_info(node, 'for_each')
            body = self.block(node.body)
            body = ['block', (*indices, *body)]
            op = get_symbol_info(node, 'lambda')
            return [symbol, ([op, ('__physl_iterator', body)], iter_space)]

        # extract the type of the iteration space- used as the lookup key in
        # `mapping_function` dictionary above.
        if isinstance(iteration_space, list):
            symbol_name = mapping_function[iteration_space[0].split('$', 1)[0]]
            symbol = get_symbol_info(node, symbol_name)
            # replace keyword `prange` to `range` for compatibility with Phylanx.
            iteration_space[0] = iteration_space[0].replace('prange', 'range')
        else:
            symbol = get_symbol_info(node, 'for_each')

        body = self.block(node.body)
        # orelse = self.block(node.orelse)
        op = get_symbol_info(node, 'lambda')
        return [symbol, ([op, (target, body)], iteration_space)]
        # return [symbol, (target, iteration_space, body, orelse)]

    def _FunctionDef(self, node):
        """class FunctionDef(name, args, body, decorator_list, returns)

        `name` is a raw string of the function name.
        `args` is a arguments node.
        `body` is the list of nodes inside the function.
        `decorator_list` is the list of decorators to be applied, stored
            outermost first (i.e. the first in the list will be applied last).
        `returns` is the return annotation (Python 3 only).

        Notes:
            We ignore decorator_list and returns.
        """

        op = get_symbol_info(node, 'define')
        symbol = get_symbol_info(node, node.name)
        args = self.apply_rule(node.args)
        body = self.block(node.body)
        lambda_op = get_symbol_info(node, 'lambda')

        if (args):
            return [op, (symbol, args, body)]
        else:
            return [op, (symbol, (lambda_op, (body,)))]

    def _Gt(self, node):
        """Leaf node, returning raw string of the 'greater than' operation."""

        return '__gt'

    def _GtE(self, node):
        """Leaf node, returning raw string of the 'greater than or equal' operation."""

        return '__ge'

    def _If(self, node):
        """class If(test, body, orelse)

       `test` holds a single node, such as a Compare node.
       `body` and `orelse` each hold a list of nodes.
       """

        symbol = '%s' % get_symbol_info(node, 'if')
        test = self.apply_rule(node.test)
        body = self.block(node.body)
        orelse = self.block(node.orelse)

        return [symbol, (test, body, orelse)]

    def _IfExp(self, node):
        """class IfExp(test, body, orelse)

       `test` holds a single node, such as a Compare node.
       `body` and `orelse` each hold a list of nodes.
       """

        symbol = '%s' % get_symbol_info(node, 'if')
        test = self.apply_rule(node.test)
        body = self.block(node.body)
        orelse = self.block(node.orelse)

        return [symbol, (test, body, orelse)]

    def _In(self, node):
        raise Exception("`In` operator is not defined in Phylanx.")

    def _Index(self, node):
        """class Index(value)

        Simple subscripting with a single value.
        """

        # TODO: **SLICING**
        # if isinstance(node.value, ast.Tuple):
        #     op = 'list'
        #     elements = tuple(map(self.apply_rule, node.value.elts))
        #     return [op, (*elements, )]
        # else:
        #     return self.apply_rule(node.value)
        return self.apply_rule(node.value)

    def _Is(self, node):
        raise Exception("`Is` operator is not defined in Phylanx.")

    def _IsNot(self, node):
        raise Exception("`IsNot` operator is not defined in Phylanx.")

    def _Lambda(self, node):
        """class Lambda(args, body)

        `body` is a single node.
        """
        symbol = get_symbol_info(node, 'lambda')
        args = self.apply_rule(node.args)
        body = self.block(node.body)

        return [symbol, (args, body)]

    def _List(self, node):
        """class List(elts, ctx)"""

        op = get_symbol_info(node, 'list')
        elements = tuple(map(self.apply_rule, node.elts))
        return [op, (*elements, )]

    def _ListComp(self, node):
        """class ListComp(elt, generators)

        `elt` (or key and value) is a single node representing the part that
              will be evaluated for each item.
        `generators` is a list of comprehension nodes.
        """

        if len(node.generators) > 1:
            raise NotImplementedError("Nested comprehensions is not yet supported!")

        elt = self.apply_rule(node.elt)
        loop = self.apply_rule(node.generators[0])

        target = loop['target']
        iter_space = loop['iter']
        if isinstance(iter_space, list) and iter_space[0].startswith('zip'):
            iter_space, iterators = physl_zip([target, iter_space])
            symbol = get_symbol_info(node, 'fmap')
            body = ['block', (*iterators, elt)]
            op = get_symbol_info(node, 'lambda')
            return [symbol, ([op, ('__physl_iterator', body)], iter_space)]

        lambda_ = ['lambda', (target, elt)]
        fmap = ['fmap', (lambda_, iter_space)]

        return fmap

    def _Lt(self, node):
        """Leaf node, returning raw string of the 'less than' operation."""

        return '__lt'

    def _LtE(self, node):
        """Leaf node, returning raw string of the 'less than or equal' operation."""

        return '__le'

    def _Module(self, node):
        """Root node of the Python AST."""
        module = list(self.apply_rule(m) for m in node.body)

        return module

    def _Mult(self, node):
        """Leaf node, returning raw string of the 'multiplication' operation."""

        return '__mul'

    def _Name(self, node):
        """class Name(id, ctx)

        A variable name.
        `id` holds the name as a string.
        `ctx` is one of `Load`, `Store`, `Del`.
        """

        symbol = get_symbol_info(node, primitive_name(node.id))
        return symbol

    def _NameConstant(self, node):
        name_constants = {None: 'nil', False: 'false', True: 'true'}
        return name_constants[node.value] + get_symbol_info(node, '')

    def _Not(self, node):
        """Leaf node, returning raw string of the 'not' operation."""

        return '__not'

    def _NotEq(self, node):
        """Leaf node, returning raw string of the 'not equal' operation."""

        return '__ne'

    def _NotIn(self, node):
        raise Exception("`NotIn` operator is not defined in Phylanx.")

    def _Num(self, node):
        """class Num(n)"""

        return str(node.n)

    def _Or(self, node):
        """Leaf node, returning raw string of the 'or' operation."""

        return '__or'

    def _Pass(self, node):
        """Empty function."""

        return 'nil'

    def _Pow(self, node):
        """Leaf node, returning raw string of the 'power' operation."""

        return 'power'

    def _Return(self, node):
        """class Return(value)

        TODO:
            implement return-from primitive (see section Function Return Values on
            https://goo.gl/wT6X4P). At this time Phylanx only supports returns from the
            end of the function!
        """

        symbol = get_symbol_info(node, "return")

        if type(node.value) == ast.Tuple:
            return [symbol, (["list", self.apply_rule(node.value)],)]

        value = self.apply_rule(node.value)
        if value is None:
            value = get_symbol_info(node, "nil")

        return [symbol, (value,)]

    def _Slice(self, node):
        """class Slice(lower, upper, step)"""

        symbol = 'list'

        lower = self.apply_rule(node.lower)
        if lower is None:
            lower = 'nil'

        upper = self.apply_rule(node.upper)
        if upper is None:
            upper = 'nil'

        step = self.apply_rule(node.step)
        if step is None:
            slice_ = self.generate_physl([symbol, (lower, upper)])
        else:
            slice_ = self.generate_physl([symbol, (lower, upper, step)])

        return slice_

    def _Str(self, node):
        """class Str(s)"""

        return '"' + node.s + '"'

    def _Sub(self, node):
        """Leaf node, returning raw string of the 'subtraction' operation."""

        return '__sub'

    def _Subscript(self, node):
        """class Subscript(value, slice, ctx)"""

        def _NestedSubscript(node):
            """Handles the subscripts of dimensions higher than 1"""

            if isinstance(node.value, ast.Subscript):
                raise NotImplementedError(
                    'Phylanx only supports 1 and 2 dimensional arrays.')
                # value = _NestedSubscript(node.value)
            else:
                op = '%s' % get_symbol_info(node, 'slice')
                value = self.apply_rule(node.value)
                op = '%s' % get_symbol_info(node, 'slice')
                # if isinstance(node.ctx, ast.Load):
                # slice_ = self.apply_rule(node.slice)
                # return [value, [slice_]]

                # if isinstance(node.ctx, ast.Store):
                slice_ = self.apply_rule(node.slice)
                return [op, (value, [slice_])]

        op = '%s' % get_symbol_info(node, 'slice')
        slice_ = self.apply_rule(node.slice)
        if isinstance(node.value, ast.Subscript):
            value = _NestedSubscript(node.value)
            return [op, (value, slice_)]
        else:
            value = self.apply_rule(node.value)
        # TODO: **SLICING**
        #     if isinstance(node.slice, ast.Index) and isinstance(slice_, str) \
        #         or isinstance(node.slice, ast.Slice):
        #         # return [op, (value, slice_, 'nil')]
        #     else:
        #         return [op, (value, slice_)]
        return [op, (value, slice_)]

    def _With(self, node):
        if 0 < len(node.items) and type(node.items[0]) == ast.withitem:
            withitem = node.items[0]
            if type(withitem.context_expr) == ast.Attribute:
                attribute = withitem.context_expr
                if attribute.attr == "parallel":
                    if self.fglobals[attribute.value.id].parallel.is_parallel_block():
                        return ["parallel_block", tuple(map(self.apply_rule, node.body))]
            elif type(withitem.context_expr) == ast.Name:
                if withitem.context_expr.id == "parallel":
                    if self.fglobals["parallel"].is_parallel_block():
                        return ["parallel_block", tuple(map(self.apply_rule, node.body))]
        raise Exception("Unsupported use of 'With'")

    def _Dict(self, node):
        res = []
        for i in range(len(node.keys)):
            key = self.apply_rule(node.keys[i])
            val = self.apply_rule(node.values[i])
            res += [["list", (key, val)]]
        return ["dict", (["list", tuple(res)],)]

    def _Tuple(self, node):
        """class Tuple(elts, ctx)"""

        expr = tuple(map(self.apply_rule, node.elts))
        return expr

    def _UAdd(self, node):
        """

        TODO:
        Make sure we do not have the equivalent of this in PhySL. Otherwise, add support.
        *** For now we never get here (see :func:_UnaryOp)
        """

        raise Exception("`UAdd` operation is not defined in PhySL.")

    def _UnaryOp(self, node):
        """class UnaryOp(op, operand)"""

        operand = self.apply_rule(node.operand)
        if isinstance(node.op, ast.UAdd):
            return [(operand, )]

        op = get_symbol_info(node, self.apply_rule(node.op))
        return [op, (operand, )]

    def _USub(self, node):
        """Leaf node, returning raw string of the 'negative' operation."""

        return '__minus'

    def _While(self, node):
        """class While(test, body, orelse)

        TODO:
        Figure out what `orelse` attribute may contain. From my experience this is always
        an empty list!
        """

        symbol = '%s' % get_symbol_info(node, 'while')
        test = self.block(node.test)
        body = self.block(node.body)
        return [symbol, (test, body)]


# #############################################################################
