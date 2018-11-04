#  Copyright (c) 2018 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# significant re-working of the algorithm implementation found on this site:
#
# https://machinelearningmastery.com/implement-random-forest-scratch-python/
#

from numpy import floor, argsort, sum, sqrt, float64, int64, zeros, argmax, inf, genfromtxt, vstack, iinfo, finfo, unique
from numpy.random import randint, rand

def test_split(idx, val, dataset):
    left, right = list(), list()
    for i in range(dataset.shape[0]):
        row = dataset[i,:]
        if row[idx] < val:
            left.append(row)
        else:
            right.append(row)

    if len(left) < 1 and len(right) > 0:
        return (zeros((0,)), vstack(right))
    elif len(left) > 0 and len(right) < 0:
        return (vstack(left), zeros((0,)))

    return (vstack(left), vstack(right))

def gini_index(groups, classes):
    groups_len = list(map(lambda x: len(x), groups))
    n_instances = float64(sum(groups_len))
    gini = 0.0

    p = zeros(len(classes), dtype=float64)
    for (group, group_len) in filter(lambda x: x[1] > 0, zip(groups, groups_len)):
        for row in group:
            p[classes[int64(row[-1])]] += 1.0
        score = sum( ((p/float64(group_len)) ** 2.0) )
        gini += (1.0 - score) * float64(group_len / n_instances)
        p[:] = 0.0

    return gini

def get_split(dataset, n_features, classes):
    cls_values = zeros(len(classes), dtype=int64)
    for i in range(len(classes)):
       cls_values[classes[i]] = i 

    b_idx, b_val, b_score, b_groups = iinfo(int64).max, finfo(float64).max, finfo(float64).max, (list(), list())
    idx_w = randint(0, dataset.shape[1]-1, size=dataset.shape[1]-1) #randrange
    idx = zeros(dataset.shape[1]-1, dtype=int64)

    for i in range(dataset.shape[1]-1):
        idx[i] = i

    features = idx[argsort(idx_w)][:n_features]
    for feature in features:
        for r in range(dataset.shape[0]):
            groups = test_split(feature, dataset[r, feature], dataset)
            gini = gini_index(groups, cls_values)
            if gini < b_score:
                b_idx, b_val, b_score, b_groups = feature, dataset[r, feature], gini, groups

    return { 'index' : b_idx, 'value' : b_val, 'groups' : b_groups, 'lw' : inf, 'rw' : inf }

def to_terminal(group, classes):
    outcome_hist = zeros(len(classes), dtype=int64)
    for g in group:
        k = int64(g[-1])
        outcome_hist[classes[k]] += 1

    return argmax(outcome_hist)

def split(node, max_depth, min_sz, n_features, depth, classes):
    IDX, VAL = 'index', 'value'
    GRP, LFT, RHT, LW, RW = 'groups', 'left', 'right', 'lw', 'rw'

    (left, right) = node[GRP]
    del(node[GRP])

    if left.shape == (0,) or right.shape == (0,):
        if left.shape == (0,):
            term = to_terminal(right, classes)
        else:
            term = to_terminal(left, classes)

        node[LW] = term
        node[RW] = term
        return

    if depth >= max_depth:
        lterm = to_terminal(left, classes)
        rterm = to_terminal(right, classes)
        node[LW] = lterm
        node[RW] = rterm
        return

    if len(left) <= min_sz:
        node[LW] = to_terminal(left, classes)
    else:
        node[LFT] = get_split(left, n_features, classes)
        split(node[LFT], max_depth, min_sz, n_features, depth+1, classes) 

    if len(right) <= min_sz:
        node[RW] = to_terminal(right, classes)
    else:
        node[RHT] = get_split(right, n_features, classes)
        split(node[RHT], max_depth, min_sz, n_features, depth+1, classes)

def build_tree(train, max_depth, min_sz, n_features, classes):
    root = get_split(train, n_features, classes)
    split(root, max_depth, min_sz, n_features, 1, classes)
    return root

def node_predict(node, r):
    if r[node['index']] < node['value']:
        if node['lw'] == inf:
            return node_predict(node['left'], r)
        else:
            return node['lw']
    else:
        if node['rw'] == inf:
            return node_predict(node['right'], r)
        else:
            return node['rw']

def subsample(dataset, ratio):
    n_sample = int64(floor(len(dataset)*ratio))
    idx_w = list(map(lambda x: rand(), range(dataset.shape[0])))
    idx_s = argsort(idx_w)
    sample = vstack(map(lambda x: dataset[idx_s[x], :], range(n_sample)))
    return sample

def bagging_predict(trees, row, classes):
    predictions = list(map(lambda tree: node_predict(tree, row), trees))
    # parallel
    #
    #predictions = list(map(lambda tree: node_predict(trees[tree], row), prange(len(trees)))
    classes_vec = zeros(len(classes), dtype=int64)
    for p in predictions:
        classes_vec[classes[p]] += 1

    idx = argmax(classes_vec)
    for (k, v) in classes.items():
        if v == idx:
            return k
    return inf

def random_forest(train, max_depth, min_sz, sample_sz, n_trees):
    cls = unique(train[:,-1])
    classes = dict()
    for c in range(cls.shape[0]):
        classes[int64(cls[c])] = c

    n_features = int64(floor(sqrt(dataset.shape[0])))
    trees = list(map(lambda i: build_tree(subsample(train, sample_sz), max_depth, min_sz, n_features, classes), range(n_trees)))
    # parallel
    #
    #trees = list(map(lambda i: build_tree(subsample(train, sample_sz), max_depth, min_sz, n_features), prange(n_trees)))
    return { 'trees' : trees, 'classes' : classes }

def predict(randomforest, test):
    trees, classes = randomforest['trees'], randomforest['classes']
    predictions = list(map(lambda row: bagging_predict(trees, test[row, :], classes), range(len(test))))
    return predictions

if __name__ == "__main__":
    file_name = "../datasets/breast_cancer.csv"
    dataset = genfromtxt(file_name, skip_header=1, delimiter=",")
    max_depth = 10
    min_size = 1
    sample_size = 1.0
    n_trees = [1, 5, 10]
    train = int64(dataset.shape[0]/2)
    trees = random_forest(dataset[:train,:], max_depth, min_size, sample_size, n_trees[1])
    print('predict')
    predict = predict(trees, dataset[train:,:])
    print(predict)
