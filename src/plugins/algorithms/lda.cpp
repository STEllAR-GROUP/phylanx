//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/algorithms/lda.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const lda_trainer::match_data = {hpx::util::make_tuple("lda_trainer",
        std::vector<std::string>{
            "lda_trainer(_1, _2, _3, _4, _5)"},
        &create_lda_trainer, &create_primitive<lda_trainer>,
        "n_topics, alpha, beta, iters, word_doc_matrix\n"
        "Args:\n"
        "\n"
        "    n_topics (integer): number of topics to compute\n"
        "    alpha (float): alpha parameter\n"
        "    beta (float): beta parameter\n"
        "    iters (integer): the number of iterations\n"
        "    word_doc_matrix (2x2 matrix float): word-document histogram, words are columns, documents are rows\n"
        "\n"
        "Returns:\n"
        "\n"
        "The algorithm returns a list of two matrices: [word_topic, document_topic] :\n"
        "word_topic: document-topic assignment matrix\n"
        "document_topic: document-topic assignment matrix\n"
        "\n"
        )};

    ///////////////////////////////////////////////////////////////////////////
    lda_trainer::lda_trainer(primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type lda_trainer::calculate_lda_trainer(
        primitive_arguments_type && args) const
    {
        // extract arguments
        auto arg1 = extract_numeric_value(args[0], name_, codename_);
        if (arg1.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lda_trainer::eval",
                generate_error_message(
                    "the lda_trainer algorithm primitive requires for the first "
                    "argument ('n_topics') to represent a matrix"));
        }
        auto topics = arg1.scalar();

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lda_trainer::eval",
                generate_error_message(
                    "the lda_trainer algorithm primitive requires for the second "
                    "argument ('alpha') to represent a scalar"));
        }
        auto alpha = arg2.scalar();

        auto arg3 = extract_numeric_value(args[2], name_, codename_);
        if (arg2.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lda_trainer::eval",
                generate_error_message(
                    "the lda_trainer algorithm primitive requires for the second "
                    "argument ('beta') to represent a scalar"));
        }
        auto beta = arg3.scalar();

        auto arg4 = extract_scalar_integer_value(args[3], name_, codename_);
        if (arg2.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lda_trainer::eval",
                generate_error_message(
                    "the lda_trainer algorithm primitive requires for the second "
                    "argument ('iters') to represent a scalar"));
        }
        auto iterations = arg3.scalar();


        // this is the word-count matrix
        //

        using namespace execution_tree;

        auto arg5 = extract_numeric_value(args[4], name_, codename_);

/*
        // what about a local matrix
        if (arg5.has_annotation())
        {
            // is distributed, run distributed algo logic
            //execution_tree::localities_information wcm_localities =
            //    extract_localities_information(args[4], name_, codename_);
            // distributed lda!
            //
            if(wcm_localities.locality_.num_localities_ > 1) {

                util::distributed_matrix<double> dist_word_doc_mat(
                    wcm_localities.annotation_.name_,
                    word_doc_mat,
                    wcm_localities,
                    wcm_localities.locality_.locality_id_
                );

                auto result = trainer(dist_word_doc_mat, topics, iterations, wcm_localities);
 
                return primitive_argument_type
                {
                    primitive_arguments_type{
                        ir::node_data<double>{std::move(std::get<0>(result))},
                        ir::node_data<double>{std::move(std::get<1>(result))}
                    }
                };
            }
        }
*/

        if (arg5.num_dimensions() > 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lda_trainer::eval",
                generate_error_message(
                    "the lda_trainer algorithm primitive requires for the second "
                    "argument ('word_doc_mat') to represent a matrix"));
        }
        auto word_doc_mat = arg5.matrix();

        using lda_trainer_t =
            phylanx::execution_tree::primitives::impl::lda_trainer;

        lda_trainer_t trainer(alpha, beta);

        auto result = trainer(word_doc_mat, topics, iterations);

        return primitive_argument_type
        {
            primitive_arguments_type{
                ir::node_data<double>{std::move(std::get<0>(result))},
                ir::node_data<double>{std::move(std::get<1>(result))}
            }
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> lda_trainer::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args) const
    {
        if (operands.size() != 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "lda_trainer::eval",
                generate_error_message("the lda_trainer algorithm primitive "
                                       "requires exactly "
                                       "five operands"));
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
                "lda_trainer::eval",
                generate_error_message(
                    "the lda_trainer algorithm primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_)](primitive_arguments_type&& args)
                    -> primitive_argument_type
                {
                    return this_->calculate_lda_trainer(std::move(args));
                }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }

/*
    hpx::future<primitive_argument_type> lda_trainer::eval(
        primitive_arguments_type const& args) const
    {
        if (this->no_operands())
        {
            return eval(args, noargs);
        }
        return eval(this->operands(), args);
    }
*/
}}}
