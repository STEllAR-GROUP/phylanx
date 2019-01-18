# Inspired by SUNIL RAY
# https://www.analyticsvidhya.com/blog/2017/05/neural-network-from-scratch-in-python-and-r/
# Copyright (c) 2019 Weile Wei
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import numpy as np
from phylanx import Phylanx


@Phylanx
def main_phylanx(X, y, wh, bh, wout, bout, lr, num_iter):
    # create local variable
    wh_local = wh
    bh_local = bh
    wout_local = wout
    bout_local = bout
    output = 0
    hidden_layer_input1 = 0
    hidden_layer_input = 0
    hidden_layer_activations = 0
    output_layer_input1 = 0
    output_layer_input = 0
    slope_hidden_layer = 0
    slope_output_layer = 0
    Error = 0
    d_output = 0
    Error_at_hidden_layer = 0
    d_hidden_layer = 0
    for i in range(num_iter):
        # Forward
        hidden_layer_input1 = np.dot(X, wh_local)
        hidden_layer_input = hidden_layer_input1 + bh_local
        hidden_layer_activations = (1 / (1 + np.exp(-hidden_layer_input)))
        output_layer_input1 = np.dot(hidden_layer_activations, wout_local)
        output_layer_input = output_layer_input1 + bout_local
        output = (1 / (1 + np.exp(-output_layer_input)))

        # Backpropagation
        Error = y - output
        slope_output_layer = (output * (1 - output))
        slope_hidden_layer = (hidden_layer_activations * (1 - hidden_layer_activations))
        d_output = Error * slope_output_layer
        Error_at_hidden_layer = np.dot(d_output, np.transpose(wout_local))
        d_hidden_layer = Error_at_hidden_layer * slope_hidden_layer
        wout_local += (np.dot(np.transpose(hidden_layer_activations), d_output)) * lr
        bout_local += np.sum(d_output, 0, True) * lr
        # bout += np.sum(d_output, axis=0,keepdims=True) *lr # pure python version
        wh_local += (np.dot(np.transpose(X), d_hidden_layer)) * lr
        bh_local += np.sum(d_hidden_layer, 0, True) * lr
        # bh += np.sum(d_hidden_layer, axis=0,keepdims=True) *lr # pure python version
    # print(output, '\n')


# Variable initialization
# Input array
X = np.array([[1, 0, 1, 0], [1, 0, 1, 1], [0, 1, 0, 1]])

# Output
output_y = np.array([[1], [1], [0]])
num_iter = 200  # Setting training iterations
lr = 0.1  # Setting learning rate
input_layer_neurons = X.shape[1]  # number of features in data set
hidden_layer_neurons = 3  # number of hidden layers neurons
output_layer_neurons = 1  # number of neurons at output layer

# weight and bias initialization
np.random.seed(0)
wh = np.random.uniform(size=(input_layer_neurons, hidden_layer_neurons))
bh = np.random.uniform(size=(1, hidden_layer_neurons))
wout = np.random.uniform(size=(hidden_layer_neurons, output_layer_neurons))
bout = np.random.uniform(size=(1, output_layer_neurons))

main_phylanx(X, output_y, wh, bh, wout, bout, lr, num_iter)
