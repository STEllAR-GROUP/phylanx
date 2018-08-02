#  Copyright (c) 2018 Chris Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#  Code ported from java to python based on the mallet implementation:
#
#  http://mallet.cs.umass.edu/index.php

from phylanx.ast import *
import numpy as np
from scipy.io import loadmat

def np_simple_lda(D, W, N, T, w, d, z, alpha, beta, iters):
    from numpy import zeros, sum, argmax
    from numpy.random import random

    betaSum = W * beta
    alpha /= T # NOTE: presents a float/int error

    word_topic_count = zeros((W, T))
    doc_topic_count = zeros((D, T))
    doc_word_count = zeros((D, W))
    word_doc_topic_mat = zeros((D, W), dtype=np.int)

    for j in range(N):
        word_topic_count[ w[j], z[j] ] += 1.0
        doc_topic_count[ d[j], z[j] ] += 1.0
        doc_word_count[ d[j], w[j] ] += 1.0
        word_doc_topic_mat[ d[j], w[j] ] = argmax(random(T))

    sum_ = 0.0
    tokens_per_topic = zeros(T)
    topic_term_scores = zeros(T)
    #doc_topics = zeros((D, W))

    for t in range(T):
        tokens_per_topic[t] = sum(word_topic_count[:,t])

    old_topic = 0
    new_topic_ = -1
    sample_ = 0.0

    for itr in range(iters):
        print("iteration\t%d" % (itr,))
        for d in range(D):
            if d % 100 == 0:
                print("\tcurrent docs processed\t%d\ttotal documents to process %d" % (d,D))
            topic_seq = word_doc_topic_mat[d,:] # ref var, maybe a view?
            local_topic_counts = doc_topic_count[d,:] # ref var, maybe a view?

            for pos in range(W):
                old_topic = topic_seq[pos]
                current_token_topic_counts = word_topic_count[pos,:]

                local_topic_counts[old_topic] -= 1
                tokens_per_topic[old_topic] -= 1
                current_token_topic_counts[old_topic] -= 1

                topic_term_scores = (alpha + local_topic_counts) * \
                    ((beta + current_token_topic_counts) /
                    (betaSum + tokens_per_topic))

                sum_ = sum(topic_term_scores) 

                sample_ = random(10)[9] * sum_

                new_topic_ = -1
                while sample_ > 0.0:
                    new_topic_ += 1
                    sample_ -= topic_term_scores[new_topic_]

                word_doc_topic_mat[d, pos] = new_topic_
                local_topic_counts[new_topic_] += 1
                tokens_per_topic[new_topic_] += 1
                current_token_topic_counts[new_topic_] += 1

    # TODO: add in print out of the trained/learned model
    return

@Phylanx
def simple_lda(D, W, N, T, w, d, z, alpha, beta, iters):
    betaSum = W * beta
    alpha /= T          # NOTE: presents a float/int error

    word_topic_count = constant(0.0, make_list(W, T))
    doc_topic_count = constant(0.0, make_list(D, T))
    doc_word_count = constant(0.0, make_list(D, W))
    word_doc_topic_mat = constant(0.0, make_list(D, W))

    for j in range(N):
        word_topic_count[ w[j], z[j] ] += 1.0
        doc_topic_count[ d[j], z[j] ] += 1.0
        doc_word_count[ d[j], w[j] ] += 1.0
        word_doc_topic_mat[ d[j], w[j] ] = argmax(random(T))

    sum_ = 0.0
    tokens_per_topic = constant(0.0, T)

    for t in range(T):
        tokens_per_topic[t] = np.sum(word_topic_count[:, t])

    topic_term_scores = np.constant(0.0, T)
    # doc_topics = np.constant(0.0, list(D, W))

    old_topic = 0
    new_topic_ = -1
    sample_ = 0.0

    for itr in range(iters):
        print("iteration\t%d\ttotal docs" % (itr, D))
        for d in range(D):
            if d % 100 == 0:
                print("\tcurrent docs processed\t%d" % (d,))
            topic_seq = word_doc_topic_mat[d, :]  # ref var, maybe a view?
            local_topic_counts = doc_topic_count[d, :]  # ref var, maybe a view?

            for pos in range(W):
                old_topic = topic_seq[pos]
                current_token_topic_counts = word_topic_count[pos, :]

                local_topic_counts[old_topic] -= 1
                tokens_per_topic[old_topic] -= 1
                current_token_topic_counts[old_topic] -= 1

                topic_term_scores = (alpha + local_topic_counts) * \
                    ((beta + current_token_topic_counts) /
                        (betaSum + tokens_per_topic))

                sum_ = sum(topic_term_scores)

                sample_ = random(10)[9] * sum_

                new_topic_ = -1
                while sample_ > 0.0:
                    new_topic_ += 1
                    sample_ -= topic_term_scores[new_topic_]

                word_doc_topic_mat[d, pos] = new_topic_
                local_topic_counts[new_topic_] += 1
                tokens_per_topic[new_topic_] += 1
                current_token_topic_counts[new_topic_] += 1

    # TODO: add in print out of the trained/learned model
    return


if __name__ == "__main__":
    matlab_vars = dict()
    loadmat('../datasets/docword.kos.train.mat', matlab_vars)

    # NOTE: the load mat function returns some unexpected/unhandled
    # numpy types. wrapping those types with a call to the 'int'
    # initializer function fixed everything such that phylanx could
    # map the types into something it can comprehend.
    #
    D = int(matlab_vars['D'][0][0])
    W = int(matlab_vars['W'][0][0])
    N = int(matlab_vars['N'][0][0])
    T = 4

    w = matlab_vars['w']
    d = matlab_vars['d']

    w = w.reshape(w.shape[0])
    d = d.reshape(d.shape[0])

    w -= 1      # w min val is 1, matlab indexing artifact
    d -= 1      # d min val is 1, matlab indexing artifact

    z = np.random.randint(0, T, size=N)
    beta = 0.01
    alpha = 0.5
    iters = 10000

    simple_lda(D, W, N, T, w, d, z, alpha, beta, iters)
