// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/rnn.hpp>

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
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const rnn::match_data = {hpx::util::make_tuple("rnn",
        std::vector<std::string>{
            "rnn(_1)"},
        &create_rnn, &create_primitive<rnn>,
        R"(step_function, inputs, initial_states, go_backwards, mask, constants,
        unroll, input_length
            Args:

                step_function (function) :
                inputs: Tensor of temporal data of shape (samples, time, ...)
                initial_states : Tensor with shape (samples, ...) (no time dimension), containing the initial values for the states used in the step function.
                go_backwards (boolean) : If True, do the iteration over the time dimension in reverse order and return the reversed sequence.
                mask : Binary tensor with shape (samples, time), with a zero for every element that is masked.
                constants : A list of constant values passed at each step.
                unroll : Whether to unroll the RNN or to use a symbolic loop (while_loop or scan depending on backend).
                input_length : Static number of timesteps in the input.
                a (array_like) : input array

            Returns:

            )")};

    ///////////////////////////////////////////////////////////////////////////
    rnn::rnn(primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> rnn::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping(
                [this_ = std::move(this_), ctx](
                    primitive_argument_type&& bound_func) mutable
                -> primitive_argument_type {
                    primitive const* p = util::get_if<primitive>(&bound_func);
                    if (p == nullptr)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "rnn::eval",
                            this_->generate_error_message(
                                "the first argument to filter must be an "
                                "invocable object"));
                    }

                    primitive_arguments_type f_inputs;
                    f_inputs.reserve(1);
                    blaze::DynamicMatrix<double> m{{1., 2.},{3., 4.}};

                    f_inputs.push_back(primitive_argument_type{std::move(m)});

                    auto result =
                        numeric_operand_sync(bound_func, std::move(f_inputs),
                            this_->name_, this_->codename_, ctx);

                    return primitive_argument_type{std::move(result)};
                }),
            value_operand(operands_[0], args, name_, codename_,
                add_mode(ctx, eval_dont_evaluate_lambdas)));
    }
}}}
