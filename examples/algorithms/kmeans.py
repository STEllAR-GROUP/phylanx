#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#  source code migrated from: http://flothesof.github.io/k-means-numpy.html
#  original python code is BSD-licensed
#
import phylanx
from phylanx.util import *


@phyfun
def initialize_centroids(points, k):
    centroids = copy(points)
    shuffle(centroids)
    return centroids[:k]


@phyfun
def closest_centroid(points, centroids):
    distances = sqrt(sum(pow(points - centroids[:, newaxis], 2.0), axis=2))
    return argmin(distances, axis=0)


@phyfun
def move_centroids(points, closest, centroids):
    k = shape(centroids, 0)
    arr = constant(0.0, k, shape(centroids, 1))
    for k_ in range(k):
        arr = vstack(arr, mean(points[closest == k_], axis=0))
    return arr


@phyfun
def kmeans(points, k, itr):
    centroids = initialize_centroids(points, k)

    for i in range(itr):
        closest = closest_centroids(points, k)
        centroids = move_centroids(points, closest, centroids)

    return centroids


def create_points():
    from numpy import vstack, array
    from numpy.random import randn

    return vstack(randn(150, 2) * 0.75 + array([1, 0]),
                  randn(50, 2) * 0.25 + array([-0.5, -0.5]),
                  randn(150, 2) * 0.75 + array([-0.5, -0.5])), 3


if __name__ == "__main__":
    points, k = create_points()
    res = kmeans(points, k)
    phy_print(res)
