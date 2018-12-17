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
#include <boost/variant/get.hpp>
#include <boost/variant/recursive_variant.hpp>
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
#include <unordered_map>
#include <cmath>

#include <blaze/Math.h>

#include <phylanx/util/variant.hpp>
#include <phylanx/ir/dictionary.hpp>


///////////////////////////////////////////////////////////////////////////////

/*
  life savers...

   https://www.boost.org/doc/libs/1_68_0/libs/spirit/doc/html/spirit/qi/tutorials/mini_xml___asts_.html
   https://www.boost.org/doc/libs/1_68_0/libs/spirit/example/qi/mini_xml1.cpp
*/

namespace phylanx { namespace algorithms { namespace impl {

struct nil {};
struct randomforest_node;

using randomforest_node_variant = phylanx::util::variant<
    nil
    , phylanx::util::recursive_wrapper<randomforest_node>
    , std::int64_t
    , double
    , std::vector< std::int64_t >
>;

using randomforest_node_map = std::map<
    std::string, randomforest_node_variant
>;

struct randomforest_node {
    randomforest_node_map fields;
};

}}} // end namespace

BOOST_FUSION_ADAPT_STRUCT(
    phylanx::algorithms::impl::randomforest_node,
    (phylanx::algorithms::impl::randomforest_node_map, fields)
)

namespace phylanx { namespace algorithms { namespace impl {

struct randomforest_impl {

    std::vector<phylanx::algorithms::impl::randomforest_node> trees;
    std::unordered_map<double, std::int64_t> classes;

    randomforest_impl(std::size_t const ntrees=10ul)
        : trees(ntrees)
        , classes() {
    }

    void grow(std::size_t const ntrees=10ul) {
        if(ntrees < 1ul) { return; }
        trees.resize(ntrees);
    }

    static void test_split(
            std::int64_t const feature
            , double const val
            , blaze::DynamicMatrix<double> const& dataset
            , std::vector<std::int64_t> & left
            , std::vector<std::int64_t> & right) {

        auto const rows = dataset.rows();

        for(auto rowidx = 0ul; rowidx < rows; ++rowidx) {
            if(dataset(rowidx, feature) < val) {
                left.push_back(rowidx);
            }
            else {
                right.push_back(rowidx);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    static double gini_index(
        blaze::DynamicMatrix<double> const& dataset
        , blaze::DynamicVector<double> const& dataset_labels
        , std::vector<std::int64_t> const& left_group
        , std::vector<std::int64_t> const& right_group
        , std::unordered_map<double
            , std::int64_t> & classes) {

        std::vector<std::int64_t> groups_len{
            static_cast<std::int64_t>(left_group.size())
            , static_cast<std::int64_t>(right_group.size())
        };

        std::vector< std::vector<std::int64_t> > groups_vec{
            left_group
            , right_group
        };

        double const n_instances = static_cast<double>(
            std::accumulate(groups_len.begin()
                , groups_len.end()
                , 0ul)
        );

        double gini = 0.0;

        blaze::DynamicVector<double> p(classes.size());

        for(auto i = 0ul; i < groups_len.size(); ++i) {
            if(groups_len[i] < 1ul) { continue; }
            double const& group_len = static_cast<double>(
                groups_len[i]
            );
            auto const& group = groups_vec[i];

            for(auto const& row_idx : group) {
                p[classes[dataset_labels[row_idx]]] += 1.0;
            }

            double const score = std::accumulate(p.begin(), p.end(), 0.0
                , [&group_len](double const pval, double const qval) {
                    return std::pow(pval/group_len, 2.0) + std::pow(qval/group_len, 2.0);
                }
            );

            gini += (1.0 - score) * static_cast<double>(group_len / n_instances);
            std::transform(p.begin()
                , p.end()
                , p.begin()
                , [](auto const i) { return 0.0; }
            );
        }

        return gini;
    }

    ///////////////////////////////////////////////////////////////////////////
    template <typename T, typename F>
    static void argsort(std::vector<T> const& v, std::vector<F> & idx ) {
        if(idx.size() < v.size())
            idx.resize(v.size());

        std::iota(idx.begin(), idx.end(), 0ul);

        std::sort(idx.begin(), idx.end()
            , [&v](F const i1, F const i2) {
                return v[i1] < v[i2];
            }
        );
    }

    ///////////////////////////////////////////////////////////////////////////
    static void get_split(
        blaze::DynamicMatrix<double> const& dataset
        , blaze::DynamicVector<double> const& dataset_labels
        , std::vector<std::int64_t> const& dataset_indices
        , std::int64_t const n_features
        , std::unordered_map<double, std::int64_t> & classes
        , randomforest_node & ret) {

        auto b_idx = (std::numeric_limits<std::int64_t>::max)();
        auto b_val = (std::numeric_limits<double>::max)();
        auto b_score = (std::numeric_limits<double>::max)();
        std::vector< std::int64_t > b_lgroups, b_rgroups;

        std::vector<std::int64_t> features(n_features);

        {
            std::default_random_engine generator;
            std::uniform_int_distribution<std::int64_t> distribution(0ul
                , dataset.columns()
            );

            auto gen = [&generator, &distribution]() {
                return distribution(generator);
            };

            std::vector<std::int64_t> idx_w(dataset.columns());
            std::generate_n(idx_w.begin(), idx_w.size(), [&gen]() { return gen(); });
            std::vector<std::int64_t> idx_w_sort;
            argsort(idx_w, idx_w_sort);

            std::vector<std::int64_t> idx(idx_w_sort.size());
            std::iota(idx.begin(), idx.end(), 0ul);

            std::transform(
                idx_w_sort.begin(), idx_w_sort.begin()+n_features
                , features.begin()
                , [&idx](auto const i) {
                    return idx[i];
                }
            );
        }

        std::vector< std::int64_t > lgroups, rgroups;

        for(auto feature : features) {
            for(auto const r : dataset_indices) {
                test_split(feature
                    , dataset(r, feature)
                    , dataset
                    , lgroups
                    , rgroups);

                auto const gini = gini_index(dataset
                    , dataset_labels
                    , lgroups
                    , rgroups
                    , classes);

                if(gini < b_score) {
                    b_idx = feature;
                    b_val = dataset(r, feature);
                    b_score = gini;
                    b_lgroups.assign(lgroups.begin(), lgroups.end());
                    b_rgroups.assign(rgroups.begin(), rgroups.end());
                }
            }
        }

        ret.fields["index"] = b_idx;
        ret.fields["value"] = b_val;
        ret.fields["left_groups"] = b_lgroups;
        ret.fields["right_groups"] = b_rgroups;
        ret.fields["lw"] = (std::numeric_limits<std::int64_t>::max)();
        ret.fields["rw"] = (std::numeric_limits<std::int64_t>::max)();
    }

    ///////////////////////////////////////////////////////////////////////////
    static std::int64_t to_terminal(
        blaze::DynamicVector<double> const& train_labels
        , std::vector<std::int64_t> const& group
        , std::unordered_map<double, std::int64_t> & classes) {

        blaze::DynamicVector<double> outcome_hist(classes.size());

        for(auto g : group) {
            double const k = train_labels[g];
            outcome_hist[classes[k]] += 1.0;
        }

        return std::distance(outcome_hist.begin()
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

        std::vector<std::int64_t> left_group{
            phylanx::util::get< std::vector<std::int64_t> >(node.fields["left_groups"])
        };

        std::vector<std::int64_t> right_group{
            phylanx::util::get< std::vector<std::int64_t> >(node.fields["right_groups"])
        };

        node.fields.erase("left_groups");
        node.fields.erase("right_groups");

        if(left_group.size() == 0ul) {
            auto term = to_terminal(train_labels
                , right_group, classes
            );
            node.fields["lw"] = term;
        }
        else if(right_group.size() == 0ul) {
            auto term = to_terminal(train_labels, left_group, classes);
            node.fields["rw"] = term;
        }

        if(depth >= max_depth) {
            auto lterm = to_terminal(train_labels, left_group, classes);
            auto rterm = to_terminal(train_labels, right_group, classes);
            node.fields["lw"] = lterm;
            node.fields["rw"] = rterm;
            return;
        }

        if(left_group.size() <= min_size) {
             auto lterm = to_terminal(train_labels, left_group, classes);
             node.fields["lw"] = lterm;
        }

        randomforest_node left_node;

        get_split(train
            , train_labels
            , left_group
            , n_features
            , classes
            , left_node);

        node.fields["left"] = left_node;

        split(phylanx::util::get<phylanx::util::recursive_wrapper<randomforest_node>>
            (node.fields["left"]).get()
            , train
            , train_labels
            , max_depth
            , min_size
            , n_features
            , depth + 1
            , classes);

        if(right_group.size() <= min_size) {
            auto rterm = to_terminal(train_labels
                , right_group
                , classes);

            node.fields["rw"] = rterm;
        }

        randomforest_node right_node;

        get_split(train
            , train_labels
            , right_group
            , n_features
            , classes
            , right_node);

        node.fields["right"] = right_node;

        split(phylanx::util::get<phylanx::util::recursive_wrapper<randomforest_node>>
            (node.fields["right"]).get()
            , train
            , train_labels
            , max_depth
            , min_size
            , n_features
            , depth + 1
            , classes);
    }

    ///////////////////////////////////////////////////////////////////////////
    static void build_tree(
        randomforest_node & tree_root
        , blaze::DynamicMatrix<double> const& train
        , blaze::DynamicVector<double> const& train_labels
        , std::vector<std::int64_t> const& sample_indices
        , std::int64_t max_depth
        , std::int64_t min_size
        , std::int64_t n_features
        , std::unordered_map<double, std::int64_t> & classes) {

        get_split(train
            , train_labels
            , sample_indices
            , n_features
            , classes
            , tree_root);

        split(tree_root
            , train
            , train_labels
            , max_depth
            , min_size
            , n_features
            , 1ul
            , classes);
    }

    ///////////////////////////////////////////////////////////////////////////
    static std::int64_t node_predict(
        randomforest_node & node
        , blaze::DynamicMatrix<double> const& r
        , std::int64_t const i) {

        auto index = phylanx::util::get<std::int64_t>(node.fields["index"]);
        auto value = phylanx::util::get<double>(node.fields["value"]);

        if(r(i, index) < value) {
            auto lw = phylanx::util::get<std::int64_t>(node.fields["lw"]);
            if( lw == (std::numeric_limits<std::int64_t>::max)()) {
                auto left = phylanx::util::get<
                    phylanx::util::recursive_wrapper<randomforest_node>
                        >(node.fields["left"]).get();
                return node_predict(left, r, i);
            }
            else {
                return lw;
            }
        }

        auto rw = phylanx::util::get<std::int64_t>(node.fields["rw"]);
        if(rw == (std::numeric_limits<std::int64_t>::max)()) {
            auto right = phylanx::util::get<
                    phylanx::util::recursive_wrapper<randomforest_node>
                        >(node.fields["right"]).get();
            return node_predict(right, r, i);
        }

        return rw;
    }

    ///////////////////////////////////////////////////////////////////////////
    static void subsample(
        blaze::DynamicMatrix<double> const& dataset
        , std::vector<std::int64_t> & idx_w_sort
        , double const ratio) {

        std::int64_t const n_sample =
            static_cast<std::int64_t>(std::floor(static_cast<double>(
                dataset.rows()) * ratio)
        );

        std::default_random_engine generator;
        std::uniform_int_distribution<std::int64_t> distribution(0ul
            , dataset.rows()
        );
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

        auto prediction_indices =
            boost::irange<std::int64_t>(0ul, predictions.size());

        hpx::parallel::for_each(
            hpx::parallel::execution::par
            , std::begin(prediction_indices)
            , std::end(prediction_indices)
            , [&classes, &predictions, &prediction_histograms](auto const i) {
                prediction_histograms[i].resize(classes.size());
                prediction_histograms[i][predictions[i]] += 1ul;
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
        , double const sample_size) {

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

        std::vector< std::vector<std::int64_t> > subsample_indices(trees.size());

        auto tree_indices = boost::irange<std::int64_t>(0, trees.size());

        hpx::parallel::for_each(
            hpx::parallel::execution::par
            , std::begin(tree_indices)
            , std::end(tree_indices)
            , [this, &train, &train_labels
               , &subsample_indices, &max_depth
               , &min_size, &sample_size, &n_features]
                (std::int64_t const idx) {
                    subsample(train, subsample_indices[idx], sample_size);
                    build_tree(trees[idx]
                        , train
                        , train_labels
                        , subsample_indices[idx]
                        , max_depth
                        , min_size
                        , n_features
                        , classes);
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

        auto indices = boost::irange<std::int64_t>(0ul, test.rows());

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
