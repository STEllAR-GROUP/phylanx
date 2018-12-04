#  Copyright (c) 2018 Christopher Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# significant re-working of the algorithm implementation found on this site:
#
# https://github.com/mimno/Mallet
#

from numpy import zeros, where, cumsum, argsort, argmax, log
from numpy.random import randint, rand, int64, float64
import numpy.ma as ma
import numpy as np

from scipy.io import loadmat


def learnParameters(alpha, locTopicDocCounts, locDocLengthCounts,
                    shape=1.001, scale=1.0, numIterations=1):
    i = 0
    k = 0

    parametersSum = sum(alpha)
    oldParametersK = 0.0
    currentDigamma = 0.0
    denominator = 0.0

    nonZeroLimit = 0
    nonZeroLimits = zeros(locTopicDocCounts.shape[0], dtype=int64)
    nonZeroLimits[:] = -1

    for i in range(locTopicDocCounts.shape[0]):
        for k in range(locTopicDocCounts.shape[1]):
            if locTopicDocCounts[i, k] > 0:
                nonZeroLimits[i] = k

    for itr in range(numIterations):
        denominator = 0.0
        currentDigamma = 0.0
        for i in range(locDocLengthCounts.shape[0]):
            currentDigamma += 1.0 / (parametersSum + float64(i) - 1.0)
            denominator += locDocLengthCounts[i] * currentDigamma

        denominator -= 1.0 / scale
        parametersSum = 0.0
        for k in range(alpha.shape[0]):
            nonZeroLimit = nonZeroLimits[k]
            oldParametersK = alpha[k]
            currentDigamma = 0.0
            for i in range(nonZeroLimit):
                currentDigamma += 1.0 / (oldParametersK + float64(i) - 1)
                alpha[k] += locTopicDocCounts[k, i] * currentDigamma
            alpha[k] = (oldParametersK * alpha[k] + shape) / denominator
            parametersSum += alpha[k]

    return parametersSum


def digamma(z):
    EULER_MASCHERONI = -0.5772156649015328606065121
    DIGAMMA_COEF_1 = 1.0/12.0
    DIGAMMA_COEF_2 = 1.0/120.0
    DIGAMMA_COEF_3 = 1.0/252.0
    DIGAMMA_COEF_4 = 1.0/240.0
    DIGAMMA_COEF_5 = 1.0/132.0
    DIGAMMA_COEF_6 = 691.0/32760.0
    DIGAMMA_COEF_7 = 1.0/12.0
    # DIGAMMA_COEF_8 = 3617.0/8160.0
    # DIGAMMA_COEF_9 = 43867.0/14364.0
    # DIGAMMA_COEF_10 = 174611.0/6600.0

    DIGAMMA_LARGE = 9.5
    DIGAMMA_SMALL = 0.000001

    if z < DIGAMMA_SMALL:
        return EULER_MASCHERONI - (1.0/z)

    psi = 0.0
    while z < DIGAMMA_LARGE:
        psi -= (1.0/z)
        z += 1.0

    invZ = 1.0/z
    invZSquared = invZ * invZ

    psi += log(z) - 0.5 * invZ - (
            invZSquared * (DIGAMMA_COEF_1 - invZSquared * (
                DIGAMMA_COEF_2 - invZSquared * (
                    DIGAMMA_COEF_3 - invZSquared * (
                        DIGAMMA_COEF_4 - invZSquared * (
                            DIGAMMA_COEF_5 - invZSquared * (
                                DIGAMMA_COEF_6 - invZSquared * (
                                    DIGAMMA_COEF_7)))))))
    )

    return psi


def learnSymmetricConcentration(countHistogram, topicSizeHistogram,
                                numTypes, betaSum):
    currentDigamma = 0.0
    largestNonZeroCount = 0
    nonZeroLengthIndex = zeros(topicSizeHistogram.shape[0])
    largestNonZeroCount = argmax(countHistogram)
    idxs = where(topicSizeHistogram > 0)[0]
    denseIdx = len(idxs)
    nonZeroLengthIndex[idxs] = idxs
    denseIdxSize = denseIdx
    fnumTypes = float64(numTypes)

    for i in range(200):
        currentParameter = betaSum / fnumTypes
        currentDigamma = 0.0
        numerator = 0

        for idx in range(largestNonZeroCount):
            currentDigamma += 1.0 / (currentParameter + float64(idx - 1))
            numerator += countHistogram[idx] * currentDigamma

        currentDigamma = 0.0
        denominator = 0.0
        previousLength = 0
        cachedDigamma = digamma(currentDigamma)

        for denseIdx in range(denseIdxSize):
            length = nonZeroLengthIndex[denseIdx]
            if length - previousLength > 20:
                # FIXME
                currentDigamma = digamma(betaSum + length) - cachedDigamma
            else:
                for idx in range(previousLength, length):
                    currentDigamma += 1.0 / (betaSum + float64(idx))

            denominator += currentDigamma * topicSizeHistogram[length]

        betaSum = currentParameter * numerator / denominator

    return betaSum


def optimizeAlpha(topicDocCounts, docLengthCounts, maxTokens,
                  numTopics, alphaSum, usingSymmetricAlpha=False):

    locDocLengthCounts = zeros(maxTokens, dtype=int64)
    locTopicDocCounts = zeros((numTopics, maxTokens+1), dtype=int64)

    for i in range(len(topicDocCounts)):

        sourceLengthCounts = topicDocCounts[i]
        sourceTopicCounts = topicDocCounts[i]
        locDocLengthCounts[:] += sourceLengthCounts[:]
        sourceLengthCounts[:] = 0

        for t in range(numTopics):
            if usingSymmetricAlpha:
                locTopicDocCounts[:, :] += sourceTopicCounts[:, :]
                # sourceTopicCounts[:, :] = 0
            else:
                locTopicDocCounts[0, :] += sum(sourceTopicCounts[:, :], axis=0)
                # sourceTopicCounts[:, :] = 0

            sourceTopicCounts[:, :] = 0

    if usingSymmetricAlpha:
        alphaSum = learnSymmetricConcentration(
            locTopicDocCounts[0, :], locDocLengthCounts,
            numTopics, alphaSum
        )
        alpha[:] = alphaSum / float64(numTopics)
    else:
        alphaSum = learnParameters(
            alpha, locTopicDocCounts,
            locDocLengthCounts, 1.001, 1.0, 1
        )

    return alphaSum


def resetBeta(beta, betaSum):
    pass


def optimizeBeta(typeTopicCounts, tokensPerTopic,
                 numTypes, numTopics, betaSum, maxTypeCount):
    countHistogram = zeros(maxTypeCount+1, dtype=int64)
    idx = 0
    for t in range(numTypes):
        while idx < typeTopicCounts[t].shape[0] and typeTopicCounts[t] > 0:
            countHistogram[typeTopicCounts[t]] += 1
            idx += 1

    maxTopicSize = tokensPerTopic[argmax(tokensPerTopic)]
    topicSizeHistogram = zeros(maxTopicSize+1, dtype=int64)
    for t in range(numTopics):
        topicSizeHistogram[tokensPerTopic[t]] += 1

    betaSum = learnSymmetricConcentration(
        countHistogram, topicSizeHistogram,
        numTypes, betaSum
    )

    beta = betaSum / float64(numTypes)
    resetBeta(beta, betaSum)
    return betaSum


def estimate(training, numTopics, alpha_, beta, numIterations=2000):
    numTypes = training.shape[1]
    # alphabet = range(numTypes)
    alpha = zeros(numTopics)
    betaSum = beta * numTypes
    typeTopicCounts = zeros((numTypes, numTopics))
    # typeTotals = zeros(numTypes)
    tokensPerTopic = zeros(numTopics)

    docLength = training.sum(1)
    maxTermFreq = docLength.max()
    # totalTokens = sum(training)

    training_indices = ma.masked_where(training > 0, training)

    topicSeq = list([
        randint(0, numTopics, sum(training[i, training_indices[i, :].mask]))
        for i in range(training_indices.shape[0])
    ])

    # n_training_indices = len(topicSeq)

    for i in range(training_indices.shape[0]):
        typ_idx = where(training_indices[i, :].mask == 1)[0]
        count_idx = training[i, training_indices[i, :].mask]
        j = 0
        for typ, count in zip(typ_idx, count_idx):
            for topc in topicSeq[i][j:j+count]:
                typeTopicCounts[typ, topc] += 1
                tokensPerTopic[topc] += 1
            j += count

    docLengthCounts = zeros(maxTermFreq+1)
    topicDocCounts = zeros((numTopics, maxTermFreq+1))
    cachedCoefficients = zeros(numTopics)
    shouldSaveState = False

    UNASSIGNED_TOPIC = -1
    i = 0
    for itr in range(numIterations):
        denom = tokensPerTopic + betaSum
        smoothingOnlyMass = sum(alpha * beta / denom)
        cachedCoefficients = alpha / denom

        for j in range(training.shape[0]):
            localTopicCounts = zeros(numTopics)
            localTopicIndex = zeros(numTopics, dtype=np.int32)
            # (old_topic, new_topic, topicWeightSum) = (-1, -1, 0.0)
            (old_topic, new_topic) = (-1, -1)
            # docLen = docLength[j]

            oneDocTopics = topicSeq[i]
            oneDocTypes = where(training_indices[i, :].mask == 1)[0]
            oneDocTypesFreq = training[i, training_indices[i, :].mask]

            for t in oneDocTopics[where(oneDocTopics != UNASSIGNED_TOPIC)[0]]:
                localTopicCounts[t] += 1

            msk = where(localTopicCounts > 0)[0]
            localTopicIndex[msk] = msk
            nonZeroTopics = len(localTopicIndex)-1
            topicBetaMass = 0.0
            denom = tokensPerTopic[localTopicIndex] + betaSum
            topicBetaMass += sum(
                (beta + localTopicCounts[localTopicIndex]) / denom
            )
            cachedCoefficients[localTopicIndex] = (
                alpha[localTopicIndex] + localTopicCounts[localTopicIndex]
            ) / denom

            topicTermMass = 0.0
            topicTermScores = zeros(numTopics)
            denseIdx = 0
            score = 0.0

            oneDocIndices = zeros(oneDocTypesFreq.shape[0]+1, dtype=np.int64)
            oneDocIndices[1:] = cumsum(oneDocTypesFreq)

            for pos in range(oneDocIndices.shape[0]-1):
                pos_rng = oneDocIndices[pos:pos+2]
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
                        denom = tokensPerTopic[old_topic] + betaSum
                        smoothingOnlyMass -= (alpha[old_topic] + beta) / denom
                        topicBetaMass -= (
                            beta * localTopicCounts[old_topic]
                        ) / denom
                        localTopicCounts[old_topic] -= 1

                        if localTopicCounts[old_topic] == 0:
                            denseIdx = 0
                            denseIdx = len(list(zip(
                                *where(localTopicIndex != old_topic)))
                            )
                            while denseIdx < nonZeroTopics:
                                if denseIdx < len(localTopicIndex):
                                    localTopicIndex[denseIdx] = (
                                        localTopicIndex[denseIdx + 1]
                                    )
                                denseIdx += 1
                            nonZeroTopics -= 1

                        tokensPerTopic[old_topic] -= 1
                        denom = tokensPerTopic[old_topic] + betaSum
                        smoothingOnlyMass += alpha[old_topic] * beta / denom
                        topicBetaMass += (
                            beta * localTopicCounts[old_topic]
                        ) / denom
                        cachedCoefficients[old_topic] = (
                            alpha[old_topic] + localTopicCounts[old_topic]
                        ) / denom

                    alreadyDecremented = (old_topic == UNASSIGNED_TOPIC)
                    topicTermMass = 0.0

                    idx_ = 0
                    while idx_ < currentTypeTopicCounts.shape[0] and (
                        currentTypeTopicCounts[idx_] > 0
                    ):
                        currentTopic = idx_
                        currentValue = currentTypeTopicCounts[idx_]
                        if not alreadyDecremented and (
                            currentTopic == old_topic
                        ):
                            currentValue -= 1
                            if currentValue == 0:
                                currentTypeTopicCounts[idx_] = 0
                            else:
                                currentTypeTopicCounts[old_topic] = (
                                    currentValue
                                )

                            subidx = idx_
                            tmp = 0
                            while subidx < len(currentTypeTopicCounts)-1 and (
                                currentTypeTopicCounts[subidx] <
                                currentTypeTopicCounts[subidx + 1]
                            ):
                                tmp = currentTypeTopicCounts[subidx]
                                currentTypeTopicCounts[subidx] = (
                                    currentTypeTopicCounts[subidx + 1]
                                )
                                currentTypeTopicCounts[subidx + 1] = tmp
                                subidx += 1

                            alreadyDecremented = True
                        else:
                            score = (
                                cachedCoefficients[currentTopic] * currentValue
                            )
                            topicTermMass += score
                            topicTermScores[idx_] = score
                            idx_ += 1

                    sample = rand() * (
                        smoothingOnlyMass + topicBetaMass + topicTermMass
                    )
                    # origSample = sample
                    new_topic = -1
                    if sample < topicTermMass:
                        i = -1
                        while sample > 0:
                            i += 1
                            sample -= topicTermScores[i]
                        new_topic = i
                        currentValue = currentTypeTopicCounts[i]
                        as_idx = argsort(currentTypeTopicCounts)
                        currentTypeTopicCounts[:] = (
                            currentTypeTopicCounts[as_idx]
                        )
                    else:
                        sample -= topicTermMass
                        if sample < topicBetaMass:
                            sample /= beta
                            denseIdx = 0
                            while denseIdx < nonZeroTopics:
                                tpc = localTopicIndex[denseIdx]
                                sample -= (
                                    localTopicCounts[tpc]
                                ) / (tokensPerTopic[tpc] + betaSum)
                                if sample <= 0.0:
                                    new_topic = tpc
                                    break
                                denseIdx += 1
                        else:
                            sample -= topicBetaMass
                            sample /= beta
                            new_topic = 0
                            sample -= (
                                alpha[new_topic]
                            ) / (tokensPerTopic[new_topic] + betaSum)

                            while sample > 0.0 and new_topic+1 < numTopics:
                                new_topic += 1
                                sample -= (
                                    alpha[new_topic]
                                ) / (tokensPerTopic[new_topic] + betaSum)
                        idx_ = 0
                        while idx_ < len(currentTypeTopicCounts)-1 and (
                            currentTypeTopicCounts[idx_] > 0
                        ) and currentTypeTopicCounts[idx_] != new_topic:
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
                    smoothingOnlyMass -= alpha[new_topic] + beta / denom
                    topicBetaMass -= beta * localTopicCounts[new_topic] / denom
                    localTopicCounts[new_topic] += 1

                    if localTopicCounts[new_topic] == 1:
                        denseIdx = abs(nonZeroTopics) % new_topic
                        while denseIdx > 0 and (
                            localTopicIndex[denseIdx-1] > new_topic
                        ):
                            localTopicIndex[denseIdx] = (
                                localTopicIndex[denseIdx-1]
                            )
                            denseIdx -= 1

                        localTopicIndex[denseIdx] = new_topic
                        nonZeroTopics += 1

                    tokensPerTopic[new_topic] += 1
                    denom = (tokensPerTopic[new_topic] + betaSum)
                    cachedCoefficients[new_topic] = (
                        alpha[new_topic] + localTopicCounts[new_topic]
                    ) / denom
                    smoothingOnlyMass += (
                        alpha[new_topic] + beta
                    ) / denom
                    topicBetaMass += (
                        beta * localTopicCounts[new_topic]
                    ) / denom

            if shouldSaveState:
                docLengthCounts[docLength] += 1
                for denseIdx in range(nonZeroTopics):
                    topic = localTopicIndex[denseIdx]
                    topicDocCounts[topic, localTopicCounts[topic]] += 1

            for denseIdx in range(nonZeroTopics):
                topic = localTopicIndex[denseIdx]
                cachedCoefficients[topic] = alpha[topic] / (
                    tokensPerTopic[topic] + betaSum
                )

    return (docLengthCounts, topicDocCounts)

if __name__ == "__main__":
    matlab_vars = dict()
    loadmat('../datasets/docword.kos.train.mat', matlab_vars)
    D = int(matlab_vars['D'][0][0])
    W = int(matlab_vars['W'][0][0])
    N = int(matlab_vars['N'][0][0])
    T = int(4)

    w = matlab_vars['w']
    d = matlab_vars['d']

    w = w.reshape(w.shape[0])
    d = d.reshape(d.shape[0])

    w -= 1
    d -= 1

    z = np.random.randint(0, T, size=T*N)
    beta = 0.01
    alpha = 0.5
    iters = 1000

    wp_1 = np.zeros((W, T))
    wp_2 = np.zeros((W, T))

    dp_1 = np.zeros((W, T))
    dp_2 = np.zeros((W, T))

    dw_1 = np.zeros((W, T))
    dw_2 = np.zeros((W, T))

    word_doc_mat = np.zeros((D, W), dtype=np.int)
    for j in range(N):
        word_doc_mat[d[j], w[j]] += 1

    doclenhist, topicdochist = estimate(word_doc_mat, T, alpha, beta, iters)
