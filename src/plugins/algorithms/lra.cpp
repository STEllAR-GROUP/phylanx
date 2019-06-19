//  Copyright (c) 2018-2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/algorithms/lra.hpp>
#include <phylanx/util/detail/add_simd.hpp>
#include <phylanx/util/detail/div_simd.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <cstddef>
#include <cstdint>
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
    match_pattern_type const lra::match_data =
    {
        hpx::util::make_tuple("lra",
            std::vector<std::string>{
                "lra(_1, _2, _3, _4, _5)",
                "lra(_1, _2, _3, _4)"
            },
            &create_lra, &create_primitive<lra>,
            R"(x, y, alpha, iters, enable_output
            Args:

                x (matrix) : a matrix
                y (vector) : a vector
            the data
                alpha (float): It is the learning rate
                iters (int): The number of iterations
                enable_output (optional, boolean): If enabled, prints out the step
            number and weights during each iteration

            Returns:

            The Calculated weights)"
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    lra::lra(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type lra::calculate_lra(
        primitive_arguments_type && args) const
    {
        // extract arguments
        auto arg1 = extract_numeric_value(args[0], name_, codename_);
        if (arg1.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the first "
                    "argument ('x') to represent a matrix"));
        }
        auto x = arg1.matrix();

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the second "
                    "argument ('y') to represent a vector"));
        }
        auto y = arg2.vector();

        // verify correctness of argument dimensions
        if (x.rows() != y.size())
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the number "
                    "of rows in 'x' to be equal to the size of 'y'"));
        }

        auto arg3 = extract_numeric_value(args[2], name_, codename_);
        if (arg3.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires for the second "
                    "argument ('alpha') the learning rate"));
        }
        auto alpha = arg3.scalar();

        auto iterations =
            extract_scalar_integer_value(args[3], name_, codename_);

        bool enable_output = false;
        if (args.size() == 5 && valid(args[4]))
        {
            enable_output =
                extract_scalar_boolean_value(args[4], name_, codename_) != 0;
        }

        using vector_type = ir::node_data<double>::storage1d_type;
        using matrix_type = ir::node_data<double>::storage2d_type;

        // perform calculations
        vector_type weights(x.columns(), 0.0);
        matrix_type transx = blaze::trans(x);

        for (std::int64_t step = 0; step < iterations; ++step)
        {
            if (enable_output)
            {
                hpx::cout << "step: " << step << ", " << weights << std::endl;
            }

            // pred = 1.0 / (1.0 + exp(-dot(x, weights)))
            auto pred = blaze::map(
                blaze::map(blaze::exp(-(x * weights)),
                    util::detail::addnd0d_simd(1.0)),
                util::detail::div0dnd_simd(1.0));

            weights = weights - (alpha * (transx * (pred - y)));
        }

        return primitive_argument_type{std::move(weights)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> lra::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 4 && operands.size() != 5)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires exactly either "
                        "four or five operands"));
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
                "lra::eval",
                generate_error_message(
                    "the lra algorithm primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type && args)
            ->  primitive_argument_type
            {
                return this_->calculate_lra(std::move(args));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
