import numpy as np


def main_phython(X, y, hiddenlayer_neurons, output_neurons, lr, num_iter):
    # weight and bias initialization
    np.random.seed(0)
    wh = np.random.uniform(size=(inputlayer_neurons, hiddenlayer_neurons))
    bh = np.random.uniform(size=(1, hiddenlayer_neurons))
    wout = np.random.uniform(size=(hiddenlayer_neurons, output_neurons))
    bout = np.random.uniform(size=(1, output_neurons))
    output = 0
    for i in range(num_iter):
        # print(i)
        hidden_layer_input1 = np.dot(X, wh)
        hidden_layer_input = hidden_layer_input1 + bh
        hiddenlayer_activations = (1 / (1 + np.exp(-hidden_layer_input)))
        output_layer_input1 = np.dot(hiddenlayer_activations, wout)
        output_layer_input = output_layer_input1 + bout
        output = (1 / (1 + np.exp(-output_layer_input)))

        # Backpropagation
        E = y - output
        slope_output_layer = (output * (1 - output))
        slope_hidden_layer = (hiddenlayer_activations * (1 - hiddenlayer_activations))
        d_output = E * slope_output_layer
        Error_at_hidden_layer = np.dot(d_output, np.transpose(wout))  # some problem
        d_hiddenlayer = Error_at_hidden_layer * slope_hidden_layer
        wout += (np.dot(np.transpose(hiddenlayer_activations), d_output)) * lr
        bout += np.sum(d_output, axis=0, keepdims=True) * lr
        wh += (np.dot(np.transpose(X), d_hiddenlayer)) * lr
        bh += np.sum(d_hiddenlayer, axis=0, keepdims=True) * lr
    print(output, '\n')


# Input array
# X = np.array([[1, 0, 1, 0], [1, 0, 1, 1], [0, 1, 0, 1]]) # 1st training dataset X

X = np.array([[15.04, 16.74], [13.82, 24.49], [12.54, 16.32], [23.09, 19.83],
              [9.268, 12.87], [9.676, 13.14], [12.22, 20.04], [11.06, 17.12],
              [16.3, 15.7], [15.46, 23.95], [11.74, 14.69], [14.81, 14.7],
              [13.4, 20.52], [14.58, 13.66], [15.05, 19.07], [11.34, 18.61],
              [18.31, 20.58], [19.89, 20.26], [12.88, 18.22], [12.75, 16.7],
              [9.295, 13.9], [24.63, 21.6], [11.26, 19.83], [13.71, 18.68],
              [9.847, 15.68], [8.571, 13.1], [13.46, 18.75], [12.34, 12.27],
              [13.94, 13.17], [12.07, 13.44]])  # 2nd training dataset X

# Output
# output_y = np.array([[1], [1], [0]]) # 1st training dataset y
output_y = np.array([1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1,
                     1])  # 2nd training dataset y

# Variable initialization
num_iter = 5000  # Setting training iterations
lr = 0.1  # Setting learning rate
inputlayer_neurons = X.shape[1]  # number of features in data set
hiddenlayer_neurons = 3  # number of hidden layers neurons
output_neurons = 30  # number of neurons at output layer

main_phython(X, output_y, hiddenlayer_neurons, output_neurons, lr, num_iter)
