//  Copyright (c) 2018 Shahrzad Shirzad
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/plugins/algorithms/als.hpp>

#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
    match_pattern_type const als::match_data = {hpx::util::make_tuple("als",
        std::vector<std::string>{
            "als(_1, _2, _3, _4, _5, _6)", "als(_1, _2, _3, _4, _5)"},
        &create_als, &create_primitive<als>,
        R"(ratings, reg, num, iters, alpha, enable_output
        Args:

            ratings (matrix): the matrix representing user feedback over
                             different items
            reg (float): the regularization parameter
            num (integer): the number of factors
            iters (integer): the number of iterations
            alpha (float): the scaling factor
            enable_output(boolean): whether output should be enabled.

        Returns:

        The algorithm returns a list of two matrices: [X, Y] :
        X: user-factors matrix
        Y: item-factors matrix

        Of possible interest: http://yifanhu.net/PUB/cf.pdf)"
        )};

    ///////////////////////////////////////////////////////////////////////////
    als::als(primitive_arguments_type && operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type als::calculate_als(
        primitive_arguments_type&& args) const
    {
        // extract arguments
        auto arg1 = extract_numeric_value(args[0], name_, codename_);
        if (arg1.num_dimensions() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "als::eval",
                generate_error_message(
                    "the als algorithm primitive requires for the first "
                    "argument ('ratings') to represent a matrix"));
        }
        auto ratings = arg1.matrix();

        auto arg2 = extract_numeric_value(args[1], name_, codename_);
        if (arg2.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "als::eval",
                generate_error_message(
                    "the als algorithm primitive requires for the second "
                    "argument ('regularization') to represent a scalar"));
        }
        auto regularization = arg2.scalar();

        auto num_factors = extract_scalar_integer_value(args[2], name_, codename_);

        auto iterations = extract_scalar_integer_value(args[3], name_, codename_);

        auto arg5 = extract_numeric_value(args[4], name_, codename_);
        if (arg5.num_dimensions() != 0)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter, "als::eval",
                generate_error_message(
                    "the als algorithm primitive requires for the second "
                    "argument ('alpha') to represent a scalar"));
        }
        auto alpha = arg5.scalar();

        bool enable_output = false;
        if (args.size() == 6 && valid(args[5]))
        {
            enable_output =
                extract_scalar_integer_value(args[5], name_, codename_) != 0;
        }

        using vector_type = ir::node_data<double>::storage1d_type;
        using matrix_type = ir::node_data<double>::storage2d_type;

        // perform calculations
        std::int64_t num_users = ratings.rows();
        std::int64_t num_items = ratings.columns();
        auto conf = alpha * ratings;

        matrix_type X(num_users, num_factors);
        matrix_type Y(num_items, num_factors);

        matrix_type XtX(num_factors, num_factors);
        matrix_type YtY(num_factors, num_factors);

        std::uint32_t seed_ = 0;
        std::mt19937 rng_{seed_};
        std::normal_distribution<double> dist;

        for (std::size_t row = 0; row != blaze::rows(X); ++row)
        {
            for (auto& val : blaze::row(X, row))
            {
                val = dist(rng_);
            }
        }

        for (std::size_t row = 0; row != blaze::rows(Y); ++row)
        {
            for (auto& val : blaze::row(Y, row))
            {
                val = dist(rng_);
            }
        }

        blaze::IdentityMatrix<double> I_f(num_factors);
        blaze::IdentityMatrix<double> I_i(num_items);
        blaze::IdentityMatrix<double> I_u(num_users);

        blaze::DynamicMatrix<double> c_u(num_items, num_items, 0);
        blaze::DynamicMatrix<double> c_i(num_users, num_users, 0);

        for (std::int64_t step = 0; step < iterations; ++step)
        {
            YtY = (blaze::trans(Y) * Y) + regularization * I_f;
            XtX = (blaze::trans(X) * X) + regularization * I_f;

            if (enable_output)
            {
                hpx::cout << "iteration " << step << "\nX: " << X
                          << "\nY: " << Y << std::endl;
            }

            for (std::int64_t u = 0; u < num_users; u++)
            {
                auto conf_u = blaze::trans(blaze::row(conf, u));
                auto diag = blaze::band(c_u, 0);
                diag = conf_u;
                blaze::DynamicVector<double> p_u =
                    blaze::map(conf_u, [&](double x) { return (x != 0.0); });
                auto A = (blaze::trans(Y) * c_u) * Y + YtY;
                auto b = (blaze::trans(Y) * (c_u + I_i)) * (p_u);
                auto row_x = blaze::row(X, u);
                row_x = (trans(b) * blaze::inv(A));
            }

            for (std::int64_t i = 0; i < num_items; i++)
            {
                auto conf_i = blaze::column(conf, i);
                auto diag = blaze::band(c_i, 0);
                diag = conf_i;
                blaze::DynamicVector<double> p_i =
                    blaze::map(conf_i, [&](double x) { return (x != 0.0); });
                auto A = (blaze::trans(X) * c_i) * X + XtX;
                auto b = (blaze::trans(X) * (c_i + I_u)) * (p_i);
                auto row_y = blaze::row(Y, i);
                row_y = (trans(b) * blaze::inv(A));
            }
        }

        return primitive_argument_type
        {
            primitive_arguments_type{
                ir::node_data<double>{std::move(X)},
                ir::node_data<double>{std::move(Y)}}
        };
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> als::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 5 && operands.size() != 6)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "als::eval",
                generate_error_message("the als algorithm primitive "
                                       "requires exactly either "
                                       "five or six operands"));
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
                "als::eval",
                generate_error_message(
                    "the als algorithm primitive requires that the "
                    "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                return this_->calculate_als(std::move(args));
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
