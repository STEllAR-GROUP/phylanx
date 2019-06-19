// Copyright (c) 2019 Shahrzad Shirzad
// Copyright (c) 2018-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/softsign_operation.hpp>

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
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const softsign_operation::match_data =
    {
        hpx::util::make_tuple("softsign",
            std::vector<std::string>{"softsign(_1)"},
            &create_softsign_operation, &create_primitive<softsign_operation>,
            R"(a
            Args:

                a (array_like) : input array

            Returns:

            Returns an array of the same shape which is defined as
            f(x) = 1/(1+abs(x).)")
    };

    ///////////////////////////////////////////////////////////////////////////
    softsign_operation::softsign_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type softsign_operation::softsign0d(arg_type&& arg) const
    {
        auto a = arg.scalar();
        return primitive_argument_type{a / (1 + blaze::abs(a))};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type softsign_operation::softsign1d(arg_type&& arg) const
    {
        auto v = arg.vector();

        if (!arg.is_ref())
        {
            arg =
                blaze::map(v, [](double a) { return a / (1 + blaze::abs(a)); });
        }
        else
        {
            arg.vector() =
                blaze::map(v, [](double a) { return a / (1 + blaze::abs(a)); });
        }
        return primitive_argument_type{std::move(arg)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type softsign_operation::softsign2d(arg_type&& arg) const
    {
        auto m = arg.matrix();

        if (!arg.is_ref())
        {
            arg =
                blaze::map(m, [](double a) { return a / (1 + blaze::abs(a)); });
        }
        else
        {
            m =
                blaze::map(m, [](double a) { return a / (1 + blaze::abs(a)); });
        }
        return primitive_argument_type{std::move(arg)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type softsign_operation::softsign3d(arg_type&& arg) const
    {
        auto t = arg.tensor();

        if (!arg.is_ref())
        {
            arg =
                blaze::map(t, [](double a) { return a / (1 + blaze::abs(a)); });
        }
        else
        {
            t =
                blaze::map(t, [](double a) { return a / (1 + blaze::abs(a)); });
        }
        return primitive_argument_type{std::move(arg)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> softsign_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.size() != 1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "softsign_operation::eval",
                generate_error_message(
                    "the softsign_operation primitive requires exactly "
                    "one operand"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "softsign_operation::eval",
                generate_error_message(
                    "the softsign_operation primitive requires that the "
                    "argument given by the operands array is valid"));
        }

        auto this_ = this->shared_from_this();
        return value_operand(operands[0], args, name_, codename_, std::move(ctx))
            .then(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& arg)
                -> primitive_argument_type
                {
                    // Extract the argument, the result should always be double
                    arg_type a = extract_numeric_value(
                        arg.get(), this_->name_, this_->codename_);

                    std::size_t a_dims = a.num_dimensions();

                    switch (a_dims)
                    {
                    case 0:
                        return this_->softsign0d(std::move(a));

                    case 1:
                        return this_->softsign1d(std::move(a));

                    case 2:
                        return this_->softsign2d(std::move(a));

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        return this_->softsign3d(std::move(a));
#endif
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "softsign_operation::eval",
                            this_->generate_error_message(
                                "operand a has an invalid number of "
                                "dimensions"));
                    }
                });
    }
}}}
