//  Copyright (c) 2020 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "phylanx/plugins/algorithms/lda_trainer.hpp"
#include <cstdint>

/////////////////////////////////////////////////////////////////////
REGISTER_DISTRIBUTED_MATRIX_DECLARATION(double);

/////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {

// https://bitbucket.org/blaze-lib/blaze/issues/158/random-number-distributions
//
template< typename Rand    // Random number generator
        , typename Dist >  // Random distribution
decltype(auto) random( size_t size, Rand& rand, Dist& dist ) {
  return blaze::generate( size, [&rand,&dist]( size_t ){
     return dist( rand );
  } );
}

void lda_trainer_impl::gibbs(
    const blaze::DynamicMatrix<double> & word_doc_mat,
    const double alpha,
    const double beta,
    blaze::DynamicVector<std::int64_t> & z,
    blaze::DynamicMatrix<double> & wp0,
    blaze::DynamicMatrix<double> & dp,
    blaze::DynamicVector<double, blaze::rowVector> & ztot) {

    const std::int64_t D = word_doc_mat.rows();
    const std::int64_t W = word_doc_mat.columns();
    const std::int64_t T = z.size();

    const double wbeta = static_cast<double>(W) * beta;
    blaze::DynamicMatrix<double> wp(W, T);
    wp = wp0;

    blaze::DynamicVector<double> lhs(T), rhs(T), zprob(T), probs(T);

    using row_iterator =
        blaze::DynamicMatrix<double, blaze::rowMajor>::Iterator;

    using vec_iterator =
        blaze::DynamicVector<double>::Iterator;

    // N (total instance count of words) == sum(word_doc_mat)
    std::int64_t n = 0;

    for(std::int64_t d = 0; d < D; ++d) {
        for(std::int64_t w = 0; w < W; ++w) {

            const auto wdf =
                static_cast<std::int64_t>(word_doc_mat(d, w));

            if(wdf < 1) { continue; }

            for(std::int64_t f = 0; f < wdf; ++f) {

                std::int64_t t = z[n];
                --ztot[t];
                --wp(w, t);
                --dp(d, t);
 
                row_iterator wp_beg = wp.begin(w);
                row_iterator wp_end = wp.end(w);

                std::transform(wp_beg, wp_end, lhs.begin(),
                    [&beta](const auto& val) {
                        return val + beta;
                });

                row_iterator dp_beg = dp.begin(d);
                row_iterator dp_end = dp.end(d);

                std::transform(dp_beg, dp_end, rhs.begin(),
                    [&alpha](const auto& val) {
                        return val + alpha;
                });

                vec_iterator ztot_beg = ztot.begin();
                vec_iterator ztot_end = ztot.end();

                std::transform(ztot_beg, ztot_end, zprob.begin(),
                    [&wbeta](const auto& val) {
                        return val + wbeta;
                });

                probs = (lhs * ( rhs / zprob ));

                double max_prob = std::abs(drand48()) *
                    (*std::max_element(probs.begin(), probs.end())) * 2.0;

                t = static_cast<std::int64_t>(std::abs(drand48()) * T);

                while(max_prob > 1e-10) {
                    max_prob -= probs[t];
                    t = ((t+1) % T);
                }

                z[n] = t;
                ++ztot[t];
                ++wp(w, t);
                ++dp(d, t);
                ++n;
            }
        }
    }
}

using dmatrix_t = blaze::DynamicMatrix<double>;
using dvector_t = blaze::DynamicVector<double>;
using i64vector_t = blaze::DynamicVector<std::int64_t>;

std::tuple<dmatrix_t, dmatrix_t> lda_trainer_impl::operator()(
    const dmatrix_t & word_doc_mat,
    const std::int64_t T,
    const std::int64_t iter) {

    const std::int64_t D = word_doc_mat.rows();
    const std::int64_t W = word_doc_mat.columns();
    const std::int64_t N =
        static_cast<std::int64_t>(blaze::sum(word_doc_mat));

    dmatrix_t wp(W, T); // this component stores information that
                            // contains a distributed reduce and scatter
                            //
    dmatrix_t dp(D, T); // this component is distributed, but unchanging
    i64vector_t z(N);

    {
        // values near the mean are the most likely
        // standard deviation affects the dispersion of generated values from the mean
        {
            std::random_device rd{};
            std::default_random_engine eng{rd()};
            std::uniform_int_distribution<> dist(0, T-1);
            z = random(N, eng, dist);
        }

        blaze::DynamicVector<double, blaze::columnwise> word_index_pos;
        blaze::DynamicVector<double, blaze::rowwise> doc_index_pos;

        word_index_pos = blaze::sum<blaze::rowwise>(word_doc_mat);
        doc_index_pos = blaze::sum<blaze::columnwise>(word_doc_mat);

        // word and doc topic updates are parallel blocks
        {
            std::int64_t k = 0;
            for(std::int64_t i = 0; i < W; ++i) {
                for(std::int64_t j = 0; j < word_index_pos[i]; ++j) {
                    wp(i, z[k]) += 1.0;
                    ++k;
                }
            }
        }

        {
            std::int64_t k = 0;
            for(std::int64_t i = 0; i < D; ++i) {
                for(std::int64_t j = 0; j < doc_index_pos[i]; ++j) {
                    dp(i, z[k]) += 1.0;
                    ++k;
                }
            }
        }
    }

    blaze::DynamicMatrix<double> wp0(W, T);
    blaze::DynamicVector<double, blaze::rowVector> ztot0; //(T);

    for(std::int64_t i = 0; i < iter; ++i) {
        wp0 = wp;
        ztot0 = blaze::eval( blaze::sum<blaze::columnwise>(wp0) );
        // parallel portion
        //
        // z, wp, dp
        //
        gibbs(word_doc_mat, alpha, beta, z, wp, dp, ztot0);
        wp = wp0 + (wp - wp0);
    }

    return std::make_tuple(wp, dp);
}

} } } // end namespaces
