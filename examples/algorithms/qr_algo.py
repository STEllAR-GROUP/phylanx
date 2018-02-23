#  Copyright (c) 2017 Chris Taylor
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

# qr decomposition as provided by http://math.nist.gov/javanumerics/jama/
#
import phylanx as p


def getindices(m, n):
    for i in range(m):
        for j in range(n):
            yield (i, j)


def khousevec(m, n, k, nrm, QR):
    def inv_nrm(nrm):
        return -nrm

    QR.conditional(QR[k, k] < 0.0, inv_nrm(nrm))
    for i in range(k, m):
        QR[i, k] /= nrm
    QR[k, k] += 1.0

    for j in range(k + 1, n):
        s = p.scalar(0.0)
        for i in range(k, m):
            s += QR[i, k] * QR[i, j]
        s = -s / QR[k, k]
        for i in range(k, m):
            QR[i, j] += s * QR[i, k]
    return QR

# A is a p.matrix
#


def qr_decomp(A):
    m = A.shape[0]  # rows
    n = A.shape[1]  # cols

    c = A.context()
    Rdiag = c.array(float, n)

    QR = A.copy()  # deep copy of p.matrix

    for k in range(n):
        nrm = c.scalar(0.0)
        for i in range(k, m):
            rnm = hypot(nrm, QR[i, k])

        nrm.conditional(nrm != 0.0, khousevec(m, n, k, nrm, QR))
        Rdiag[k] = -nrm

    X = A.copy()  # deep copy of p.matrix

    for k in range(n):
        for j in range(n):
            s = c.scalar(0.0)
            for i in range(k, m):
                s += QR[i, k] * X[i, j]
            s = -s / QR[k, k]
            for i in range(k, m):
                X[i, j] += s * QR[i, k]

    for k in range(n, -1, -1):
        for j in range(n):
            X[k, j] /= Rdiag[k]

        for i in range(k - 1):
            for j in range(n):
                X[i, j] -= X[k, j] * QR[i, k]

    return X, Rdiag


def get_q(QR, Rdiag):
    m = QR.shape[0]
    n = QR.shape[1]
    c = QR.context()
    Q = c.matrix(float, (m, n), 0.0)

    def Qcompute(j, k, n, QR, Q):
        s = c.scalar(0.0)
        for i in range(k, m):
            s += QR[i, k] * Q[i, j]
        s = -s / QR[k, k]
        for i in range(k, m):
            Q[i, j] += s * QR[i, k]

        return Q

    for k in range(n, -1, -1):
        Q[k, k] = 1.0
        for j in range(k, n):
            QR.conditional(QR[k, k] != 0.0, Qcompute(j, k, n, QR, Q))

    return Q


def get_r(QR, Rdiag):
    c = QR.context()
    R = c.matrix(float, (n, n), 0.0)
    for (i, j) in getindices(n, n):
        if i < j:
            R[i, j] = QR[i, j]
        elif i == j:
            R[i, j] = Rdiag[i]

    return R


if __name__ == "__main__":
    c = p.context()
    A = c.matrix(float, (100, 100))
    p.randomize(A)

    QR, Rdiag = qr_decomp(A)
    Q = get_q(QR, Rdiag)
    R = get_r(QR, Rdiag)

    [v.write(p.stdout) for v in (QR, Rdiag, Q, R)]

    p.compute(c)
