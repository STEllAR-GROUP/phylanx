//  Copyright (c) 2018 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#if !defined(__PHYLANX_RANDOMFORESTIMPL_HPP__)
#define __PHYLANX_RANDOMFORESTIMPL_HPP__

#include <hpx/throw_exception.hpp>
#include <hpx/parallel/algorithms/for_each.hpp>
#include <hpx/parallel/algorithms/transform.hpp>
#include <hpx/parallel/algorithms/reduce.hpp>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/range/irange.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

#include <blaze/Math.h>
#include <phylanx/phylanx.hpp>
#include <phylanx/util/variant.hpp>
#include <phylanx/execution_tree/primitives.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/ir/node_data.hpp>

///////////////////////////////////////////////////////////////////////////////

/*
  life savers...

   https://www.boost.org/doc/libs/1_68_0/libs/spirit/
       doc/html/spirit/qi/tutorials/mini_xml___asts_.html
   https://www.boost.org/doc/libs/1_68_0/libs/spirit/
       example/qi/mini_xml1.cpp
 */
namespace phylanx { namespace algorithms { namespace impl {

using randomforest_node = phylanx::execution_tree::primitive_argument_type; //phylanx::ir::dictionary;

struct randomforest_impl {

    std::vector<randomforest_node> trees;
    std::int64_t ntrees;
    std::unordered_map<double, std::int64_t> classes;

    explicit randomforest_impl(const std::int64_t n_trees)
        : trees(n_trees)
        , ntrees(n_trees)
        , classes() {
    }

    static void test_split(
            std::int64_t const feature
            , double const val
            , blaze::DynamicMatrix<double> const& dataset
            , std::tuple< blaze::DynamicVector< std::int64_t >
                , blaze::DynamicVector< std::int64_t > > & groups ) {

        std::vector< std::int64_t > left, right;
        auto const rows = dataset.rows();

        for(auto rowidx = 0; rowidx < rows; ++rowidx) {
            if(dataset(rowidx, feature) < val) {
                left.push_back(rowidx);
            }
            else {
                right.push_back(rowidx);
            }
        }

        blaze::DynamicVector< std::int64_t > lvec(left.size()), rvec(right.size());
        std::copy(left.begin(), left.end(), lvec.begin());
        std::copy(right.begin(), right.end(), rvec.begin());

        groups = std::make_tuple(lvec, rvec);
    }

    ///////////////////////////////////////////////////////////////////////////
    static double gini_index(
        blaze::DynamicMatrix<double> const& dataset
        , blaze::DynamicVector<double> const& dataset_labels
        , std::tuple< blaze::DynamicVector<std::int64_t>, blaze::DynamicVector<std::int64_t> > const& groups
        , std::unordered_map<double, std::int64_t> & classes) {

        std::vector<std::int64_t> groups_len{static_cast<std::int64_t>(std::get<0>(groups).size())
            , static_cast<std::int64_t>(std::get<1>(groups).size())};

        std::vector< blaze::DynamicVector<std::int64_t> > groups_vec{std::get<0>(groups)
            , std::get<1>(groups)};

        double const n_instances = static_cast<double>(
            std::accumulate(groups_len.begin(), groups_len.end(), 0UL)
        );
        double gini = 0.0;

        blaze::DynamicVector<double> p(classes.size());

        for(auto i = 0; i < groups_len.size(); ++i) {
            if(groups_len[i] < 1) { continue; }
            double const& group_len = static_cast<double>(groups_len[i]);
            auto const& group = groups_vec[i];

            for(auto const& row_idx : group) {
                p[classes[dataset_labels[row_idx]]] += 1.0;
            }

            double const score = std::accumulate(p.begin(), p.end(), 0.0
                , [&group_len](double const pval, double const qval) {
                    return std::pow(pval/group_len, 2.0) +
                        std::pow(qval/group_len, 2.0);
                }
            );

            gini += (1.0 - score) * static_cast<double>(group_len / n_instances);
            std::transform(p.begin()
                , p.end(), p.begin()
                , [](auto const i) { return 0.0; });
        }

        return gini;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename F>
    static void argsort(T const& v, F & idx ) {
        if(idx.size() < v.size())
            idx.resize(v.size());

        std::iota(idx.begin(), idx.end(), 0);
        std::sort(idx.begin(), idx.end()
            , [&v](auto const i1, auto const i2) {
                return v[i1] < v[i2];
            }
        );
    }

    ///////////////////////////////////////////////////////////////////////////
    static void get_split(
        blaze::DynamicMatrix<double> const& dataset
        , blaze::DynamicVector<double> const& dataset_labels
        , blaze::DynamicVector<std::int64_t> const& dataset_indices
        , std::int64_t const n_features
        , std::unordered_map<double, std::int64_t> & classes
        , randomforest_node & ret) {

        std::int64_t b_idx = std::numeric_limits<std::int64_t>::max();
        double b_val = std::numeric_limits<double>::max();
        double b_score = std::numeric_limits<double>::max();
        //std::tuple< std::vector<std::int64_t>, std::vector<std::int64_t> > b_groups;
        std::tuple< blaze::DynamicVector<std::int64_t>, blaze::DynamicVector<std::int64_t> > b_groups;

        std::vector<std::int64_t> features(n_features);

        {
            std::default_random_engine generator;
            std::uniform_int_distribution<std::int64_t> distribution(0
                , dataset.columns());
            auto gen = [&generator, &distribution]() {
                return distribution(generator);
            };

            std::vector<std::int64_t> idx_w(dataset.columns());
            std::generate_n(idx_w.begin(), idx_w.size()
                , [&gen]() { return gen(); });
            std::vector<std::int64_t> idx_w_sort;
            argsort(idx_w, idx_w_sort);

            std::vector<std::int64_t> idx(idx_w_sort.size());
            std::iota(idx.begin(), idx.end(), 0);

            std::transform(
                idx_w_sort.begin(), idx_w_sort.begin()+n_features
                , features.begin()
                , [&idx](auto const i) {
                    return idx[i];
                }
            );
        }

        std::tuple< blaze::DynamicVector< std::int64_t >
            , blaze::DynamicVector< std::int64_t > > groups;

        for(std::int64_t feature : features) {
            for(std::int64_t r : dataset_indices) {
                test_split(feature, dataset(r, feature), dataset, groups);
                auto const gini = gini_index(dataset
                    , dataset_labels, groups, classes);
                if(gini < b_score) {
                    b_idx = feature;
                    b_val = dataset(r, feature);
                    b_score = gini;
                    b_groups = std::make_tuple( std::get<0>(groups)
                        , std::get<1>(groups) );
                }
            }
        }

        auto& node_ = phylanx::util::get<phylanx::ir::dictionary>(ret);
        node_[phylanx::execution_tree::primitive_argument_type{std::string("index")}] =
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<std::int64_t>(b_idx)
            };
        node_[phylanx::execution_tree::primitive_argument_type{std::string("value")}] =
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(b_val)
            };
        node_[phylanx::execution_tree::primitive_argument_type{std::string("groups_l")}] =
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<std::int64_t>( std::get<0>(b_groups) )
            };
        node_[phylanx::execution_tree::primitive_argument_type{std::string("groups_r")}] =
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<std::int64_t>( std::get<1>(b_groups) )
            };
        node_[phylanx::execution_tree::primitive_argument_type{std::string("lw")}] =
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<std::int64_t>(
                    std::numeric_limits<std::int64_t>::max()
                )
            };
        node_[phylanx::execution_tree::primitive_argument_type{std::string("rw")}] =
            phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<std::int64_t>(
                    std::numeric_limits<std::int64_t>::max()
                )
            };
    }

    ///////////////////////////////////////////////////////////////////////////
    static std::int64_t to_terminal(
        blaze::DynamicVector<double> const& train_labels
        , blaze::DynamicVector<std::int64_t> const& group
        , std::unordered_map<double, std::int64_t> & classes) {

        blaze::DynamicVector<double> outcome_hist(classes.size());

        for(auto g : group) {
            double const k = train_labels[g];
            outcome_hist[classes[k]] += 1.0;
        }

        return (outcome_hist.size() < 1) ? 0 : std::distance(outcome_hist.begin()
            , std::max_element(outcome_hist.begin(), outcome_hist.end())
        );
    }

    ///////////////////////////////////////////////////////////////////////////
    static void split(randomforest_node & node
        , blaze::DynamicMatrix<double> const& train
        , blaze::DynamicVector<double> const& train_labels
        , std::int64_t max_depth
        , std::int64_t min_size
        , std::int64_t n_features
        , std::int64_t depth
        , std::unordered_map<double, std::int64_t> & classes) {

        auto& node_ = phylanx::util::get<phylanx::ir::dictionary>(node);

        blaze::DynamicVector<std::int64_t> left =
            phylanx::util::get<phylanx::ir::node_data<std::int64_t>>(
                node_[phylanx::execution_tree::primitive_argument_type{std::string("groups_l")}].get().variant()
            ).vector();

        blaze::DynamicVector<std::int64_t> right =
            phylanx::util::get<phylanx::ir::node_data<std::int64_t>>(
                node_[phylanx::execution_tree::primitive_argument_type{std::string("groups_r")}].get().variant()
            ).vector();


        node_.erase(phylanx::execution_tree::primitive_argument_type{std::string("groups_l")});
        node_.erase(phylanx::execution_tree::primitive_argument_type{std::string("groups_r")});

        if(left.size() == 0 || right.size() == 0) {
            blaze::DynamicVector<std::int64_t> joint{left};
            auto jsz = joint.size();
            joint.reserve(jsz + right.size());
            std::for_each(right.begin(), right.end(), [&joint, &jsz](auto rval) { joint[jsz] = rval; ++jsz; });
            auto term = to_terminal(train_labels, joint, classes);
            node_[phylanx::execution_tree::primitive_argument_type{std::string("lw")}] =
                phylanx::execution_tree::primitive_argument_type{phylanx::ir::node_data<std::int64_t>{term}};
            node_[phylanx::execution_tree::primitive_argument_type{std::string("rw")}] =
                phylanx::execution_tree::primitive_argument_type{phylanx::ir::node_data<std::int64_t>{term}};
            return;
        }

        if(depth >= max_depth) {
            auto lterm = to_terminal(train_labels, left, classes);
            auto rterm = to_terminal(train_labels, right, classes);
            node_[phylanx::execution_tree::primitive_argument_type{std::string("lw")}] =
                phylanx::execution_tree::primitive_argument_type{phylanx::ir::node_data<std::int64_t>{lterm}};
            node_[phylanx::execution_tree::primitive_argument_type{std::string("rw")}] =
                phylanx::execution_tree::primitive_argument_type{phylanx::ir::node_data<std::int64_t>{rterm}};
            return;
        }

        if(left.size() <= min_size) {
             auto lterm = to_terminal(train_labels, left, classes);
             node_[phylanx::execution_tree::primitive_argument_type{std::string("lw")}] =
                phylanx::execution_tree::primitive_argument_type{phylanx::ir::node_data<std::int64_t>{lterm}};
        }
        else {
            randomforest_node left_node{};
            get_split(train, train_labels, left, n_features, classes, left_node);
            split(left_node, train, train_labels, max_depth, min_size, n_features
                , depth + 1, classes);
            node_[phylanx::execution_tree::primitive_argument_type{std::string("left")}] =
                std::move(left_node);
        }
        
        if(right.size() <= min_size) {
            auto rterm = to_terminal(train_labels, right, classes);
            node_[phylanx::execution_tree::primitive_argument_type{std::string("rw")}] =
                phylanx::execution_tree::primitive_argument_type{phylanx::ir::node_data<std::int64_t>{rterm}};
        }
        else {
            randomforest_node right_node{};
            get_split(train, train_labels, right, n_features, classes, right_node);
            split(right_node, train, train_labels, max_depth, min_size, n_features
                , depth + 1, classes);
            node_[phylanx::execution_tree::primitive_argument_type{std::string("right")}] =
                std::move(right_node);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    static void build_tree(
        randomforest_node & tree_root
        , blaze::DynamicMatrix<double> const& train
        , blaze::DynamicVector<double> const& train_labels
        , blaze::DynamicVector<std::int64_t> const& sample_indices
        , std::int64_t max_depth
        , std::int64_t min_size
        , std::int64_t n_features
        , std::unordered_map<double, std::int64_t> & classes) {

        get_split(train, train_labels, sample_indices, n_features, classes
            , tree_root);
        split(tree_root, train, train_labels, max_depth, min_size, n_features
            , 1, classes);
    }

    ///////////////////////////////////////////////////////////////////////////
    static std::int64_t node_predict(
        randomforest_node & node
        , blaze::DynamicMatrix<double> const& r
        , std::int64_t const i) {

        auto& node_ = phylanx::util::get<phylanx::ir::dictionary>(node);

        std::int64_t index = phylanx::util::get<phylanx::ir::node_data<std::int64_t>>(
            node_[phylanx::execution_tree::primitive_argument_type{std::string("index")}].get().variant()
        ).scalar();

        double value = phylanx::util::get<phylanx::ir::node_data<double>>(
            node_[phylanx::execution_tree::primitive_argument_type{std::string("value")}].get().variant()
        ).scalar();

        if(r(i, index) < value) {
            std::int64_t lw = phylanx::util::get<phylanx::ir::node_data<std::int64_t>>(
                node_[phylanx::execution_tree::primitive_argument_type{std::string("lw")}].get().variant()
            ).scalar();

            if( lw == std::numeric_limits<std::int64_t>::max()) {
                auto& left = //phylanx::util::get<phylanx::ir::dictionary>(
                    node_[phylanx::execution_tree::primitive_argument_type{std::string("left")}].get(); //.variant()
                //);
                return node_predict(left, r, i);
            }
            else {
                return lw;
            }
        }

        auto rw = phylanx::util::get<phylanx::ir::node_data<std::int64_t>>(
            node_[phylanx::execution_tree::primitive_argument_type{std::string("rw")}].get().variant()
        ).scalar();

        if(rw == std::numeric_limits<std::int64_t>::max()) {
            auto& right = //phylanx::util::get<phylanx::ir::dictionary>(
                node_[phylanx::execution_tree::primitive_argument_type{std::string("right")}].get(); //.variant()
            //);
            return node_predict(right, r, i);
        }

        return rw;
    }

    ///////////////////////////////////////////////////////////////////////////
    static void subsample(
        blaze::DynamicMatrix<double> const& dataset
        , blaze::DynamicVector<std::int64_t> & idx_w_sort
        , std::int64_t const ratio) {

        std::default_random_engine generator;
        std::uniform_int_distribution<std::int64_t> distribution(0, dataset.rows());
        auto gen = [&generator, &distribution] {
            return static_cast<std::int64_t>(distribution(generator));
        };

        std::vector<std::int64_t> idx_w(dataset.rows());
        std::generate_n(idx_w.begin(), idx_w.size(), gen);
        argsort(idx_w, idx_w_sort);
    }

    ///////////////////////////////////////////////////////////////////////////
    static std::int64_t bagging_predict(
        std::vector<randomforest_node> & trees
        , blaze::DynamicMatrix<double> const& dataset
        , std::int64_t const row
        , std::unordered_map<double, std::int64_t> & classes) {

        std::vector<std::int64_t> predictions(trees.size());

        hpx::parallel::transform(
            hpx::parallel::execution::par
            , trees.begin()
            , trees.end()
            , predictions.begin()
            , [&dataset, &row](auto & tree) {
                return node_predict(tree, dataset, row);
            }
        );

        std::vector< blaze::DynamicVector<std::int64_t> > prediction_histograms(
            predictions.size()
        );
        auto prediction_indices = boost::irange<std::int64_t>(0UL
            , predictions.size());

        hpx::parallel::for_each(
            hpx::parallel::execution::par
            , std::begin(prediction_indices)
            , std::end(prediction_indices)
            , [&classes, &predictions, &prediction_histograms](auto const i) {
                prediction_histograms[i].resize(classes.size());
                prediction_histograms[i][predictions[i]] += 1UL;
            }
        );

        blaze::DynamicVector<double> fhistogram(classes.size());

        fhistogram = hpx::parallel::reduce(
            hpx::parallel::execution::par
            , prediction_histograms.begin()
            , prediction_histograms.end()
            , fhistogram
            , std::plus<blaze::DynamicVector<double>>()
        );

        auto itr = std::max_element(fhistogram.begin(), fhistogram.end());
        return std::distance(fhistogram.begin(), itr);
    }

    void fit(
        blaze::DynamicMatrix<double> const& train
        , blaze::DynamicVector<double> const& train_labels
        , std::int64_t const max_depth
        , std::int64_t const min_size
        , std::int64_t const sample_size) {

        std::vector<double> labels(train_labels.size());
        std::copy(train_labels.begin(), train_labels.end(), labels.begin());
        std::sort(labels.begin(), labels.end());
        auto last = std::unique(labels.begin(), labels.end());
        labels.erase(last, labels.end());

        std::int64_t i = 0;
        for(auto itr = labels.begin(); itr != labels.end(); ++itr, ++i) {
            classes[(*itr)] = i;
        }

        std::int64_t const n_features = static_cast<std::int64_t>(
            std::floor(std::sqrt(train.rows()))
        );
        
        std::vector< blaze::DynamicVector<std::int64_t> > subsample_indices(ntrees);
        auto tree_indices = boost::irange<std::int64_t>(0, ntrees);

        std::for_each(std::begin(tree_indices)
            , std::end(tree_indices)
            , [this](std::int64_t const idx) {
               trees[idx] =
                    std::move(phylanx::execution_tree::primitive_argument_type{
                        phylanx::ir::dictionary()
                    });
             });

        hpx::parallel::for_each(
            hpx::parallel::execution::par
            , std::begin(tree_indices)
            , std::end(tree_indices)
            , [this, &train, &train_labels, &subsample_indices, &max_depth, &min_size
                , &sample_size, &n_features]
                (std::int64_t const idx) {
                    subsample(train, subsample_indices[idx], sample_size);
                    build_tree( trees[idx]
                        , train, train_labels, subsample_indices[idx]
                        , max_depth, min_size, n_features, classes);
            }
        );
    }

    void predict(
        blaze::DynamicMatrix<double> const& test
        , blaze::DynamicVector<double> & y) {

        if(y.size() < 1UL) {
            y.resize(test.rows());
        }

        blaze::DynamicVector<double> labels(classes.size());
        for(auto & cls : classes) {
            labels[cls.second] = cls.first;
        }

        auto indices = boost::irange<std::int64_t>(0, test.rows());

        hpx::parallel::for_each(
            hpx::parallel::execution::par
            , std::begin(indices)
            , std::end(indices)
            , [this, &test, &y, &labels](auto const& idx) {
                y[idx] = labels[bagging_predict(trees, test, idx, classes)];
            }
        );
    }

};

}}} // end namespace

#endif
