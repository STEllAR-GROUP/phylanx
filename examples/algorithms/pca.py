#  Copyright (c) 2018 Steven R. Brandt
#  Copyright (c) 2018 Christopher Taylor
#  Copyright (c) 2018 R. Tohid
#
#  Distributed under the Boost Software License, Version 1.0. (See accompanying
#  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
#  Implementation as presented here:
#
#  https://glowingpython.blogspot.com/2011/07/principal-component-analysis-with-numpy.html
#  
#  Additional implementation:
#
#  https://glowingpython.blogspot.com/2011/07/pca-and-image-compression-with-numpy.html
#
#  Special thanks to John Tukey for providing explanations of this algorithm
#  and an implementation available in the public domain
#
import phylanx
from phylanx.ast import *
from phylanx.ast.utils import printout 

@Phylanx
def pca(A):
    M = transpose(A - mean( transpose(A), axis=1))
    # TODO: cov, eig primitives
    latent, coeff = eig(cov(M))
    score = dot(transpose(coeff), M)
    return coeff, score, latent

if __name__ == "__main__":

    A = array([ [2.4,0.7,2.9,2.2,3.0,2.7,1.6,1.1,1.6,0.9],
                [2.5,0.5,2.2,1.9,3.1,2.3,2,1,1.5,1.1] ])

    coeff, score, latent = pca(transpose(A))
    printout(coeff, score, latent)

    res = princomp(transpose(A))
    printout(res)
