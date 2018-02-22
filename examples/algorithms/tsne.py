#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 Christopher Taylor 
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#  Implementation ported from https://github.com/nlml/tsne_raw
#  Adjoining blog post: https://nlml.github.io/in-raw-numpy/in-raw-numpy-t-sne/
#
#  Special thanks to Liam Schoneveld for providing implementation and notes in
#  the public domain.
#
import phylanx
from phylanx.ast import *
from phylanx.ast.utils import printout 

@Phylanx("PhySL")
def neg_squared_euc_distance(X):
    sum_X = sum( pow(X, 2.0), axis=1 )
    D = ( ( transpose( (-2.0 * dot(X, transpose(X))) + sum_X) ) + sum_X )
    return -D

@Phylanx("PhySL")
def softmax(X, diag_zero=True):
    e_x = exp( X - reshape( max(X, axis=1), [-1, 1] ) )
    if diag_zero:
        fill_diagonal(e_x, 0.0)
    e_x = e_x + 1e-8

    return e_x / reshape( sum(e_x, axis=1), [-1, 1] )

@Phylanx("PhySL")
def calc_prob_mat(distances, sigmas=None):
    if sigmas is not None:
        two_sig_sq = 2.0 * pow( reshape(sigmas, [-1, 1]), 2.0 )
        return softmax( distances / two_sig_sq )
    else:
        return softmax(distances)
@Phylanx("PhySL")
def bin_search(eval_fn, target, i_iter, sigma_, tol=1e-10, max_iter=10000, lower=1e-20, upper=1000.0):
    for i in range(max_iter):
        guess = (lower + upper) / 2.0
        val = eval_fn(guess, i, sigma)
        if val > target:
            upper = guess
        else:
            lower = guess

        if abs(val - target) <= tol:
            break
    return guess

@Phylanx("PhySL")
def calc_perplexity(prob_matrix):
    entropy = -sum( prob_matrix * log2(prob_matrix), axis=1)
    perplexity = pow(2.0, entropy)
    return entropy

@Phylanx("PhySL")
def perplexity(distances, sigmas):
    return calc_perplexity(calc_prob_matrix(distances, sigmas))

@Phylanx("PhySL")
def eval_fn(distances, i, sigma):
    return perplexity( distances[i:i+1, :], sigma )

@Phylanx("PhySL")
def find_optimal_sigmas(distances, target_perplexity):
    N = shape(distances, 0)
    sigmas = array(0.0, N)
    # TODO: parallelize this block?
    #
    for i in range( N ):
        correct_sigma = bin_search(eval_fn, target_perplexity, i, sigma)
        sigmas = vstack(sigmas, correct_sigma)
    return sigmas

@Phylanx("PhySL")
def p_conditional_to_joint(P):
    return (P + transpose(P)) / (2.0 * float(shape(P, 0)))

# NOTE: Ignore these for now...
#
#@Phylanx("PhySL")
#def q_joint(Y):
#    dists = neg_squared_euc_dists(Y)
#    exp_dists = exp(distances)
#    # TODO: does fill return a ref to inv_distances?
#    #
#    fill_diagonal(exp_dists, 0.0)
#
#    # TODO: return tuples?
#    #
#    return exp_dists / sum(exp_dists), None
#
#@Phylanx("PhySL")
#def symmetric_sne_grad(P, Q, Y):
#    pq_diff = P-Q
#    pq_expanded = expand_dims(pq_diff, 2) # NxNx1
#    y_diffs = expand_dims(Y, 1) - expand_dims(Y, 0) # NxNx2
#    grad = 4.0 * sum(pq_expanded * y_diffs, axis=1) # Nx2
#    return grad

@Phylanx("PhySL")
def q_tsne(Y):
    distances = neg_squared_euc_dists(Y)
    inv_distances = pow(1.0-distances, -1.0)
    # TODO: does fill return a ref to inv_distnaces?
    #
    fill_diagonal(inv_distances, 0.0)

    # TODO: return tuples?
    #
    return inv_distances / sum(inv_distances), inv_distances

@Phylanx("PhySL")
def tsne_grad(P, Q, Y, distances):
    pq_diff = P - Q
    pq_expanded = expand_dims(pq_diff, 2) # NxNx1
    y_diffs = expanded_dims(Y, 1) - expanded_dims(Y, 0) # NxNx2
    distances_expanded = expand_dims(distances, 2) # NxNx1
    y_diffs_wt = y_diffs * distances_expanded # NxNx2
    grad = 4.0 * sum(pq_expanded * y_diffs_wt, axis=1) # Nx2
    return grad

@Phylanx("PhySL")
def p_joint(X, target_preplexity):
    distances = neg_squared_euc_dists(X)
    sigmas = find_optimal_sigmas(distances, target_perplexity)
    p_conditional = calc_prob_matrix(distances, sigmas)
    P = p_conditional_joint(p_conditional)
    return P

@Phylanx("PhySL")
def estimate_sne(X, y, P, rng, num_iters, q_fn, grad_fn, learning_rate, momentum):
    shape_x_arr = array(0, 2)
    shape_x_arr[0] = shape(X, 0)
    shape_x_arr[1] = 2

    # TODO: https://docs.scipy.org/doc/numpy/reference/generated/numpy.random.RandomState.html
    Y = random('normal',(0.0, 0.0001, shape_x_arr)

    # TODO: original line didn't have shape_x_arr...originally expressed as...
    #Y = rng.normal(0.0, 0.0001, [shape(X, 0), 2])
    #

    if momentum:
        Y_m2 = Y # TODO: copy 
        Y_m1 = Y # TODO: copy

    for i in range(num_iters):
        Q, distances = q_fn(Y)
        grads = grad_fn(P, Q, Y, distances)
        Y = Y - learning_rate * grads
        if momentum:
            Y += momentum * (Y_m1 - Y_m2)
            Y_m2 = Y_m1 # TODO: copy
            Y_m1 = Y # TODO: copy

     return Y

if __name__ == "__main__":
    PERPLEX = 20
    NUM_ITERS = 500
    LR = 10.0
    M = 0.9

    P = p_joint(X, PERPLEX)
    Y = estimate_sne(X, y, P, num_iters=NUM_ITERS, q_fn=q_tsne, grad_fn=tsne_grad, learning_rate=LR, momentum=M)
    printout(Y)
