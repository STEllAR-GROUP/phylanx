# Copyright (c) 2019 R. Tohid
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

from phylanx.ir import nodes
import pytest


class TestArgument:
    def test_argument(self):
        arg_info = ('arg', 'dummy_space', 123, 4)
        arg = nodes.Argument(*arg_info)
        assert (arg.name, arg.scope, arg.lineno, arg.col_offset) == arg_info

    def test_equality(self):
        arg_info = ('arg', 'dummy_space', 123, 4)
        arg_1 = nodes.Argument(*arg_info)
        arg_2 = nodes.Argument(*arg_info)
        assert arg_1 == arg_2


class TestVariable:
    def test_variable(self):
        var_info = ('var', 'dummy_space', 123, 4, 'float')
        var = nodes.Variable(*var_info)
        assert (var.name, var.scope, var.lineno, var.col_offset,
                var.type) == var_info

    def test_equality(self):
        var_info = ('var', 'dummy_space', 123, 4, 'float')
        var_1 = nodes.Variable(*var_info)
        var_2 = nodes.Variable(*var_info)
        assert var_1 == var_2


class TestArray:
    def test_default_array(self):
        array_info = ('array', 'dummy_space', 123, 4)
        expected_info = (*array_info, None, None)
        array = nodes.Array(*array_info)
        assert (array.name, array.scope, array.lineno, array.col_offset,
                array.dimensionality, array.shape) == expected_info

    def test_dimension(self):
        array_info = ('array', 'dummy_space', 123, 4, 2)
        expected_info = (*array_info, None)
        array = nodes.Array(*array_info)
        assert (array.name, array.scope, array.lineno, array.col_offset,
                array.dimensionality, array.shape) == expected_info

    def test_invalid_dimension(self):
        array_info = ('array', 'dummy_space', 123, 4, -1)
        with pytest.raises(ValueError):
            nodes.Array(*array_info)

    def test_shape(self):
        array_info = ('array', 'dummy_space', 123, 4, 2, (4, 4))
        array = nodes.Array(*array_info)
        assert (array.name, array.scope, array.lineno, array.col_offset,
                array.dimensionality, array.shape) == array_info

    def test_invalid_shape(self):
        array_info = ('array', 'dummy_space', 123, 4, 2, (3, 3, 3))
        with pytest.raises(ValueError):
            nodes.Array(*array_info)

    def test_equality(self):
        array_info = ('array', 'dummy_space', 123, 4, 2, (2, 2))
        array_1 = nodes.Array(*array_info)
        array_2 = nodes.Array(*array_info)
        assert array_1 == array_2


class TestFunction:
    def test_default_function(self):
        function_info = ('func', 'dummy_space', 123, 4)
        expected_info = (*function_info, '')
        function = nodes.Function(*function_info)
        assert (function.name, function.scope, function.lineno,
                function.col_offset, function.dtype) == expected_info

    def test_dtype(self):
        function_info = ('func', 'dummy_space', 123, 4, 'float')
        function = nodes.Function(*function_info)
        assert (function.name, function.scope, function.lineno,
                function.col_offset, function.dtype) == function_info

    def test_add_arg(self):
        function_info = ('func', 'dummy_space', 123, 4)
        function = nodes.Function(*function_info)
        function.add_arg('arg')
        assert ['arg'] == function.args_list

    def test_add_multiple_args(self):
        function_info = ('func', 'dummy_space', 123, 4)
        function = nodes.Function(*function_info)
        function.add_arg(['arg_0', 'arg_1'])
        assert ['arg_0', 'arg_1'] == function.args_list

    def test_insert_arg(self):
        function_info = ('func', 'dummy_space', 123, 4)
        function = nodes.Function(*function_info)
        function.add_arg(['arg_0', 'arg_1'])
        function.insert_arg('arg_2', 1)
        assert ['arg_0', 'arg_2', 'arg_1'] == function.args_list

    def test_prepend_arg(self):
        function_info = ('func', 'dummy_space', 123, 4)
        function = nodes.Function(*function_info)
        function.add_arg(['arg_0', 'arg_1'])
        function.prepend_arg('arg_2')
        assert ['arg_2', 'arg_0', 'arg_1'] == function.args_list

    def test_arguments(self):
        function_info = ('func', 'dummy_space', 123, 4)
        function = nodes.Function(*function_info)
        function.add_arg('arg')
        assert ['arg'] == function.arguments()

    def test_equality(self):
        function_info = ('func', 'dummy_space', 123, 4)
        function_01 = nodes.Function(*function_info)
        function_02 = nodes.Function(*function_info)
        assert function_01 == function_02
