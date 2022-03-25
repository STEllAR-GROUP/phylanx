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
# flake8: noqa

from phylanx import Phylanx, PhylanxSession
import argparse
import csv
import numpy as np
import os
import time

np.set_printoptions(threshold=np.inf)
PhylanxSession.init(16)

@Phylanx
def initialize_centroids(points, k):
    centroids = points
    np.shuffle(centroids)
    return centroids[:k]


@Phylanx
def closest_centroid(points, centroids):
    points_x = np.expand_dims(np.slice_column(points, 0), -1)
    points_y = np.expand_dims(np.slice_column(points, 1), -1)
    centroids_x = np.slice_column(centroids, 0)
    centroids_y = np.slice_column(centroids, 1)
    return np.argmin(np.sqrt(
        np.power(points_x - centroids_x, 2) + np.power(points_y - centroids_y, 2)
    ), 1)


@Phylanx
def move_centroids(points, closest, centroids):
    return np.fmap(
        lambda k: np.sum(points * np.expand_dims(closest == k, -1), 0)
        / np.sum(closest == k, 0),
        range(np.shape(centroids, 0))
    )


@Phylanx
def kmeans_t(points, k, iterations):
    centroids = initialize_centroids(points, k)
    for i in range(iterations):
        centroids = np.apply(
            np.vstack,
            [move_centroids(
                points,
                closest_centroid(points, centroids),
                centroids)]
        )
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
        print('Number of points: ', len(args.points))
        print('Points')
        print(args.points)
        print('Numebr of iterations: ', args.iterations)
        print('Cluster centroids are:\n',
              kmeans_t(args.points, args.centroids, args.iterations))
    execution_time = time.time() - start_time
    print('Time:', execution_time)


if __name__ == '__main__':
    main()
