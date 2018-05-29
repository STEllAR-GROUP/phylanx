#!/usr/bin/env python
#
# Copyright (c) 2018 Christopher Taylor
# Copyright (c) 2018 Parsa Amini
# Copyright (c) 2018 Hartmut Kaiser
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

from phylanx.ast import *
import numpy as np


@Phylanx
def initialize_centroids(points, k):
    centroids = points
    shuffle(centroids)
    return centroids[:k]


@Phylanx
def closest_centroid(points, centroids):
    points_x = add_dim(slice_column(points, 0))
    points_y = add_dim(slice_column(points, 1))
    centroids_x = slice_column(centroids, 0)
    centroids_y = slice_column(centroids, 1)
    return argmin(sqrt(
        power(points_x - centroids_x, 2) + power(points_y - centroids_y, 2)
    ), 0)


@Phylanx
def move_centroids(points, closest, centroids):
    return map(
        lambda k: mean(points * add_dim(closest == k), 1),
        range(shape(centroids, 0))
    )


@Phylanx
def kmeans(points, k, iterations):
    centroids = initialize_centroids(points, k)
    for i in range(iterations):
        centroids = apply(
            vstack,
            move_centroids(
                points,
                closest_centroid(points, centroids),
                centroids)
        )
    return centroids


if __name__ == '__main__':
    points = np.vstack((
        (np.random.randn(150, 2) * 0.75 + np.array([1, 0])),
        (np.random.randn(50, 2) * 0.25 + np.array([-0.5, 0.5])),
        (np.random.randn(50, 2) * 0.5 + np.array([-0.5, -0.5]))
    ))
    print('Cluster centroids are:\n', kmeans(points, 3, 2))
