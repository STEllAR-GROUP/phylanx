unary_scalar = {
    "add": "scalar",
    "minus": "scalar",
    "not": "scalar",
    "sub": "scalar",
    "add_dim": "vector",
    "arange": "vector",
    "define": "scalar",
    "filter": "scalar",
    "len": "scalar",
    "mean": "scalar",
    "range": "scalar"
}

unary_vector = {
    "add": "vector",
    "minus": "vector",
    "sub": "vector",
    "decomposition": "matrix",  # Decompose vector into one or more vectors, sounds like a matrix to me
    "linear_solver": "vector",
    "add_dim": "matrix",
    "all": " ",
    "als": " ",
    "any": " ",
    "append": "vector",
    "argmax": "scalar",
    "argmin": "scalar",
    "cumsum": "scalar",
    "inverse": "vector",
    "len": "scalar",
    "list": "vector",
    "make_list": "vector",
    "mean": "scalar",
    "shape": "scalar",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar",
    "transpose": "vector"  # This is debatable, could be considered a matrix
}

unary_col_vector = {
    "add": "column vector",
    "minus": "column vector",
    "sub": "vector",
    "decomposition": "matrix",  # Decompose vector into one or more vectors, sounds like a matrix to me
    "linear_solver": "column vector",
    "add_dim": "matrix",
    "argmax": "scalar",
    "argmin": "scalar",
    "cumsum": "scalar",
    "inverse": "column vector",
    "len": "scalar",
    "list": "column vector",
    "make_list": "column vector",
    "mean": "scalar",
    "shape": "scalar",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar",
    "transpose": "row vector"  # This is debatable, could be considered a matrix
}

unary_row_vector = {
    "add": "row vector",
    "minus": "row vector",
    "sub": "vector",
    "decomposition": "matrix",  # Decompose vector into one or more vectors, sounds like a matrix to me
    "linear_solver": "row vector",
    "add_dim": "matrix",
    "argmax": "scalar",
    "argmin": "scalar",
    "cumsum": "scalar",
    "inverse": "row vector",
    "len": "scalar",
    "list": "row vector",
    "make_list": "row vector",
    "mean": "scalar",
    "shape": "scalar",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar",
    "transpose": "column vector"  # This is debatable, could be considered a matrix
}

unary_matrix = {
    "add": "matrix",
    "minus": "matrix",
    "decomposition": "matrices",  # Matrix factorization yields multiple matrices
    "linear_solver": "vector",
    "add_dim": "matrix",
    "append": "matrix",
    "argmax": " ",  # Should these count?
    "argmin": " ",
    "determinant": "depends",
    "diag": "vector",
    "identity": "matrix",  # Return matrix of same dimension but identity?
    "inverse": "matrix",
    "invert" : "matrix",
    "len": "scalar",
    "linearmatrix": "vector",  # Here assuming that linear matrix is concat'ed matrix
                               # rows/columns
    "mean": "scalar",
    "power": "matrix",
    "random": "matrix",
    "shape": "vector",
    "slice": "vector",
    "slice_column": "vector",
    "slice_row": "vector",
    "sum": "scalar",
    "transpose": "matrix"
}

unary_ops = {
    "scalar": unary_scalar,
    "vector": unary_vector,
    "matrix": unary_matrix
}

####################################################

scalar_scalar = {
    "add": "scalar",
    "and": "scalar",
    "div": "scalar",
    "minus": "scalar",
    "mul": "scalar",
    "sub": "scalar",
    "argmax": "scalar",
    "argmin": "scalar",
    "dot": "scalar",
    "linspace": " ",
    "power": "scalar",
    "sum": "scalar"
}

scalar_vector = {
    "add": "vector",
    "minus": "vector",
    "mul": "vector",
    "sub": "vector",
    "cumsum": "scalar",
    "dot": "vector",
    "filter": "vector",
    "power": "vector",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar"
}

scalar_column_vector = {
    "add": "column vector",
    "minus": "column vector",
    "mul": "column vector",
    "sub": "column vector",
    "cumsum": "scalar",
    "dot": "column vector",
    "filter": "column vector",
    "power": "column vector",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar"
}

scalar_row_vector = {
    "add": "row vector",
    "minus": "row vector",
    "mul": "row vector",
    "sub": "row vector",
    "cumsum": "scalar",
    "dot": "row vector",
    "filter": "row vector",
    "power": "row vector",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar"
}

scalar_matrix = {
    "add": "matrix",
    "mul": "matrix",
    "dot": "matrix",
    "power": "matrix"
}


vector_scalar = {
    "add": "vector",
    "div": "vector",
    "minus": "vector",
    "mul": "vector",
    "sub": "vector",
    "cumsum": "scalar",
    "power": "vector",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar"
}

column_vector_scalar = {
    "add": "column vector",
    "div": "column vector",
    "minus": "column vector",
    "mul": "column vector",
    "sub": "column vector",
    "cumsum": "scalar",
    "power": "column vector",
    "slice": "scalar",
    "slice_column": "column vector",
    "slice_row": "scalar",
    "sum": "scalar"
}

row_vector_scalar = {
    "add": "row vector",
    "div": "row vector",
    "minus": "row vector",
    "mul": "row vector",
    "sub": "row vector",
    "cumsum": "scalar",
    "power": "row vector",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "row column",
    "sum": "scalar"
}

vector_vector = {
    "add": "vector",
    "div": "vector",
    "minus": "vector",
    "mul": "vector",
    "sub": "vector",
    "cumsum": "scalar",
    "dot": "scalar",
    "mean": "scalar",
    "power": "vector",
    "sum": "vector"  # This could go either way I think vector | scalar
}

column_vector_column_vector = {
    "add": "column vector",
    "div": "column vector",  # Assuming element-wise division here
    "minus": "column vector",
    "mul": "column vector",  # Element-wise
    "sub": "column vector",
    "cumsum": "scalar",
    "mean": "scalar",  # This could be a vector of averages between the two elements, or average of all elements
    "power": "column vector"
}

column_vector_row_vector = {
    "add": "matrix",  # Should this even be supported? I sorta think not
    "div": "matrix",
    "minus": "matrix",
    "mul": "matrix",
    "sub": "matrix",
    "dot": "matrix",
    "power": "matrix"
}

row_vector_row_vector = {
    "add": "row vector",
    "div": "row vector",
    "minus": "row vector",
    "mul": "row vector",
    "sub": "row vector",
    "power": "row vector"
}

row_vector_column_vector = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "matrix",
    "sub": "matrix",
    "dot": "scalar",
    "power": "matrix"
}

vector_matrix = {
    "add": "matrix",
    "mul": "vector",
    "argmax": "vector",
    "argmin": "vector",
    "cumsum": "",  # ? Don't know what to do with this
    "dot": "vector",
    "power": "matrix",
    "sum": "vector"
}

row_vector_matrix = {
    "add": "matrix",
    "mul": "matrix",
    "dot": "row vector",
    "power": "matrix"
}

column_vector_matrix = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "matrix",
    "sub": "matrix",
    "power": "matrix"
}

matrix_scalar = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "matrix",
    "sub": "matrix",
    "decomposition": "matrix",  # Maybe decompose indexed vector?
    "cumsum": "scalar",
    "diag": "vector",  # indexed diagonal?
    "dot": "matrix",
    "power": "matrix",
    "slice": "vector",
    "slice_column": "vector",
    "slice_row": "vector"
}

matrix_vector = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "vector",
    "sub": "matrix",
    "cumsum": "scalar",
    "dot": "vector",
    "power": "matrix",
    "sum": "scalar"  # Is this right? Maybe when I'm less tired it will be obvious
}

matrix_row_vector = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "matrix",
    "sub": "matrix",
    "dot": "row vector",
    "power": "matrix"
}

matrix_column_vector = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "vector",
    "sub": "matrix",
    "dot": "column vector",
    "power": "matrix"
}

matrix_matrix = {
    "add": "matrix",
    "and": "matrix",  # Element-wise and operation
    "div": "matrix",  # Either elementwise or the opposite of dot, either is matrix
    "minus": "matrix",
    "mul": "matrix",
    "sub": "matrix",
    "linear_solver": "matrix",  # Maybe?
    "append": "matrix",
    "cumsum": "scalar",
    "dot": "matrix",
    "mean": "matrix",
    "power": "matrix",
    "sum": "matrix"
}

scalar = {
    "scalar": scalar_scalar,
    "row vector": scalar_row_vector,
    "column vector": scalar_column_vector,
    "matrix": scalar_matrix
}

vector = {
    "scalar": vector_scalar,
    "vector": vector_vector,
    "matrix": vector_matrix
}

row_vector = {
    "scalar": vector_scalar,
    "row vector": row_vector_row_vector,
    "column vector": row_vector_column_vector,
    "matrix": row_vector_matrix
}

column_vector = {
    "scalar": column_vector_scalar,
    "row vector": column_vector_row_vector,
    "column vector": column_vector_column_vector,
    "matrix": column_vector_matrix
}

matrix = {
    "scalar": matrix_scalar,
    "vector": matrix_vector,
    "row_vector": matrix_row_vector,
    "column_vector": matrix_column_vector,
    "matrix": matrix_matrix
}

first_type = {
    "scalar": scalar,
    "vector": vector,
    "matrix": matrix
}


# This needs review
def parse_zeros(args, keywords):
    print("Inside parse_zeros!")
    if len(args) > 0:
        print(args[0])
        if isinstance(args[0], ast.Tuple):
            print("It's a tuple!")
            if len(args[0].elts) == 0:
                return 'scalar'
            elif len(args[0].elts) == 1:
                if args[0].elts[0].n > 1:
                    return 'vector'
                else:
                    return 'scalar'
            else:
                if args[0].elts[0].n > 1:
                    if args[0].elts[1].n > 1:
                        return 'matrix'
                    else:
                        return 'vector'
                else:
                    if args[0].elts[1].n > 1:
                        return 'vector'
                    else:
                        return 'scalar'
    return None


def parse_identity(args, keywords):
    if len(args) > 0:
        if isinstance(args[0], int):
            if args[0] == 1:
                return 'scalar'
            elif args[0] >= 2:
                return 'matrix'
    return None


numpy_parsers = {
    'zeros': parse_zeros,
    'ones': parse_zeros,
    'identity': parse_identity
}


def get_type(op, lhs, rhs=None):
    try:
        if rhs is not None:
            dict_1 = first_type[lhs]
            dict_2 = dict_1[rhs]
            result = dict_2[op]
        else:
            result = unary_ops[lhs][op]
        return result
    except KeyError:
        if rhs is not None:
            raise TypeError("Operation {} not supported with argument types {} "
                            "and {}".format(op, lhs, rhs))
        else:
            raise TypeError("Operation {} not supported with argument "
                            "type {}".format(op, lhs))


# from phylanx import Phylanx
# import sys
# sys.path.append('/home/mreeser/src/repos/phylanx/python/phylanx/ast')
# import physl
import ast
import inspect
import astpretty
import numpy
import time


class LinearAlgebraContainer(object):

    def __init__(self, name, lineno, col_offset, dims):
        self.name = name
        self.lineno = lineno
        self.col_offset = col_offset
        self.dims = dims


'''
@Phylanx(debug=True)
def tea_func(b : str , d):
  a = numpy.zeros(b, d)
  a += b(d)
  b = a + d
  return b
'''


class Variable(object):

    def __init__(self, name, lineno, col_offset, struct_type,
                 conditional_assigned=False):
        self.name = name
        self.lineno = lineno
        self.col_offset = col_offset
        self.var_type = struct_type
        self.conditional_assigned = conditional_assigned

    def __lt__(self, other):
        if isinstance(other, Variable):
            if self.lineno < other.lineno:
                return True
            elif self.lineno == other.lineno:
                if self.col_offset < other.col_offset:
                    return True
            return False
        else:
            return NotImplemented

    def print(self):
        print("Variable: {}\n\tLine Number: {}\n\tColumn Offset: {}\n\t"
              "Struct Type: {}\n".format(self.name, self.lineno, self.col_offset,
                                         self.var_type))


class TypeDeducerState(object):

    def __init__(self, variable_dict, inside_conditional=False):
        self.assign_list = []
        self.target_list = []
        self.var_list = []
        self.variable_dict = variable_dict
        self.inside_conditional = inside_conditional

    def get_closest_ref_type(self, my_name, my_lineno, my_col):
        closest_ref = None
        for var in self.var_list:
            if var.name == my_name:
                if var.lineno < my_lineno:
                    if closest_ref is not None and var.lineno > closest_ref.lineno:
                        closest_ref = var
                    else:
                        closest_ref = var
                elif var.lineno == my_lineno:
                    if var.col_offset < my_col:
                        if closest_ref is not None and closest_ref.lineno:
                            if closest_ref.lineno == var.lineno:
                                if closest_ref.col_offset < var.col_offset:
                                    closest_ref = var
                            else:
                                closest_ref = var
                        else:
                            closest_ref = var
        if closest_ref is not None:
            assert(self.inside_conditional is not True)
            return closest_ref.var_type
        else:
            return None


class TypeDeducer(ast.NodeVisitor):

    def __init__(self, state, target=False):
        self.var_type = None
        self.type_deducer_state = state
        self.target = target

    def add_to_target_list(self, node, struct_type):
        # ref_var_type = self.type_deducer_state.get_closest_ref_type(node.id, \
        #            node.lineno, node.col_offset) # May be totally unnecessary
        # var_tmp = None
        if self.type_deducer_state.inside_conditional:
            # print("We're inside a conditional, baby!")
            var_tmp = Variable(node.id, node.lineno, node.col_offset, struct_type,
                               conditional_assigned=True)
        else:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, struct_type)
        if var_tmp is not None:
            self.type_deducer_state.target_list.append(var_tmp)
            self.type_deducer_state.var_list.append(var_tmp)


    def generic_visit(self, node):
            ast.NodeVisitor.generic_visit(self, node)


    def visit_Assign(self, node):
        x = TypeDeducer(self.type_deducer_state)
        x.visit(node.value)
        self.var_type = x.var_type
        # if isinstance(x, ast.Name):
        # print("The type for variable {} is {}".format(x.id, self.var_type))
        self.add_to_target_list(node.targets[0], self.var_type)
        node = ast.AnnAssign(lineno=node.lineno, col_offset=node.col_offset,
                             target=node.targets, annotation=self.var_type,
                             value=node.value, simple=1)
        self.type_deducer_state.assign_list.append(node)

    def visit_AnnAssign(self, node):
        # print("Inside an annotated assignment!")
        # for k in self.type_deducer_state.var_list:
        # k.print()
        x = TypeDeducer(self.type_deducer_state)
        x.visit(node.value)
        # print(x.var_type)
        # print(type(node.value).__name__)
        self.var_type = x.var_type
        self.add_to_target_list(node.target, self.var_type)
        # astpretty.pprint(node)
        # print(self.var_type)
        # for k in self.type_deducer_state.var_list:
        # k.print()
        assert(node.annotation.id == self.var_type)
        node.annotation = node.annotation.id

    def visit_AugAssign(self, node):
        aug_op = ast.BinOp(left=node.target, op=node.op, right=node.value)
        x = TypeDeducer(self.type_deducer_state)
        x.visit(aug_op)
        self.var_type = x.var_type
        node = ast.AnnAssign(lineno=node.lineno, col_offset=node.col_offset,
                             target=node.target, annotation=self.var_type,
                             value=aug_op, simple=1)
        self.type_deducer_state.assign_list.append(node)
        self.add_to_target_list(node.target, self.var_type)


    def visit_Name(self, node):
        # print("Inside Name!")
        # print(node.id, node.lineno, node.col_offset)
        var_type = self.type_deducer_state.get_closest_ref_type(node.id,
                                            node.lineno, node.col_offset)
        if self.type_deducer_state.inside_conditional:
            # print("We're inside a conditional, baby!")
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               conditional_assigned=True)
        else:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type)
        if var_tmp is not None:
            # print("var_tmp is not None!", var_tmp.var_type)
            self.type_deducer_state.var_list.append(var_tmp)
            self.var_type = var_tmp.var_type
        ast.NodeVisitor.generic_visit(self, node)

    def visit_arg(self, node):
        var_type = self.type_deducer_state.get_closest_ref_type(node.arg,
                                            node.lineno, node.col_offset)
        var_tmp = Variable(node.arg, node.lineno, node.col_offset, var_type)
        if node.annotation is not None:
            var_tmp.var_type = node.annotation.id
            node.annotation = node.annotation.id
        self.type_deducer_state.var_list.append(var_tmp)
        # print("Inside an arg and the type is: {}".format(var_tmp.struct_type))
        self.var_type = var_tmp.var_type
        ast.NodeVisitor.generic_visit(self, node)

    def visit_BinOp(self, node):
        left_type_deducer = TypeDeducer(self.type_deducer_state)
        left_type_deducer.visit(node.left)
        left_type = left_type_deducer.var_type
        right_type_deducer = TypeDeducer(self.type_deducer_state)
        right_type_deducer.visit(node.right)
        right_type = right_type_deducer.var_type
        if left_type is not None and right_type is not None:
            op_name = type(node.op).__name__.lower()
            if type(node.op).__name__ == 'Mult':
                op_name = 'mul'
            self.var_type = get_type(op_name, left_type, right_type)
        else:
            self.var_type = None

    def visit_UnaryOp(self, node):
        type_deducer = TypeDeducer(self.type_deducer_state)
        type_deducer.visit(node.operand)
        type_var = type_deducer.var_type
        if type_var is not None:
            op_name = type(node.op).__name__.lower()
            self.var_type = get_type(op_name, type_var)
        else:
            self.var_type = None

    def visit_BoolOp(self, node):
        raise NotImplementedError("Boolean operations are not supported")

    def visit_Call(self, node):
        print("Inside a call!")
        if isinstance(node.func, ast.Attribute):
            callable_name = node.func.value.id
            if callable_name == 'np' or callable_name == 'numpy':
                print("Found a numpy function!")
                if len(node.args) == 2 and isinstance(node.args[0], ast.Name)\
                        and isinstance(node.args[1], ast.Name):
                    try:
                        left_type_deducer = TypeDeducer(self.type_deducer_state)
                        left_type_deducer.visit(node.args[0])
                        left_type = left_type_deducer.var_type
                        right_type_deducer = TypeDeducer(self.type_deducer_state)
                        right_type_deducer.visit(node.args[1])
                        right_type = right_type_deducer.var_type
                        if left_type is not None and right_type is not None:
                            self.var_type = get_type(node.func.attr,
                                                 left_type, right_type)
                        else:
                            self.var_type = None
                    except TypeError:
                        try:
                            func = numpy_parsers[node.func.attr]
                            self.var_type = func(node.args, node.keywords)
                        except KeyError:
                            raise NotImplementedError(
                                "Numpy function {} not supported".format(node.func.attr))
                elif len(node.args) == 1 and isinstance(node.args[0], ast.Name):
                    try:
                        type_deducer = TypeDeducer(self.type_deducer_state)
                        type_deducer.visit(node.args[0])
                        type_var = type_deducer.var_type
                        if type_var is not None:
                            self.var_type = get_type(node.func.attr, type_var)
                        else:
                            self.var_type = None
                    except TypeError:
                        try:
                            func = numpy_parsers[node.func.attr]
                            self.var_type = func(node.args, node.keywords)
                        except KeyError:
                            time.sleep(0.01)
                            raise NotImplementedError(
                                "Numpy function {} not supported".format(node.func.attr))
                else:
                    try:
                        print("Trying numpy parsers! {}".format(node.func.attr))
                        func = numpy_parsers[node.func.attr]
                        self.var_type = func(node.args, node.keywords)
                        print(numpy_parsers)
                    except KeyError:
                        raise NotImplementedError(
                            "Numpy function {} not supported".format(node.func.attr))




def get_var_and_target_list(expr):
    tree = ast.parse(expr)
    astpretty.pprint(tree)
    time.sleep(0.01)
    var_dict = {'dummy': 'me'}
    state = TypeDeducerState(variable_dict=var_dict)
    k2 = TypeDeducer(state)
    k2.visit(tree)

    print('--------------Variable List-----------------')
    for tmp in k2.type_deducer_state.var_list:
        tmp.print()
    print('---------------Target List------------------')
    for tmp in k2.type_deducer_state.target_list:
        tmp.print()


expr_ = '''
def tea_func(a : vector):
  b  = np.ones((2,2))
  c  = np.transpose(a)
  d  = np.dot(a,b)
'''

get_var_and_target_list(expr_)


