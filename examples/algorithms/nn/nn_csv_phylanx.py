# Inspired by SUNIL RAY
# https://www.analyticsvidhya.com/blog/2017/05/neural-network-from-scratch-in-python-and-r/
# Copyright (c) 2019 Weile Wei
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

import pandas as pd
import numpy as np
import time
import argparse
import sys
from phylanx import Phylanx

if not len(sys.argv) == 6:
    print(
        "This program requires the following 6 arguments seperated by a space "
    )
    print("iterations Row_START Row_STOP Col_START Col_STOP")
    exit(-57)

parser = argparse.ArgumentParser(
    description='Iteration and slicing operations')
parser.add_argument(
    'integers',
    metavar='Iteration  + Slicing Parameters',
    type=int,
    nargs='+',
    help='iterations, Row_START, Row_STOP, Col_START, Col_STOP')

args = parser.parse_args()
print("Command Line: ", args.integers[0], args.integers[1], args.integers[2],
      args.integers[3], args.integers[4])

iterations = args.integers[0]
row_start = args.integers[1]
row_stop = args.integers[2]
col_start = args.integers[3]
col_stop = args.integers[4]

treading = time.time()

print("Reading Data ....")
df = pd.read_csv('/phylanx-data/CSV/10kx10k.csv', sep=',', header=None)
df = df.values
print("Slicing ....")
X = df[row_start:row_stop, col_start:col_stop]
Y = np.squeeze(np.asarray(df[row_start:row_stop, 10000:10001]))
trslice = time.time()
print("Reading and Slicing done in ", trslice - treading, " s ")

print("Starting NN ....")

tnn = time.time()


@Phylanx
def SingleLayerNeuralNetwork(X, y, num_iter, lr):

    input_layer_neurons = np.shape(X)[1]  # number of features in data set
    output_neurons = np.shape(y)[0]  # number of neurons at output layer
    hidden_layer_neurons = input_layer_neurons / 2

    # weight and bias initialization
    wh = random([input_layer_neurons, hidden_layer_neurons], "uniform") # noqa
    bh = random([1, hidden_layer_neurons], "uniform") # noqa
    wout = random([hidden_layer_neurons, output_neurons], "uniform")  # noqa
    bout = random([1, output_neurons], "uniform") # noqa

    for i in range(num_iter):
        # Forward
        hidden_layer_input1 = np.dot(X, wh)
        hidden_layer_input = hidden_layer_input1 + bh
        hidden_layer_activations = (1 / (1 + np.exp(-hidden_layer_input)))
        output_layer_input1 = np.dot(hidden_layer_activations, wout)
        output_layer_input = output_layer_input1 + bout
        output = (1 / (1 + np.exp(-output_layer_input)))

        # Backpropagation
        Error = y - output
        slope_output_layer = (output * (1 - output))
        slope_hidden_layer = (
            hidden_layer_activations * (1 - hidden_layer_activations))
        d_output = Error * slope_output_layer
        Error_at_hidden_layer = np.dot(d_output, np.transpose(wout))
        d_hidden_layer = Error_at_hidden_layer * slope_hidden_layer
        wout += (np.dot(np.transpose(hidden_layer_activations), d_output)) * lr
        bout += np.sum(d_output, 0, True) * lr
        # bout += np.sum(d_output, axis=0,keepdims=True) *lr # pure python version
        wh += (np.dot(np.transpose(X), d_hidden_layer)) * lr
        bh += np.sum(d_hidden_layer, 0, True) * lr
        # bh += np.sum(d_hidden_layer, axis=0,keepdims=True) *lr # pure python version
    # print(output, '\n')
    return wh


weights = SingleLayerNeuralNetwork(X, Y, iterations, 1e-5)

tfinal = time.time()

print(" result = ", weights, " time: ", tfinal - tnn)
