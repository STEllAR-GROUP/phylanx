#!/usr/bin/env python
#
# Copyright (c) 2018 Christopher Taylor
# Copyright (c) 2018 Parsa Amini
# Copyright (c) 2018 Hartmut Kaiser
# Copyright (c) 2020 Bita Hasheminezhad
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Phylanx K-Means algorithm example in Python. Iteratively clusters 250
# randomly generated points into 3 clusters for the specified number of
# iterations.
#
# Code adapted from: http://flothesof.github.io/k-means-numpy.html
# Original source code is BSD-licensed
#
# \param iterations Number of iterations
# \returns the cluster centroids

import argparse
import numpy as np
import time


def closest_centroid(points, centroids):
    distances = np.sqrt(((points - centroids[:, np.newaxis]) ** 2).sum(axis=2))
    return np.argmin(distances, axis=0)


def move_centroids(points, closest, centroids):
    return np.array([points[closest == k].mean(axis=0) for k in range(
        centroids.shape[0])])


def kmeans(points, iterations, initial_centroids, enable_output):
    centroids = initial_centroids
    for i in range(iterations):
        if enable_output:
            print("centroids in iteration ", i, ": ", centroids)
        centroids = move_centroids(points, closest_centroid(points, centroids),
                                   centroids)
    return centroids


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--iterations', type=int, default=5,
        help='number of iterations to run')
    parser.add_argument(
        '--enable_output', type=bool, default=False,
        help='nenable progress output')
    return parser.parse_args()


def main():
    args = parse_args()
    points = np.array([[0.75, 0.25], [1.25, 3.], [2.75, 3.],
                       [1., 0.25], [3., 0.25], [1.5, 2.75],
                       [3., 0.], [3., 3.], [2.75, 2.25],
                       [2.5, 1.75], [11., 4.5], [10.5, 3.],
                       [9.5, 5.], [10., 3.5], [11.25, 6.25],
                       [9.25, 3.], [11.75, 0.75], [10., 2.75],
                       [12., 5.75], [15.25, 2.25], [-3., 14.75],
                       [4.75, 10.25], [-1.25, 13.25], [-0.25, 13.],
                       [0.75, 9.25], [-0.25, 9.25], [2.5, 8.75],
                       [-2.25, 10.25], [-2.25, 10.75], [2.5, 11.75]])

    initial_centroids = np.array([[-3., 14.75], [1.5, 2.75], [3., 0.]])

    # time the execution
    start_time = time.time()

    print('Centroids are: ',
          kmeans(points, args.iterations, initial_centroids, args.enable_output), '\n')
    execution_time = time.time() - start_time
    print('Calculated in: ', execution_time)


if __name__ == '__main__':
    main()
