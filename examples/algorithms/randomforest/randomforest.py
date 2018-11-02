from numpy.random import rand
from numpy import argsort, sum, float64, zeros, max, nan

# code ported from https://machinelearningmastery.com/implement-random-forest-scratch-python/

def test_split(idx, val, dataset):
    left, right = list(), list()
    for row in dataset:
        if row[idx] < val:
            left.append(row)
        else:
            right.append(row)

    return (left, right, nan, nan) # tuple of lists

def gini_index(groups, classes):
    groups_len = map(lambda x: len(x), groups)
    n_instances = float64(sum(groups_len))
    gini = 0.0
    classes_len = len(classes)
    for (group, group_len) in filter(lambda x: x[1] > 0, zip(groups, groups_len)):
        score = 0.0
        p = zeros(classes_len)
        for row in group:
            p[classes[row[-1]]] += 1
        p /= size
        score += p * p

        gini += (1.0 - score) * (size / n_instances)

    return gini

def get_split(dataset, n_features):
    cls_values = dict()
    cls_counter = 0
    for row in range(len(dataset)):
        cls_val = dataset[row][-1]
        if cls_values.has_key(cls_val):
            cls_values[cls_val] = cls_counter
            cls_counter+=1

    b_idx, b_val, b_score, b_groups = nan, nan, nan, {}
    idx_w = rand(len(dataset[0]-1))
    idx = list(range(idx_w))
    features = idx[ argsort(idx_w) ][:n_features]

    for feature in features:
        for row in range(len(dataset)):
            groups = test_split(feature, dataset[row,:], dataset)
            gini = gini_index(groups, cls_values)
            if gini < b_score: 
                b_idx, b_val, b_score, b_groups = feature, dataset[row,:], gini, groups

    return {'idx' : b_idx, 'val' : b_val, 'groups' : b_groups }

def to_terminal(classes, group):
    outcome_hist = zeros(len(classes))

    def update(hist, val):
        hist[val] += 1

    map(lambda g: update(outcome_hist, classes[g[-1]]), group)

    return max(outcome_hist)

def split(node, max_depth, min_sz, n_features, depth):
    IDX, VAL = 'idx', 'val'
    GRP, LFT, RHT, LW, RW = 'groups', 0, 1, 2, 3 #'left', 'right', 'lw', 'rw'

    (left, right, lw, rw) = node[GRP]
    if len(left) == 0 or len(right) == 0:
        if len(node) < 4:
            lr_list = left + right
            term = to_terminal(lr_list)
            node[GRP] = (term, term)

        if depth >= max_depth:
            lterm = to_terminal(left)
            rterm = to_terminal(right)
            node[GRP] = (list(), list(), lterm, rterm)

        if len(left) <= min_sz:
            node[GRP][LFT] = (list(), list(), to_terminal(left), nan)
        else:
            node[GRP][LFT] = get_split(left, n_features)
            split(node[GRP][LFT], max_depth, min_sz, n_features, depth+1)

        if len(right) <= min_sz:
             node[GRP][RHT] = (list(), list(), nan, to_terminal(right))
        else:
            node[GRP][RHT] = get_split(right, n_features)
            split(node[GRP][RHT], max_depth, min_sz, n_features, depth+1)
          
def build_tree(train, max_depth, min_sz, n_features):
    root = get_split(train, n_features)
    split(root, max_depth, min_size, n_features, 1)
    return root

def predict(node, row):
    if row[node[0]] < node[1]:
        if node[2].has_key('lterm'):
            return node[2][0]
        else:
            return predict(node[2][0], row)
    else:
        if node[2].has_key('rterm'):
            return node[2][1]
        else:
            return predict(node[2][1], row)

def subsample(dataset, ratio):
    n_sample = round(len(dataset) * ratio) 
    idx_w = list(map(lambda x: rand(), range(len(dataset))))
    idx_s = argsort(idx_w)
    sample = list(map(lambda i: idx_w[idx_s[i]], range(n_sample)))
    return sample

def bagging_predict(trees, row, classes):
    predictions = list(map(lambda tree: predict(tree, row) for tree in trees))
    cls_hist = zeros(len(classes.keys()))
    for p in range(len(predictions)):
        cls_hist[classes[p]] += 1
    return max(cls_hist)

def random_forest(train, test, max_depth, min_sz, sample_sz, n_trees, n_features):
    trees = list(map(lambda x: build_tree(subsample(train, sample_sz), max_depth, min_sz, n_features), prange(n_trees)))
    predictions = list(map(lambda t: bagging_predict(t, row), test))
    return (trees, predictions)

if __name__ == "__main__":
