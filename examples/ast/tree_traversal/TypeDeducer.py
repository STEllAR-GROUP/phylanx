# Copyright (c) 2018 Maxwell Reeser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import get_type
import ast


class Variable(object):
    """ Keeps track of anything which is found in an ast.Name node
    Used in the var_list and target_list in the TypeDeducerState.
    """
    def __init__(self, name, lineno, col_offset, var_type,
                 conditional_assigned=False, dims=None):
        self.name = name
        self.lineno = lineno
        self.col_offset = col_offset
        self.var_type = var_type
        self.conditional_assigned = conditional_assigned
        self.dims = dims

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
              "Var Type: {}".format(self.name, self.lineno, self.col_offset,
                                    self.var_type))
        if self.dims is not None:
            print("\tDimensions: {}\n".format(self.dims))
        else:
            print()


class TypeDeducerState(object):
    """ Used as a reference to certain information for the recursive
    TypeDeducer data structure
    Also contains two auxiliary functions, get_closest_ref as well as
    add_to_target_list
    """

    def __init__(self, variable_dict, inside_conditional=False):
        self.assign_list = []
        self.target_list = []
        self.var_list = []
        self.variable_dict = variable_dict
        self.inside_conditional = inside_conditional
        self.new_variable_ref = False
        self.last_id = 0

    def get_closest_ref(self, my_name, my_lineno, my_col):
        """Given a variable name and location returns the most recent type
        that that variable had"""
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
            return closest_ref
        else:
            raise LookupError("Variable {} not found".format(my_name))

    def variable_previously_used(self, my_name, my_lineno, my_col):
        """Given a variable name and location checks whether that variable has
        been previously declared"""
        for var in self.var_list:
            if var.name == my_name:
                if var.lineno < my_lineno:
                    return True
                elif var.lineno == my_lineno:
                    if var.col_offset < my_col:
                        return True
                    else:
                        return False
                else:
                    return False
        return False

    def add_to_target_list(self, node, var_type, dims=None):
        """Adds nodes found as targets in assignments as variables to
        TypeDeducerState's target_list"""
        if self.inside_conditional:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               conditional_assigned=True, dims=dims)
        else:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               dims=dims)
        if var_tmp is not None:
            self.target_list.append(var_tmp)
            self.var_list.append(var_tmp)


class TypeDeducer(ast.NodeVisitor):
    """
    Recursive data structure used to traverse the AST and extract
    type information from all nodes in a single traversal of the AST
    Inherits from ast.NodeVisitor, in order to allow the use of
    TypeDeducer.visit() to traverse the AST
    """
    var_type: str
    type_deducer_state: TypeDeducerState
    target: bool
    dims: tuple
    # TODO - Should refactor TypeDeducer and associated classes/functions to use dims the
    # the same way Numpy does, as the primary way of interpreting what is currently
    # 'var_type'

    def __init__(self, state, target=False):
        self.var_type = None
        self.type_deducer_state = state
        self.target = target
        self.referenced_var_list = []
        self.dims = None
        self.var_representation = None

    def generic_visit(self, node):
            ast.NodeVisitor.generic_visit(self, node)

    def make_var_representation(self, node):
        self.var_representation = Variable("r{}".format(self.type_deducer_state.last_id),
                                           node.lineno, node.col_offset, self.var_type,
                                           conditional_assigned=self.
                                           type_deducer_state.inside_conditional,
                                           dims=self.dims)
        self.type_deducer_state.last_id += 1
        self.type_deducer_state.var_list.append(self.var_representation)

    def visit_Assign(self, node):
        """Versions of visit_Assign are one of the workhorses for
        TypeDeducer, as they track the types of assignment targets
        Each Assignment node also gets turned into an Annotated assignment
        such that the annotation bears the type of the target. This
        functionality may not be needed in the future, depending on if
        these annotations are ever used or if the var and target lists
        in TypeDeducerState will be sufficient
        """
        if type(node.value).__name__ == "Num":
            self.var_type = 'scalar'
        else:
            x = TypeDeducer(self.type_deducer_state)
            x.visit(node.value)
            if x.type_deducer_state.new_variable_ref:
                raise Exception("Attempting to use undeclared variable in"
                                " assignment: Line number: {} Column Offset: {}".format(
                                    node.lineno, node.col_offset))
            self.var_type = x.var_type
            self.dims = x.dims
        self.type_deducer_state.add_to_target_list(node.targets[0], self.var_type,
                                                   self.dims)
        node = ast.AnnAssign(lineno=node.lineno, col_offset=node.col_offset,
                             target=node.targets, annotation=self.var_type,
                             value=node.value, simple=1)
        self.type_deducer_state.assign_list.append(node)

    def visit_AnnAssign(self, node):
        x = TypeDeducer(self.type_deducer_state)
        x.visit(node.value)
        if x.type_deducer_state.new_variable_ref:
            raise Exception("Attempting to use undeclared variable in annotated"
                            " assignment: Line number: {} Column Offset: {}".format(
                                node.lineno, node.col_offset))
        self.var_type = x.var_type
        self.dims = x.dims
        self.type_deducer_state.add_to_target_list(node.target, self.var_type, self.dims)
        # If an assignment is previously annotated, it should be annotated
        # with the type of the target
        if isinstance(node.annotation, ast.Name):
            assert(node.annotation.id == self.var_type)
            node.annotation = node.annotation.id
        elif isinstance(node.annotation, ast.NameConstant):
            assert(node.annotation.value == self.var_type)
            node.annotation = node.annotation.value

    def visit_AugAssign(self, node):
        aug_op = ast.BinOp(left=node.target, op=node.op, right=node.value,
                           lineno=node.lineno, col_offset=node.col_offset)
        x = TypeDeducer(self.type_deducer_state)
        x.visit(aug_op)
        if x.type_deducer_state.new_variable_ref:
            raise Exception("Attempting to use undeclared variable in augmented"
                            " assignment: Line number: {} Column Offset: {}".format(
                                node.lineno, node.col_offset))
        self.var_type = x.var_type
        self.dims = x.dims
        node = ast.AnnAssign(lineno=node.lineno, col_offset=node.col_offset,
                             target=node.target, annotation=self.var_type,
                             value=aug_op, simple=1)
        self.type_deducer_state.assign_list.append(node)
        self.type_deducer_state.add_to_target_list(node.target, self.var_type, self.dims)

    def visit_Num(self, node):
        self.var_type = 'scalar'
        self.dims = (1, 1)

    def visit_Name(self, node):
        """Creates variables and inserts them into the TypeDeducerState's
        var_list. Also checks if the variable is being assigned inside a
        conditional. This is not particularly useful though, as it only
        matters if the Name is an assignment target
        """
        try:
            var = self.type_deducer_state.get_closest_ref(node.id,
                                                          node.lineno,
                                                          node.col_offset)
            var_type = var.var_type
            self.dims = var.dims
        except LookupError:
            self.type_deducer_state.new_variable_ref = True
            var_type = None
        if self.type_deducer_state.inside_conditional:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               conditional_assigned=True, dims=self.dims)
        else:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               dims=self.dims)
        if var_tmp is not None:
            self.type_deducer_state.var_list.append(var_tmp)
            self.var_type = var_tmp.var_type
            self.referenced_var_list.append(var_tmp)
        ast.NodeVisitor.generic_visit(self, node)

    def visit_arg(self, node):
        """Much like visit_Name, visit_arg inserts variables passed
        as arguments to the function into the var list. If the argument
        is annotated, which should be the argument's type, it adds that
        information to the variable."""
        try:
            var = self.type_deducer_state.get_closest_ref(node.arg,
                                                          node.lineno,
                                                          node.col_offset)
            var_type = var.var_type
            var_tmp = Variable(node.arg, node.lineno, node.col_offset, var_type)
        except LookupError:
            var_tmp = Variable(node.arg, node.lineno, node.col_offset, None)
        if node.annotation is not None:
            var_tmp.var_type = node.annotation.id
            node.annotation = node.annotation.id
        self.type_deducer_state.var_list.append(var_tmp)
        self.var_type = var_tmp.var_type
        self.referenced_var_list.append(var_tmp)
        ast.NodeVisitor.generic_visit(self, node)

    def visit_Tuple(self, node):
        self.var_type = 'row_vector'
        if len(node.elts) == 0:
            self.dims = ()
        elif len(node.elts) == 1:
            if isinstance(node.elts[0], ast.Num):
                self.dims = (node.elts[0].n, )
        elif len(node.elts) == 2:
            if isinstance(node.elts[0], ast.Num) and isinstance(node.elts[1], ast.Num):
                self.dims = (node.elts[0].n, node.elts[1].n)

    def visit_BinOp(self, node):
        """Another linchpin for TypeDeducer, BinOp handles type inference
        for binary operations"""
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
            self.var_type, self.dims = get_type.get_output(op_name,
                                                           [left_type, right_type])
            self.make_var_representation(node)
            self.referenced_var_list.append(left_type_deducer.referenced_var_list)
            self.referenced_var_list.append(right_type_deducer.referenced_var_list)
        else:
            self.var_type = None

    def visit_UnaryOp(self, node):
        """Similar to visit_BinOP, visit_UnaryOp handles type deduction
         of unary operations carried out on variables"""
        type_deducer = TypeDeducer(self.type_deducer_state)
        type_deducer.visit(node.operand)
        type_var = type_deducer.var_type
        if type_var is not None:
            op_name = type(node.op).__name__.lower()
            self.var_type, self.dims = get_type.get_output(op_name, [type_var])
            self.make_var_representation(node)
        else:
            self.var_type = None

    def visit_BoolOp(self, node):
        """TODO - Finish the implementation for BoolOps"""
        raise NotImplementedError("Boolean operations are not supported: "
                                  "Line Number: {} Column Offset: "
                                  "{}".format(node.lineno, node.col_offset))

    def visit_Call(self, node):
        """Like visit_BinOp and visit_UnaryOp, visit_Call handles type deduction
        for arbitrary function calls. Currently however, it only supports numpy unary or
        binary function calls, and determinant"""
        op_name = None
        if isinstance(node.func, ast.Attribute):
            if node.func.value.id == "np" or node.func.value.id == "numpy":
                op_name = node.func.attr
            else:
                raise NotImplementedError("Non-Numpy functions not supported\n"
                                          "\tLine number: {}\n\tColumn Offset: {}"
                                          .format(node.lineno, node.col_offset))
        elif isinstance(node.func, ast.Name):
            op_name = node.func.id
        td_list = []
        type_list = []
        dims_list = []
        if len(node.args) > 0:
            for arg in node.args:
                td = TypeDeducer(self.type_deducer_state)
                td.visit(arg)
                if td.var_type is None:
                    self.var_type = None
                    return
                td_list.append(td)
                type_list.append(td.var_type)
                dims_list.append(td.dims)
            self.var_type, self.dims = get_type.get_output(op_name, type_list, dims_list)
            self.make_var_representation(node)
            for td in td_list:
                self.referenced_var_list.append(td.referenced_var_list)
