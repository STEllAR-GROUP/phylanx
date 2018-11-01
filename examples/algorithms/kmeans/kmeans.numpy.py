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

import argparse
import csv
import numpy as np
import time
import os


def initialize_centroids(points, k):
    centroids = points.copy()
    np.random.shuffle(centroids)
    return centroids[:k]


def closest_centroid(points, centroids):
    distances = np.sqrt(((points - centroids[:, np.newaxis]) ** 2).sum(axis=2))
    return np.argmin(distances, axis=0)


def move_centroids(points, closest, centroids):
    return np.array([points[closest == k].mean(axis=0) for k in range(
        centroids.shape[0])])


def kmeans(points, k, iterations):
    centroids = initialize_centroids(points, k)
    for i in range(iterations):
        centroids = move_centroids(points, closest_centroid(points, centroids),
                                   centroids)
    return centroids


def generate_random(centroids, points):
    # a portion of total number of points cluster around each centroid
    raw_shares = np.random.rand(centroids)
    shares = (points * raw_shares / np.sum(raw_shares)).astype(int)
    raw_points_collection = []

    for i in shares:
        # random points in each cluster gather around a random centroid
        # a random factor is multiplied to separate clusters
        raw_points_collection.append(
            np.random.rand(i, 2) * np.random.rand() + np.random.rand(2))

    return np.vstack(raw_points_collection)


def csv_records(path):
    if os.path.exists(path):
        with argparse.FileType('r')(path) as csv_file:
            data = [d for d in csv.reader(csv_file, delimiter=',')]
            return np.asarray(data, dtype=np.float_)
    if path.isdigit():
        return int(path)
    raise ValueError("provided path argument is not either an integer or a valid path")


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--centroids', type=int, default=3,
        help='number of centroids')
    parser.add_argument(
        '--iterations', type=int, default=2,
        help='number of iterations to run')
    parser.add_argument(
        '--points', type=csv_records, default=250,
        help='number of random points to generate or path to CSV file containing points')
    parser.add_argument(
        '--dry-run', type=bool, nargs='?', const=True,
        default=False)
    return parser.parse_args()


def main():
    args = parse_args()

    # points should be replaced with actual random points in case it contains
    # the set count of random points
    if isinstance(args.points, int):
        args.points = generate_random(args.centroids, args.points)

    # time the execution
    start_time = time.time()

    # print what is going to be run and do not run
    if args.dry_run:
        print('kmeans', args.points.shape, args.centroids, args.iterations)
    else:
        print('Cluster centroids are:\n',
              kmeans(args.points, args.centroids, args.iterations))
    execution_time = time.time() - start_time
    print('Time:', execution_time)


if __name__ == '__main__':
    main()
