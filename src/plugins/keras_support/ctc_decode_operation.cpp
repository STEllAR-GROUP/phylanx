// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/ctc_decode_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const ctc_decode_operation::match_data = {
        hpx::util::make_tuple("ctc_decode",
            std::vector<std::string>{
                "ctc_decode(_1, _2, __arg(_3_greedy, 1), __arg(_4_beam_width, "
                "100), __arg(_5_top_paths, 1))"},
            &create_ctc_decode_operation,
            &create_primitive<ctc_decode_operation>,
            R"(y_pred, input_length, greedy, beam_width, top_paths
            Args:

                y_pred : The scalar, vector, matrix, or tensor to perform ctc_decode over
                input_length
                greedy : boolean, if True performs best-path search otherwise beam-search
                beam_width : Integer, if greedy is False specifies the width of the beam.
                top_paths : Integer, if greedy is False specifies the number of top paths
                            desired.
            Returns:

            Returns the result of Connectionist temporal classification applied to a
            squence.)")};

    ///////////////////////////////////////////////////////////////////////////
    ctc_decode_operation::ctc_decode_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> ctc_decode_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() < 2 || operands.size() > 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "ctc_decode_operation::eval",
                generate_error_message("the ctc_decode_operation primitive "
                                       "requires at least two and at "
                                       "most five operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "ctc_decode_operation::eval",
                generate_error_message(
                    "the ctc_decode_operation primitive requires that the "
                    "argument given by the operands array is valid"));
        }

        auto&& op0 = numeric_operand(operands[0], args, name_, codename_, ctx);
        auto&& op1 =
            integer_operand_strict(operands[1], args, name_, codename_, ctx);
        auto&& op2 =
            scalar_boolean_operand(operands[2], args, name_, codename_, ctx);
        auto&& op3 = scalar_integer_operand_strict(
            operands[3], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](ir::node_data<double>&& arg1,
                ir::node_data<std::int64_t>&& arg2, std::uint8_t greedy,
                std::int64_t beam_width, std::int64_t top_paths)
            -> primitive_argument_type
            {
                if (arg1.num_dimensions() != 3)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "ctc_decode_operation::eval",
                        this_->generate_error_message(
                            "y_pred should be a tensor"));

                auto y_pred = arg1.tensor();
                std::size_t num_samples = y_pred.pages();
                std::size_t seq_length = y_pred.rows();
                std::size_t num_classes = y_pred.columns();

                if (arg2.num_dimensions() != 1)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "ctc_decode_operation::eval",
                        this_->generate_error_message(
                            "input_length should be a vector"));

                auto input_length = arg2.vector();
                blaze::DynamicMatrix<double> log_prob(num_samples, 1, 0.);
                blaze::DynamicMatrix<double> decoded_dense(
                    num_samples, seq_length, -1.);

                blaze::DynamicVector<std::int64_t> decoded_length(
                    num_samples, 0.);

                if (!greedy)
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "ctc_decode_operation::eval",
                        this_->generate_error_message(
                            "has not been implemented yet"));

                using phylanx::util::matrix_row_iterator;

                for (std::size_t i = 0; i < num_samples; ++i)
                {
                    std::int64_t length = input_length[i];
                    auto prob = blaze::pageslice(y_pred, i);
                    auto tmp =
                        blaze::submatrix(prob, 0, 0, length, num_classes);
                    matrix_row_iterator<decltype(prob)> tmp_begin(prob);
                    matrix_row_iterator<decltype(prob)> tmp_end(
                        prob, length);

                    blaze::DynamicVector<double> decoded(length);
                    auto decoded_it = decoded.begin();
                    double sum = 0.;

                    for (auto it = tmp_begin; it != tmp_end;
                            ++it, ++decoded_it)
                    {
                        auto local_max =
                            std::max_element(it->begin(), it->end());
                        sum += blaze::log(*local_max);
                        *decoded_it = std::distance(it->begin(), local_max);
                    }
                    log_prob(i, 0) = -sum;
                    std::size_t k = 0;
                    for (std::size_t j = 0; j < decoded.size() - 1; ++j)
                    {
                        if ((decoded[j] != decoded[j + 1]) &&
                            (decoded[j] < num_classes - 1))
                            decoded[k++] = decoded[j];
                    }
                    decoded[k++] = decoded[decoded.size() - 1];
                    decoded.resize(k);

                    decoded_length[i] = k;
                    auto decoded_row = blaze::row(decoded_dense, i);
                    auto decoded_row_length =
                        blaze::subvector(decoded_row, 0, k);
                    decoded_row_length = blaze::trans(decoded);
                }

                blaze::DynamicMatrix<double> decoded_dense_final =
                    blaze::submatrix(decoded_dense, 0, 0, num_samples,
                        (blaze::max)(decoded_length));

                primitive_arguments_type result;
                result.reserve(2);

                result.emplace_back(std::move(decoded_dense_final));
                result.emplace_back(std::move(log_prob));

                return primitive_argument_type{std::move(result)};
            }),
            std::move(op0), std::move(op1), std::move(op2), std::move(op3),
            scalar_integer_operand_strict(
                operands[4], args, name_, codename_, std::move(ctx)));
    }
}}}
#endif
