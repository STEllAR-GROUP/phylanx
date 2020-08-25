//  Copyright (c) 2020 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once
#ifndef __PHYLANX_LDA_TRAINER_IMPL_HPP__
#define __PHYLANX_LDA_TRAINER_IMPL_HPP__

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/annotation.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/execution_tree/locality_annotation.hpp>
#include <phylanx/execution_tree/meta_annotation.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/common/dot_operation_nd.hpp>
#include <phylanx/plugins/dist_matrixops/dist_cannon_product.hpp>
#include <phylanx/util/distributed_matrix.hpp>

#include <hpx/assert.hpp>
#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <iostream>

#include <blaze/Blaze.h>

/////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

class lda_trainer_impl {

    private:

    double alpha, beta;

    public:

    lda_trainer_impl(const double alpha_=0.1, const double beta_=0.01) :
        alpha(alpha_), beta(beta_) {
    }

    static void gibbs(
        const blaze::DynamicMatrix<double> & word_doc_mat,
        const double alpha,
        const double beta,
        blaze::DynamicVector<std::int64_t> & z,
        blaze::DynamicMatrix<double> & wp0,
        blaze::DynamicMatrix<double> & dp,
        blaze::DynamicVector<double, blaze::rowVector> & ztot);

    using dmatrix_t = blaze::DynamicMatrix<double>;
    using dvector_t = blaze::DynamicVector<double>;
    using i64vector_t = blaze::DynamicVector<std::int64_t>;

    std::tuple<dmatrix_t, dmatrix_t> operator()(
        const dmatrix_t & word_doc_mat,
        const std::int64_t T,
        const std::int64_t iter=500);
};

} } } // end namespaces

#endif
