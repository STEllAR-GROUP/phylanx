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
#include <future>

#include <blaze/Math.h>

using namespace phylanx::algorithms::impl;

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    static phylanx::ir::dictionary randomforest_fit_fn(
        blaze::DynamicMatrix<double> const& train
        , blaze::DynamicVector<double> const& train_labels
        , std::int64_t const max_depth
        , std::int64_t const min_size
        , std::int64_t const sample_size
        , std::int64_t const n_trees) {

        randomforest_impl rf(n_trees);
        rf.fit(train, train_labels, max_depth, min_size, sample_size);

        return std::move(forest);
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const randomforest_fit::match_data =
    {
        hpx::util::make_tuple("randomforest_fit",
            std::vector<std::string>{
                "randomforest_fit(_1, _2, _3, _4, _5, _6, _7)",
            },
            &create_randomforest_fit, &create_primitive<randomforest_fit>,
            "training, training_labels, max_depth, min_size, sample_size, n_trees, test\n"
            "Args:\n"
            "\n"
            "    training (matrix) : training features\n"
            "    training_labels (vector) : training labels\n"
            "the data\n"
            "    max_depth (int) : depth of tree splits\n"
            "    min_size (int) : min size\n"
            "    sample_size (int) : number of samples per tree\n"
            "    n_trees (int) : number of trees to train\n"
            "    test (matrix) : testing data to predict upon\n"
            "\n"
            "Returns:\n"
            "\n"
            "The Calculated weights"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    randomforest_fit::randomforest_fit(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type randomforest_fit::calculate(
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
                    "of rows in 'training_data' to be equal to the size of 'training_labels'"));
        }

        auto max_depth = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[2], name_, codename_));
        auto min_size = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[3], name_, codename_));
        auto sample_size = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[4], name_, codename_));
        auto n_trees = static_cast<std::int64_t>(
            extract_scalar_integer_value(args[5], name_, codename_));

/*
        auto arg6 = extract_numeric_value(args[6], name_, codename_);
        if (arg6.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the first "
                    "argument ('testing') to represent a matrix"));
        }

        auto testing_data = arg6.matrix();
*/
//        using vector_type = ir::node_data<double>::storage1d_type;

        // perform calculations
//        vector_type testing_labels(training_data.rows(), 0);


        auto forest = randomforest_fit_fn(training_data
            , training_labels
            , max_depth
            , min_size
            , sample_size
            , n_trees);

        return primitive_argument_type{std::move(forest)}; //std::move(forest)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> randomforest_fit::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 7)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires exactly seven "
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
                return this_->calculate(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }

    ///////////////////////////////////////////////////////////////////////////
    static blaze::DynamicVector<double> randomforest_predict_fn(
        phylanx::ir::dictionary & tree
        , blaze::DynamicMatrix<double> const& test) {

        blaze::DynamicVector<double> test_labels(test.rows());
        randomforest_impl rf(tree.size());
        transform(tree, rf);
        rf.predict(test, test_labels);
        return std::move(test_labels);
    }

    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const randomforest_predict::match_data =
    {
        hpx::util::make_tuple("randomforest_predict",
            std::vector<std::string>{
                "randomforest_predict(_1, _2)",
            },
            &create_randomforest_predict, &create_primitive<randomforest_predict>,
            "training, training_labels, max_depth, min_size, sample_size, n_trees, test\n"
            "Args:\n"
            "\n"
            "    model (randomforest_fit) : trained randomforest model\n"
            "    data (matrix) : features to predict\n"
            "\n"
            "Returns:\n"
            "\n"
            "The Calculated labels"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    randomforest_predict::randomforest_predict(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type randomforest_predict::calculate(
        primitive_arguments_type && args) const
    {
        // extract arguments
        auto model = extract_dictionary_value(args[0], name_, codename_);
        if (!is_dictionary_operand(model))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the first "
                    "argument ('model') to represent a phylanx::ir::dictionary"));
        }

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "randomforest::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires for the second "
                    "argument ('data') to represent a matrix"));
        }
        auto data = arg2.matrix();

        // perform calculations
        auto predicted_labels = randomforest_predict_fn(model, data);

        return primitive_argument_type{std::move(predicted_labels)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> randomforest_predict::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "randomforest_predict::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires exactly two "
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
                "randomforest_predict::eval",
                generate_error_message(
                    "the randomforest algorithm primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type && args)
            ->  primitive_argument_type
            {
                return this_->calculate(std::move(args));
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }

}}}
