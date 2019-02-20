# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


class Argument:
    def __init__(self, name, scope, lineno=None, col_offset=None):
        self.name = name
        self.scope = scope
        self.lineno = lineno
        self.col_offset = col_offset

    def __eq__(self, other):
        self_ = (self.name, self.scope, self.lineno, self.col_offset)
        other_ = (other.name, other.scope, other.lineno, other.col_offset)
        return self_ == other_


class Variable(Argument):
    def __init__(self, name, scope, lineno, col_offset, type_=None):
        super().__init__(name, scope, lineno, col_offset)
        self.type = type_

    def __eq__(self, other):
        self_ = (self.name, self.scope, self.lineno, self.col_offset,
                 self.type)
        other_ = (other.name, other.scope, other.lineno, other.col_offset,
                  self.type)
        return self_ == other_


class Array(Variable):
    def __init__(self,
                 name,
                 scope,
                 lineno,
                 col_offset,
                 dimension=None,
                 shape=None):
        super().__init__(name, scope, lineno, col_offset)

        if dimension and dimension < 1:
            raise ValueError(
                f"Arrays must have 1 or more dimension(s). {dimension} given.")
        self.dimensionality = dimension

        if shape and not len(shape) == dimension:
            raise ValueError(
                f"Array dimensionality({dimension}) does not match the shape({shape})."
            )
        self.shape = shape

    def __eq__(self, other):
        self_ = (self.name, self.scope, self.lineno, self.col_offset,
                 self.dimensionality, self.shape)
        other_ = (other.name, other.scope, other.lineno, other.col_offset,
                  other.dimensionality, other.shape)
        return self_ == other_


class Function(Argument):
    def __init__(self, name, scope, lineno, col_offset, dtype=''):
        super().__init__(name, scope, lineno, col_offset)
        self.args_list = []
        self.dtype = dtype

    def add_arg(self, argument):
        if isinstance(argument, list):
            self.args_list.extend(argument)
        else:
            self.args_list.append(argument)

    def insert_arg(self, argument, position):
        self.args_list.insert(position, argument)

    def prepend_arg(self, argument):
        self.insert_arg(argument, 0)

    def arguments(self):
        return self.args_list

    def __eq__(self, other):
        self_ = (self.name, self.scope, self.lineno, self.col_offset,
                 self.args_list, self.dtype)
        other_ = (other.name, other.scope, other.lineno, other.col_offset,
                  other.args_list, other.dtype)
        return self_ == other_


class FunctionCall(Function):
    pass


class FunctionDef(Function):
    pass
