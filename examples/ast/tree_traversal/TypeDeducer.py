# Copyright (c) 2018 Maxwell Reeser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import get_type
import numpy_output_type
import ast
import time


class Variable(object):
    """ Keeps track of anything which is found in an ast.Name node

    Used in the var_list and target_list in the TypeDeducerState.
    """
    def __init__(self, name, lineno, col_offset, var_type,
                 conditional_assigned=False):
        self.name = name
        self.lineno = lineno
        self.col_offset = col_offset
        self.var_type = var_type
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
              "Var Type: {}\n".format(self.name, self.lineno, self.col_offset,
                                      self.var_type))
        

class TypeDeducerState(object):
    """ Used as a reference to certain information for the recursive
    TypeDeducer data structure

    Also contains two auxiliary functions, get_closest_ref_type as well as
    add_to_target_list
    """
    assign_list: list
    target_list: list
    var_list: list
    variable_dict: dict
    inside_conditional: bool

    def __init__(self, variable_dict, inside_conditional=False):
        self.assign_list = []
        self.target_list = []
        self.var_list = []
        self.variable_dict = variable_dict
        self.inside_conditional = inside_conditional

    def get_closest_ref_type(self, my_name, my_lineno, my_col):
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
            return closest_ref.var_type
        else:
            return None

    def add_to_target_list(self, node, var_type):
        """Adds nodes found as targets in assignments as variables to
        TypeDeducerState's target_list"""
        if self.inside_conditional:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               conditional_assigned=True)
        else:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type)
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

    def __init__(self, state, target=False):
        self.var_type = None
        self.type_deducer_state = state
        self.target = target

    def generic_visit(self, node):
            ast.NodeVisitor.generic_visit(self, node)

    def visit_Assign(self, node):
        """Versions of visit_Assign are one of the workhorses for
        TypeDeducer, as they track the types of assignment targets

        Each Assignment node also gets turned into an Annotated assignment
        such that the annotation bears the type of the target. This
        functionality may not be needed in the future, depending on if
        these annotations are ever used or if the var and target lists
        in TypeDeducerState will be sufficient
        """
        x = TypeDeducer(self.type_deducer_state)
        x.visit(node.value)
        self.var_type = x.var_type
        self.type_deducer_state.add_to_target_list(node.targets[0], self.var_type)
        node = ast.AnnAssign(lineno=node.lineno, col_offset=node.col_offset,
                             target=node.targets, annotation=self.var_type,
                             value=node.value, simple=1)
        self.type_deducer_state.assign_list.append(node)

    def visit_AnnAssign(self, node):
        x = TypeDeducer(self.type_deducer_state)
        x.visit(node.value)
        self.var_type = x.var_type
        self.type_deducer_state.add_to_target_list(node.target, self.var_type)
        # If an assignment is previously annotated, it should be annotated
        # with the type of the target
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
        self.type_deducer_state.add_to_target_list(node.target, self.var_type)

    def visit_Name(self, node):
        """Creates variables and inserts them into the TypeDeducerState's
        var_list. Also checks if the variable is being assigned inside a
        conditional. This is not particularly useful though, as it only
        matters if the Name is an assignment target
        """
        var_type = self.type_deducer_state.get_closest_ref_type(node.id,
                                                                node.lineno,
                                                                node.col_offset)
        if self.type_deducer_state.inside_conditional:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type,
                               conditional_assigned=True)
        else:
            var_tmp = Variable(node.id, node.lineno, node.col_offset, var_type)
        if var_tmp is not None:
            self.type_deducer_state.var_list.append(var_tmp)
            self.var_type = var_tmp.var_type
        ast.NodeVisitor.generic_visit(self, node)

    def visit_arg(self, node):
        """Much like visit_Name, visit_arg inserts variables passed
        as arguments to the function into the var list. If the argument
        is annotated, which should be the argument's type, it adds that
        information to the variable."""
        var_type = self.type_deducer_state.get_closest_ref_type(node.arg,
                                                                node.lineno,
                                                                node.col_offset)
        var_tmp = Variable(node.arg, node.lineno, node.col_offset, var_type)
        if node.annotation is not None:
            var_tmp.var_type = node.annotation.id
            node.annotation = node.annotation.id
        self.type_deducer_state.var_list.append(var_tmp)
        self.var_type = var_tmp.var_type
        ast.NodeVisitor.generic_visit(self, node)

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
            self.var_type = get_type.get_type(op_name, left_type, right_type)
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
            self.var_type = get_type.get_type(op_name, type_var)
        else:
            self.var_type = None

    def visit_BoolOp(self, node):
        """TODO - Finish the implementation for BoolOps"""
        raise NotImplementedError("Boolean operations are not supported")

    def visit_Call(self, node):
        """Like visit_BinOp and visit_UnaryOp, visit_Call handles type deduction
        for arbitrary function calls. Currently, it only supports numpy unary or
        binary function calls however"""
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
                            self.var_type = get_type.get_type(node.func.attr,
                                                              left_type, right_type)
                        else:
                            self.var_type = None
                    except TypeError:
                        try:
                            func = numpy_output_type.numpy_parsers[node.func.attr]
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
                            self.var_type = get_type.get_type(node.func.attr, type_var)
                        else:
                            self.var_type = None
                    except TypeError:
                        try:
                            func = numpy_output_type.numpy_parsers[node.func.attr]
                            self.var_type = func(node.args, node.keywords)
                        except KeyError:
                            time.sleep(0.01)
                            raise NotImplementedError(
                                "Numpy function {} not supported".format(node.func.attr))
                else:
                    try:
                        print("Trying numpy parsers! {}".format(node.func.attr))
                        func = numpy_output_type.numpy_parsers[node.func.attr]
                        self.var_type = func(node.args, node.keywords)
                        print(numpy_output_type.numpy_parsers)
                    except KeyError:
                        raise NotImplementedError(
                            "Numpy function {} not supported".format(node.func.attr))
            else:
                raise NotImplementedError(
                    "Non-Numpy function calls not supported"
                )
        else:
            raise NotImplementedError(
                "Non-Numpy function calls not supported"
            )
