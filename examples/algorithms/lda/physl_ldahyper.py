#  Copyright (c) 2018 Chris Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#  Code ported from java to python based on the mallet implementation:
#
#  http://mallet.cs.umass.edu/index.php

import numpy as np
from numpy.random import randint
from scipy.io import loadmat
import sys
import phylanx
from phylanx.ast import Phylanx
from phylanx.ast import *


@Phylanx("PhySL")
def learnParameters(alpha,
                    locTopicDocCounts,
                    locDocLengthCounts,
                    shape=1.001, scale=1.0,
                    numIterations=1):

    i = 0
    k = 0
    parametersSum = np.sum(alpha)
    oldParametersK = 0.0
    currentDigamma = 0.0
    denominator = 1e-10

    nonZeroLimit = 0
    nonZeroLimits = np.zeros(locTopicDocCounts.shape[0])
    nonZeroLimits[:] = -1

    histogram = np.zeros(locTopicDocCounts.shape[1])
    for i in range(locTopicDocCounts.shape[0]):
        histogram[:] = locTopicDocCounts[i, :]
        for k in range(histogram.shape[0]):
            if histogram[k] > 0:
                nonZeroLimits[i] = k

    for itr in range(numIterations):
        denominator = 1e-10
        currentDigamma = 0.0

        for i in range(locDocLengthCounts.shape[0]):
            currentDigamma += 1.0 / ((
                parametersSum + np.float64(i) - 1.0) + 1e-100
            )
            denominator += locDocLengthCounts[i] * currentDigamma

        denominator -= 1.0 / scale
        parametersSum = 0.0
        for k in range(alpha.shape[0]):
            nonZeroLimit = nonZeroLimits[k]
            oldParametersK = alpha[k]
            currentDigamma = 0.0
            histogram[:] = locTopicDocCounts[k, :]
            for i in range(np.int64(np.round(nonZeroLimit))):
                currentDigamma += 1.0 / (oldParametersK + np.float64(i) - 1.0)
                alpha[k] += histogram[i] * currentDigamma
            alpha[k] = (oldParametersK * alpha[k] + shape) / denominator
            parametersSum += alpha[k]

    return parametersSum


@Phylanx("PhySL")
def digamma(z):
    EULER_MASCHERONI = -0.5772156649015328606065121

    DIGAMMA_COEF_1 = 1.0 / 12.0
    DIGAMMA_COEF_2 = 1.0 / 120.0
    DIGAMMA_COEF_3 = 1.0 / 252.0
    DIGAMMA_COEF_4 = 1.0 / 240.0
    DIGAMMA_COEF_5 = 1.0 / 132.0
    DIGAMMA_COEF_6 = 691.0 / 32760.0
    DIGAMMA_COEF_7 = 1.0 / 12.0
    # DIGAMMA_COEF_8 = 3617.0/8160.0
    # DIGAMMA_COEF_9 = 43867.0/14364.0
    # DIGAMMA_COEF_10 = 174611.0/6600.0

    DIGAMMA_LARGE = 9.5
    DIGAMMA_SMALL = 0.000001

    invZ = (1.0 / np.float64(z))

    if z < DIGAMMA_SMALL:
        return EULER_MASCHERONI - invZ

    psi = 0.0
    while z < DIGAMMA_LARGE:
        psi -= invZ
        z += 1.0
    invZSquared = invZ * invZ

    psi += np.log(z) - 0.5 * invZ
    - (invZSquared * (DIGAMMA_COEF_1 - invZSquared * (
        DIGAMMA_COEF_2 - invZSquared * (
            DIGAMMA_COEF_3 - invZSquared * (
                DIGAMMA_COEF_4 - invZSquared * (
                    DIGAMMA_COEF_5 - invZSquared * (
                        DIGAMMA_COEF_6 - invZSquared * (DIGAMMA_COEF_7)
                    )
                )
            )
        )
    )))

    return psi


@Phylanx("PhySL")
def learnSymmetricConcentration(countHistogram, topicSizeHistogram,
                                numTypes, betaSum, optitr=200):
    currentDigamma = 0.0
    largestNonZeroCount = 0
    nonZeroLengthIndex = np.zeros(topicSizeHistogram.shape[0])
    largestNonZeroCount = np.argmax(countHistogram)
    idxs = np.where(topicSizeHistogram > 0)[0]
    denseIdx = len(idxs)
    nonZeroLengthIndex[idxs] = idxs
    denseIdxSize = denseIdx
    fnumTypes = np.float64(numTypes)

    for i in range(optitr):
        currentParameter = betaSum / fnumTypes
        currentDigamma = 0.0
        numerator = 0
        for idx in range(largestNonZeroCount):
            currentDigamma += 1.0 / (currentParameter + idx - 1.0)
            numerator += countHistogram[idx] * currentDigamma

        currentDigamma = 0.0
        denominator = 1e-10
        previousLength = 0
        cachedDigamma = digamma(betaSum)
        for denseIdx in range(denseIdxSize):
            length = nonZeroLengthIndex[denseIdx]
            if length - previousLength > 20:
                currentDigamma = digamma(betaSum + np.float64(length))
                - cachedDigamma
            else:
                for idx in range(np.int64(previousLength), np.int64(length)):
                    currentDigamma += 1.0 / (betaSum + np.float64(length))

            denominator += currentDigamma * topicSizeHistogram[
                np.int64(length)
            ]

        betaSum = currentParameter * numerator / denominator

    return betaSum


@Phylanx("PhySL")
def optimizeAlpha(topicDocCounts, docLengthCounts, maxTokens,
                  numTopics, alphaSum, usingSymmetricAlpha=False):

    locDocLengthCounts = np.zeros(maxTokens + 1)
    locTopicDocCounts = np.zeros((numTopics, maxTokens + 1))
    alpha = np.zeros(numTopics)
    alpha[:] = alphaSum / np.float64(numTopics)

    locDocLengthCounts[:] = docLengthCounts[:]
    if usingSymmetricAlpha:
        locTopicDocCounts[0, :] = np.sum(topicDocCounts, axis=0)
        alphaSum = learnSymmetricConcentration(locTopicDocCounts[0, :],
                                               locDocLengthCounts, numTopics,
                                               alphaSum)
    else:
        locTopicDocCounts[:, :] = topicDocCounts[:, :]
        alphaSum = learnParameters(alpha, locTopicDocCounts,
                                   locDocLengthCounts, 1.001, 1.0, 1)

    return alphaSum


@Phylanx("PhySL")
def optimizeBeta(typeTopicCounts, tokensPerTopic,
                 numTypes, betaSum, maxTypeCount):
    countHistogram = np.zeros(np.int64(np.round(
        maxTypeCount
    )) + 1, dtype=np.int64)

    for t in range(numTypes):
        x = np.where(typeTopicCounts[t] > 0)
        for xidx in x:
            countHistogram[np.int64(np.round(typeTopicCounts[xidx]))] += 1

    maxTopicSize = tokensPerTopic[np.argmax(tokensPerTopic)]
    topicSizeHistogram = np.zeros(np.int64(
        np.round(maxTopicSize)
    ) + 1, dtype=np.int64)

    for t in range(tokensPerTopic.shape[0]):
        topicSizeHistogram[np.int64(np.round(tokensPerTopic[t]))] += 1

    betaSum = learnSymmetricConcentration(
        countHistogram, topicSizeHistogram, numTypes, betaSum
    )

    beta = betaSum / np.float64(numTypes)
    return beta


@Phylanx("PhySL")
def estimate(training, numTopics, alpha_, beta_,
             numIterations=2000, burnInPeriod=200, optimizeInterval=50):

    # alphabet = range(training.shape[1])
    numTypes = training.shape[1]  # types means 'words'
    alphaSum = alpha_
    alpha = np.zeros(numTopics)
    alpha[:] = alphaSum / np.float(numTopics)
    beta = beta_
    betaSum = beta * np.float(numTypes)
    typeTopicCounts = np.zeros((numTypes, numTopics))
    # typeTotals = np.zeros(numTypes)
    tokensPerTopic = np.zeros(numTopics)

    docLength = np.sum(training, axis=1)
    maxTermFreq = np.max(docLength)
    # totalTokens = np.sum(training)

    initBeta = beta
    initBetaSum = betaSum

    training_indices = np.ma.masked_where(training > 0, training)

    topicSeq = list([randint(0, numTopics, np.sum(
        training[i, training_indices[i, :].mask]
    )) for i in range(training_indices.shape[0])])

    for i in range(training_indices.shape[0]):
        typ_idx = np.where(training_indices[i, :].mask)[0]
        count_idx = training[i, training_indices[i, :].mask]
        j = 0
        for typ, count in zip(typ_idx, count_idx):
            for topc in topicSeq[i][j:j + count]:
                typeTopicCounts[typ, topc] += 1
                tokensPerTopic[topc] += 1
            j += count

    docLengthCounts = np.zeros(maxTermFreq + 1)
    topicDocCounts = np.zeros((numTopics, maxTermFreq + 1))
    cachedCoefficients = np.zeros(numTopics)
    shouldSaveState = False

    UNASSIGNED_TOPIC = -1
    i = 0
    for itr in range(numIterations):
        if itr % 10 == 0:
            print(itr)

        denom = tokensPerTopic + betaSum
        smoothingOnlyMass = np.sum(alpha * beta / denom)
        cachedCoefficients = alpha / denom

        for j in range(training.shape[0]):
            localTopicCounts, localTopicIndex = \
                (np.zeros(numTopics), np.zeros(numTopics, dtype=np.int))
            # (old_topic, new_topic, topicWeightSum) = (-1, -1, 0.)
            (old_topic, new_topic) = (-1, -1)
            docLen = docLength[j]

            oneDocTopics = topicSeq[j]
            oneDocTypes = np.where(training_indices[i, :].mask)[0]
            oneDocTypesFreq = training[i, training_indices[i, :].mask]
            for t in oneDocTopics[
                np.where(oneDocTopics != UNASSIGNED_TOPIC)[0]
            ]:
                localTopicCounts[t] += 1

            msk = np.where(localTopicCounts > 0)[0]
            localTopicIndex[msk] = msk
            # nonZeroTopics = len(localTopicIndex)-1
            nonZeroTopics = msk.shape[0]
            topicBetaMass = 0.0

            denom = tokensPerTopic[localTopicIndex] + betaSum
            topicBetaMass += np.sum(
                (beta * localTopicCounts[localTopicIndex]) / denom
            )

            cachedCoefficients[localTopicIndex] = (alpha[
                localTopicIndex
            ] + localTopicCounts[localTopicIndex]) / denom

            topicTermMass = 0.0
            topicTermScores = np.zeros(numTopics)
            denseIdx = 0
            score = 0.0
            oneDocIndices = np.zeros(
                oneDocTypesFreq.shape[0] + 1, dtype=np.uint64
            )
            oneDocIndices[1:] = np.cumsum(oneDocTypesFreq)
            for pos in range(oneDocIndices.shape[0] - 1):
                pos_rng = oneDocIndices[pos:pos + 2]
                if pos_rng[1] >= oneDocTopics.shape[0]:
                    break
                topics = oneDocTopics[pos_rng]
                odtf = oneDocTypesFreq[pos]
                types_freq = [odtf] * odtf
                types = [oneDocTypes[pos]] * odtf
                for pos_range, old_topic, type_, type_freq in zip(
                    pos_rng, topics, types, types_freq
                ):
                    currentTypeTopicCounts = typeTopicCounts[type_, :]
                    if old_topic != UNASSIGNED_TOPIC:
                        denom = (tokensPerTopic[old_topic] + betaSum)
                        smoothingOnlyMass -= alpha[old_topic] * beta / denom
                        topicBetaMass -= \
                            beta * localTopicCounts[old_topic] / denom
                        localTopicCounts[old_topic] -= 1
                        if localTopicCounts[old_topic] == 0:
                            denseIdx = 0
                            denseIdx = len(list(zip(*np.where(
                                localTopicIndex != old_topic
                            ))))
                            while denseIdx < nonZeroTopics:
                                if denseIdx < len(localTopicIndex) - 1:
                                    localTopicIndex[
                                        denseIdx
                                    ] = localTopicIndex[denseIdx + 1]
                                denseIdx += 1
                            nonZeroTopics -= 1

                        tokensPerTopic[old_topic] -= 1
                        denom = tokensPerTopic[old_topic] + betaSum
                        smoothingOnlyMass += alpha[old_topic] * beta / denom

                        topicBetaMass += beta * localTopicCounts[
                            old_topic
                        ] / denom

                        cachedCoefficients[old_topic] = (alpha[
                            old_topic
                        ] + localTopicCounts[old_topic]) / denom

                    alreadyDecremented = (old_topic == UNASSIGNED_TOPIC)
                    topicTermMass = 0.0

                    idx_ = 0
                    while (idx_ < currentTypeTopicCounts.shape[0]) and \
                          (currentTypeTopicCounts[idx_] > 0):

                        currentTopic = idx_
                        currentValue = currentTypeTopicCounts[idx_]
                        if (not alreadyDecremented) and \
                           (currentTopic == old_topic):
                            currentValue -= 1
                            if currentValue == 0:
                                currentTypeTopicCounts[idx_] = 0
                            else:
                                currentTypeTopicCounts[
                                    old_topic
                                ] = currentValue
                            subidx = idx_
                            tmp = 0
                            while (
                                subidx < len(currentTypeTopicCounts) - 1) and \
                                (currentTypeTopicCounts[subidx] < \
                                    currentTypeTopicCounts[subidx + 1]):

                                tmp = currentTypeTopicCounts[subidx]
                                currentTypeTopicCounts[subidx] = \
                                    currentTypeTopicCounts[subidx + 1]
                                currentTypeTopicCounts[subidx + 1] = tmp
                                subidx += 1
                            alreadyDecremented = True
                        else:
                            score = cachedCoefficients[
                                currentTopic
                            ] * currentValue

                            topicTermMass += score
                            topicTermScores[idx_] = score
                            idx_ += 1

                    sample = np.random.rand() * \
                        (smoothingOnlyMass + topicBetaMass + topicTermMass)
                    # origSample = sample
                    new_topic = -1

                    if sample < topicTermMass:
                        i = -1
                        while sample > 0:
                            i += 1
                            sample -= topicTermScores[i]
                        new_topic = i
                        currentValue = currentTypeTopicCounts[i]
                        currentTypeTopicCounts[:] = currentTypeTopicCounts[
                            np.argsort(currentTypeTopicCounts)
                        ]
                    else:
                        sample -= topicTermMass
                        if sample < topicBetaMass:
                            sample /= beta
                            denseIdx = 0
                            while denseIdx < nonZeroTopics:
                                tpc = localTopicIndex[denseIdx]

                                sample -= localTopicCounts[tpc] / \
                                    (tokensPerTopic[tpc] + betaSum)

                                if sample <= 0.0:
                                    new_topic = tpc
                                    break
                                denseIdx += 1
                        else:
                            sample -= topicBetaMass
                            sample /= beta
                            new_topic = 0
                            sample -= alpha[new_topic] / \
                                (tokensPerTopic[new_topic] + betaSum)

                            while sample > 0.0 and new_topic + 1 < numTopics:
                                new_topic += 1
                                sample -= alpha[new_topic] / \
                                    (tokensPerTopic[new_topic] + betaSum)

                        idx_ = 0
                        while (idx_ < len(currentTypeTopicCounts) - 1) and \
                              (currentTypeTopicCounts[idx_] > 0) and \
                              (currentTypeTopicCounts[idx_] != new_topic):
                            idx_ += 1

                        if currentTypeTopicCounts[idx_] == 0:
                            currentTypeTopicCounts[idx_] = new_topic
                        else:
                            currentValue = idx_
                            currentTypeTopicCounts[idx_] = new_topic

                    if new_topic < 0:
                        new_topic = numTopics - 1

                    oneDocTopics[pos_range] = new_topic
                    denom = tokensPerTopic[new_topic] + betaSum

                    topicBetaMass -= beta * localTopicCounts[new_topic] / denom
                    localTopicCounts[new_topic] += 1

                    if localTopicCounts[new_topic] == 1:
                        denseIdx = nonZeroTopics
                        while (denseIdx > 0) and \
                              (localTopicIndex[denseIdx - 1] > new_topic):
                            localTopicIndex[denseIdx] = \
                                localTopicIndex[denseIdx - 1]
                            denseIdx -= 1

                        localTopicIndex[denseIdx] = new_topic
                        nonZeroTopics += 1

                    tokensPerTopic[new_topic] += 1
                    denom = (tokensPerTopic[new_topic] + betaSum)
                    cachedCoefficients[new_topic] = \
                        (alpha[new_topic] + localTopicCounts[new_topic]) \
                        / denom

                    smoothingOnlyMass += (alpha[new_topic] + beta) / denom
                    topicBetaMass += beta * localTopicCounts[new_topic] / denom

            if shouldSaveState:
                docLengthCounts[docLen] += 1
                for denseIdx in range(nonZeroTopics):
                    topic = localTopicIndex[denseIdx]
                    topicDocCounts[topic, localTopicCounts[topic]] += 1

            for denseIdx in range(nonZeroTopics):
                topic = localTopicIndex[denseIdx]
                cachedCoefficients[topic] = alpha[topic] / \
                    (tokensPerTopic[topic] + betaSum)

        if itr > burnInPeriod and optimizeInterval != 0 and \
           itr % optimizeInterval == 0:

            alpha_ = optimizeAlpha(topicDocCounts, docLengthCounts,
                                   maxTermFreq, numTopics, np.sum(alpha))

            ttc_mx = np.max(typeTopicCounts)

            optimizeBeta(typeTopicCounts, tokensPerTopic, numTypes, betaSum,
                         ttc_mx)

            beta = initBeta
            betaSum = initBetaSum

    return (docLengthCounts, topicDocCounts)


if __name__ == "__main__":
    matlab_vars = dict()
    # if len(sys.argv) > 1 and sys.argv[1] == 0:
    #     loadmat('../datasets/docword.nips.train.mat', matlab_vars)
    # else:
    #     loadmat('../datasets/docword.kos.train.mat', matlab_vars)
    #
    loadmat(sys.argv[1], matlab_vars)

    D = int(matlab_vars['D'][0][0])
    W = int(matlab_vars['W'][0][0])
    N = int(matlab_vars['N'][0][0])
    T = int(4)

    w = matlab_vars['w']
    d = matlab_vars['d']

    w.reshape(w.shape[0])
    d.reshape(d.shape[0])

    w -= 1
    d -= 1

    z = np.random.randint(0, T, size=T * N)
    beta = 0.01
    alpha = 0.5
    iters = 1000
    word_doc_mat = np.zeros((D, W), dtype=np.int)
    for j in range(N):
        word_doc_mat[d[j], w[j]] += 1

    doclenhist, topicdochist = \
        estimate(word_doc_mat, T, alpha, beta, iters)
