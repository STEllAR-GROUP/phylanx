//  Copyright (c) 2018 Hartmut Kaiser
//  Copyright (c) 2018 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <phylanx/config.hpp>
#include <phylanx/util/variant.hpp>
#include <phylanx/util/generate_error_message.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/plugins/algorithms/randomforest.hpp>
#include <phylanx/plugins/algorithms/impl/randomforest.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <boost/range/irange.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

using namespace phylanx::algorithms::impl;

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    static void transform(std::string const& key
        , nil & none
        , phylanx::ir::dictionary & phy) {
        HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::transform"
            , phylanx::util::generate_error_message(
            "nil element discovered in tree"));
    }

    static void transform(std::string const& key
        , double weight
        , phylanx::ir::dictionary & phy) {

        auto dkey = phylanx::execution_tree::primitive_argument_type{
            std::string(key)};

        auto value = phylanx::execution_tree::primitive_argument_type{
            phylanx::ir::node_data<double>(weight)};

        phy[dkey] = value;
    }

    static void transform(std::string const& key
        , std::int64_t term
        , phylanx::ir::dictionary & phy) {

        auto dkey = phylanx::execution_tree::primitive_argument_type{
            std::string(key)};

        auto value = phylanx::execution_tree::primitive_argument_type{
            phylanx::ir::node_data<std::int64_t>(term)};

        phy[dkey] = value;
    }

    static void transform(std::string const& key
        , std::vector< std::int64_t > & group
        , phylanx::ir::dictionary & phy) {

        auto dkey = phylanx::execution_tree::primitive_argument_type{
            std::string(key)};

        std::vector<phylanx::execution_tree::primitive_argument_type>
            group_indices(group.size());

        for(auto const g : group) {
            group_indices.push_back(
                phylanx::execution_tree::primitive_argument_type{
                    std::int64_t(g)} );
        }

        phylanx::ir::range rng(group_indices.begin(), group_indices.end());
        auto value = phylanx::execution_tree::primitive_argument_type{rng};

        phy[dkey] = value;
    }

    static void transform(std::string const& key
        , phylanx::algorithms::impl::randomforest_node & node
        , phylanx::ir::dictionary & phy) {

        for(auto field : node.fields) {
            switch(field.second.index()) {

               case 0: // nil
                   transform(field.first
                       , phylanx::util::get<phylanx::algorithms::impl::nil>(
                           field.second), phy);
                   break;

               case 1: // randomforest_node
                   {
                       phylanx::ir::dictionary child;

                       transform(field.first
                           , phylanx::util::get<phylanx::util::recursive_wrapper<
                                   phylanx::algorithms::impl::randomforest_node> >(
                                       field.second).get(), child);

                       auto dkey = phylanx::execution_tree::primitive_argument_type{
                           std::string(key)};

                       auto value = phylanx::execution_tree::primitive_argument_type{
                           child};

                       phy[dkey] = value;
                   }

                   break;

               case 2: // std::int64_t
                   transform(field.first, phylanx::util::get<std::int64_t>(
                       field.second), phy);
                   break;

               case 3: // double
                   transform(field.first, phylanx::util::get<double>(
                       field.second), phy);
                   break;

               case 4: // std::vector<std::int64_t>
                   transform(field.first, phylanx::util::get<std::vector<std::int64_t>>(
                       field.second), phy);
                   break;

               default:
                   HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::transform"
                       , phylanx::util::generate_error_message(
                           "unexpected element discovered in tree"));
            }
        }

    }

    static void transform(randomforest_impl & forest
        , phylanx::ir::dictionary &phy) {

        std::size_t const forest_size = forest.trees.size();
        auto forest_index_range = boost::irange(0ul, forest_size);
        phy.reserve(forest_size);

        std::vector<std::string> phy_keys(forest_size);
        std::string const root_key_prefix("root_node_");
        std::transform(
            forest_index_range.begin()
            , forest_index_range.end()
            , phy_keys.begin()
            , [&root_key_prefix](auto const i) {
                return root_key_prefix + std::to_string(i);
        });

        std::vector<phylanx::ir::dictionary> phy_trees(forest_size);
        for(auto const forest_index : forest_index_range) {
            transform(phy_keys[forest_index]
                , forest.trees[forest_index]
                , phy_trees[forest_index]);

            auto tree_key = phylanx::execution_tree::primitive_argument_type{
                std::string(phy_keys[forest_index])};

            phy[tree_key] = phy_trees[forest_index];
        }

        phylanx::ir::dictionary cls_dictionary;

        for(auto & cls : forest.classes) {
            auto dkey = phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<double>(cls.first)};
            auto dval = phylanx::execution_tree::primitive_argument_type{
                phylanx::ir::node_data<std::int64_t>(cls.second)};
            cls_dictionary[dkey] = dval;
        }

        auto cls_key = phylanx::execution_tree::primitive_argument_type{
            std::string("classes")};

        phy[cls_key] = cls_dictionary;
    }

    ///////////////////////////////////////////////////////////////////////////
    static void transform(std::string const& key
        , phylanx::ir::node_data<double> weight
        , phylanx::algorithms::impl::randomforest_node & node) {
        node.fields[key] = weight.scalar();
    }

    static void transform(std::string const& key
        , phylanx::ir::node_data<std::int64_t> term
        , phylanx::algorithms::impl::randomforest_node & node) {
        node.fields[key] = term.scalar();
    }

    static void transform(std::string const& key
        , phylanx::ir::range & group
        , phylanx::algorithms::impl::randomforest_node & node) {

        std::vector<std::int64_t> grp(group.size());

        std::transform(
            group.begin(), group.end()
            , grp.begin()
            , [](auto const& g) { return phylanx::util::get<
                phylanx::ir::node_data<std::int64_t> >(
                    g.variant()).scalar(); });

        node.fields[key] = grp;
    }
 
    static void transform(std::string const& key
        , phylanx::ir::dictionary & node
        , phylanx::algorithms::impl::randomforest_node & forest) {

        for(auto & field : node) {
            switch(field.second.get().index()) {

               case 2: // std::int64_t
                   transform(phylanx::util::get<std::string>(field.first.get())
                       , phylanx::util::get< phylanx::ir::node_data<std::int64_t> >(
                           field.second.get()), forest);
                   break;

               case 4: // double
                   transform(phylanx::util::get<std::string>(field.first.get())
                       , phylanx::util::get< phylanx::ir::node_data<double> >(
                           field.second.get()), forest);
                   break;

               case 7: // phylanx::ir::range -> std::vector<std::int64_t>
                   transform(phylanx::util::get<std::string>(field.first.get())
                       , phylanx::util::get< phylanx::ir::range >(
                           field.second.get()), forest);
                   break;

               case 8: // phylanx::ir::dictionary -> randomforest_node
                   {
                       phylanx::algorithms::impl::randomforest_node child;

                       transform(phylanx::util::get<std::string>(field.first.get())
                           , phylanx::util::get<phylanx::ir::dictionary>(
                                       field.second.get()), child);

                       forest.fields[key] = child;
                   }
                   break;

               default:
                   HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::transform"
                       , phylanx::util::generate_error_message(
                           "unexpected element discovered in tree"));
            }
        }
    }

    static void transform(phylanx::ir::dictionary & phy
        , randomforest_impl & forest) {

        std::size_t tree_count = 0ul;
        std::size_t const forest_size = phy.size();
        forest.grow(forest_size);

        for(auto & entry : phy) {
            auto entry_first = phylanx::util::get<std::string>(entry.first.get());

            if(entry_first.compare("classes") == 0ul) {

                auto cls_dictionary =
                    phylanx::util::get<phylanx::ir::dictionary>(entry.second.get());

                forest.classes.reserve(cls_dictionary.size());

                for(auto cls : cls_dictionary) {
                    auto k = phylanx::util::get< phylanx::ir::node_data<double> >(
                        cls.first.get()).scalar();
                    auto v = phylanx::util::get< phylanx::ir::node_data<std::int64_t> >(
                        cls.second.get()).scalar();
                    forest.classes[k] = v;
                }
            }
            else {

                switch(entry.second.get().variant().index()) {

                   case 2: // std::int64_t
                       transform(phylanx::util::get<std::string>(entry.first.get())
                           , phylanx::util::get< phylanx::ir::node_data<std::int64_t> >(
                               entry.second.get()), forest.trees[tree_count]);
                       break;

                   case 4: // double
                       transform(phylanx::util::get<std::string>(entry.first.get())
                           , phylanx::util::get< phylanx::ir::node_data<double> >(
                               entry.second.get()), forest.trees[tree_count]);
                       break;

                   case 7: // phylanx::ir::range -> std::vector<std::int64_t>
                       transform(phylanx::util::get<std::string>(entry.first.get())
                           , phylanx::util::get< phylanx::ir::range >(
                               entry.second.get()), forest.trees[tree_count]);
                       break;

                   case 8: // phylanx::ir::dictionary -> randomforest_node
                       transform(phylanx::util::get<std::string>(entry.first.get())
                           , phylanx::util::get<phylanx::ir::dictionary>(
                               entry.second.get()), forest.trees[tree_count]);
                       break;

                   default:
                       HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::transform"
                           , phylanx::util::generate_error_message(
                               "unexpected element discovered in tree"));
                }
            }

            ++tree_count;
        }
    }

    static primitive_argument_type randomforest_fit_fn(
        blaze::DynamicMatrix<double> const& train
        , blaze::DynamicVector<double> const& train_labels
        , std::int64_t const max_depth
        , std::int64_t const min_size
        , double const sample_size
        , std::int64_t const n_trees) {

        randomforest_impl rf(n_trees);
        rf.fit(train, train_labels, max_depth, min_size, sample_size);

        phylanx::ir::dictionary forest;
        transform(rf, forest);

        return primitive_argument_type{std::move(forest)};
    }

    static primitive_argument_type randomforest_predict_fn(
        phylanx::ir::dictionary & tree
        , blaze::DynamicMatrix<double> const& input_data) {

        blaze::DynamicVector<double> labels(input_data.rows());

        randomforest_impl rf;
        transform(tree, rf);
        rf.predict(input_data, labels);

        return primitive_argument_type{std::move(labels)};
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const randomforest_fit::match_data =
    {
        hpx::util::make_tuple("randomforest_fit",
            std::vector<std::string>{
                "randomforest_fit(_1, _2, _3, _4, _5, _6)",
            },
            &create_randomforest_fit, &create_primitive<randomforest_fit>,
            "training, training_labels, max_depth, min_size, sample_size, n_trees\n"
            "Args:\n"
            "\n"
            "    training (matrix) : training features\n"
            "    training_labels (vector) : training labels\n"
            "the data\n"
            "    max_depth (int) : depth of tree splits\n"
            "    min_size (int) : min size\n"
            "    sample_size (double) : ratio of samples per tree\n"
            "    n_trees (int) : number of trees to train\n"
            "\n"
            "Returns:\n"
            "\n"
            "A trained model as a dictionary"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    randomforest_fit::randomforest_fit(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type randomforest_fit::calculate_randomforest_fit(
        primitive_arguments_type && args) const
    {
        // extract arguments
        auto arg1 = extract_numeric_value(args[0], name_, codename_);
        if (arg1.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the first "
                    "argument ('training_data') to represent a matrix"));
        }
        auto training_data = arg1.matrix();

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the second "
                    "argument ('training_labels') to represent a vector"));
        }
        auto training_labels = arg2.vector();

        // verify correctness of argument dimensions
        if (training_data.rows() != training_labels.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the number "
                    "of rows in 'training_data' to be equal to the size of  "
                    "'training_labels'"
                )
            );
        }

        auto max_depth = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[2], name_, codename_));
        auto min_size = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[3], name_, codename_));
        auto sample_size = static_cast<double>(
            extract_scalar_numeric_value(args[4], name_, codename_));
        auto n_trees = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[5], name_, codename_));

        using vector_type = ir::node_data<double>::storage1d_type;

        // perform calculations
        vector_type testing_labels(training_data.rows(), 0);

        auto forest = randomforest_fit_fn(
            training_data
            , training_labels
            , max_depth
            , min_size
            , sample_size
            , n_trees);

        return primitive_argument_type{std::move(forest)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> randomforest_fit::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "randomforest::eval",
                generate_error_message(
                    "the randomforest_fit primitive requires exactly six "
                        "operands"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type && args)
            ->  primitive_argument_type
            {
                return this_->calculate_randomforest_fit(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const randomforest_predict::match_data =
    {
        hpx::util::make_tuple("randomforest_predict",
            std::vector<std::string>{
                "randomforest_predict(_1, _2)",
            },
            &create_randomforest_predict, &create_primitive<randomforest_predict>,
            "model, input_data\n"
            "Args:\n"
            "\n"
            "    model (dictionary) : randomforest\n"
            "    input_data (matrix) : features\n"
            "the data\n"
            "\n"
            "Returns:\n"
            "\n"
            "The predicted classes as a vector for each row in training (matrix)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    randomforest_predict::randomforest_predict(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type randomforest_predict::calculate_randomforest_predict(
        primitive_arguments_type && args) const
    {
        // extract arguments
        auto model = extract_dictionary_value(args[0], name_, codename_);
        if (model.size() > 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the first "
                    "argument ('model') to have more than 0 trees"));
        }

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the second "
                    "argument ('input_data') to represent a matrix"));
        }
        auto input_data = arg2.matrix();

        using vector_type = ir::node_data<double>::storage1d_type;

        // perform calculations
        vector_type input_labels(input_data.rows(), 0.0);

        auto output_labels = randomforest_predict_fn(
            model
            , input_data);

        return primitive_argument_type{std::move(output_labels)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> randomforest_predict::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "randomforest::eval",
                generate_error_message(
                    "the randomforest_predict primitive requires exactly two "
                        "operands"));
        }

        bool arguments_valid = true;
        for (std::size_t i = 0; i != operands.size(); ++i)
        {
            if (!valid(operands[i]))
            {
                arguments_valid = false;
            }
        }

        if (!arguments_valid)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type && args)
            ->  primitive_argument_type
            {
                return this_->calculate_randomforest_predict(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_));
    }

}}}
