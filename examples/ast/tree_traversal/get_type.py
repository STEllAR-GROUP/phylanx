# Copyright (c) 2018 Maxwell Reeser
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#################################################
""" Dictionary for mapping unary operators/functions to output types """

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
    "add": "column_vector",
    "minus": "column_vector",
    "sub": "vector",
    "decomposition": "matrix",  # Decompose vector into one or more vectors, sounds like a matrix to me
    "linear_solver": "column_vector",
    "add_dim": "matrix",
    "argmax": "scalar",
    "argmin": "scalar",
    "cumsum": "scalar",
    "inverse": "column_vector",
    "len": "scalar",
    "list": "column_vector",
    "make_list": "column_vector",
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
    "transpose": "column_vector"  # This is debatable, could be considered a matrix
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
    "row_vector": unary_row_vector,
    "column_vector": unary_col_vector,
    "matrix": unary_matrix
}

####################################################
""" Dictionaries for mapping input types and operator/function name to output type """

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
    "add": "column_vector",
    "minus": "column_vector",
    "mul": "column_vector",
    "sub": "column_vector",
    "cumsum": "scalar",
    "dot": "column_vector",
    "filter": "column_vector",
    "power": "column_vector",
    "slice": "scalar",
    "slice_column": "scalar",
    "slice_row": "scalar",
    "sum": "scalar"
}

scalar_row_vector = {
    "add": "row_vector",
    "minus": "row_vector",
    "mul": "row_vector",
    "sub": "row_vector",
    "cumsum": "scalar",
    "dot": "row_vector",
    "filter": "row_vector",
    "power": "row_vector",
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
    "add": "column_vector",
    "div": "column_vector",
    "minus": "column_vector",
    "mul": "column_vector",
    "sub": "column_vector",
    "cumsum": "scalar",
    "power": "column_vector",
    "slice": "scalar",
    "slice_column": "column_vector",
    "slice_row": "scalar",
    "sum": "scalar"
}

row_vector_scalar = {
    "add": "row_vector",
    "div": "row_vector",
    "minus": "row_vector",
    "mul": "row_vector",
    "sub": "row_vector",
    "cumsum": "scalar",
    "power": "row_vector",
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
    "add": "column_vector",
    "div": "column_vector",  # Assuming element-wise division here
    "minus": "column_vector",
    "mul": "column_vector",  # Element-wise
    "sub": "column_vector",
    "cumsum": "scalar",
    "mean": "scalar",  # This could be a vector of averages between the two elements, or average of all elements
    "power": "column_vector"
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
    "add": "row_vector",
    "div": "row_vector",
    "minus": "row_vector",
    "mul": "row_vector",
    "sub": "row_vector",
    "power": "row_vector"
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
    "dot": "row_vector",
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
    "dot": "row_vector",
    "power": "matrix"
}

matrix_column_vector = {
    "add": "matrix",
    "div": "matrix",
    "minus": "matrix",
    "mul": "vector",
    "sub": "matrix",
    "dot": "column_vector",
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
    "row_vector": scalar_row_vector,
    "column_vector": scalar_column_vector,
    "matrix": scalar_matrix
}

vector = {
    "scalar": vector_scalar,
    "vector": vector_vector,
    "matrix": vector_matrix
}

row_vector = {
    "scalar": vector_scalar,
    "row_vector": row_vector_row_vector,
    "column_vector": row_vector_column_vector,
    "matrix": row_vector_matrix
}

column_vector = {
    "scalar": column_vector_scalar,
    "row_vector": column_vector_row_vector,
    "column_vector": column_vector_column_vector,
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
    "row_vector": row_vector,
    "column_vector": column_vector,
    "matrix": matrix
}

""" End of dictionaries for output types on two input types """
###################################################


def get_type(op, lhs, rhs=None) -> str:
    """Utility function for obtaining output type information for a given
    function and its argument types"""
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

