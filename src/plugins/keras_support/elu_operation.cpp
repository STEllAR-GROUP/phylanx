// Copyright (c) 2018-2019 Hartmut Kaiser
// Copyright (c) 2018-2019 Jules Penuchot
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/elu_operation.hpp>

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
    match_pattern_type const elu_operation::match_data =
    {
         match_pattern_type{
            "elu",
            std::vector<std::string>{"elu(_1, _2)"},
            &create_elu_operation,
            &create_primitive<elu_operation>, R"(
            a
            Args:

                a (array_like) : input array
                alpha (optional, number) : scale for the negative factor

            Returns:

            Returns an array of the same shape that is the exponential linear
            unit of the input and is defined as:
            * f(x) = alpha * (exp(x) - 1.) for x < 0,
            * f(x) = x for x >= 0.)"
        }
    };

    elu_operation::elu_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base{ std::move(operands), name, codename }
    {}

    primitive_argument_type elu_operation::elu0d(mat_type&& arg,
        double alpha) const
    {
        auto elu_ = [alpha](auto const& x)
        {
            return (x >= 0.) * ( x )
                 + (x <  0.) * ( alpha * (std::exp(x) - 1.) );
        };

        return primitive_argument_type{ elu_(arg.scalar()) };
    }

    primitive_argument_type elu_operation::elu1d(mat_type&& arg,
        double alpha) const
    {
        auto elu_ = [alpha](auto const& x)
        {
            return (x >= 0.) * ( x )
                 + (x <  0.) * ( alpha * (std::exp(x) - 1.) );
        };

        if(!arg.is_ref())
        {
            arg.vector() = blaze::map(arg.vector(), elu_);
        }
        else
        {
            arg = blaze::map(arg.vector(), elu_);
        }

        return primitive_argument_type{ std::move(arg) };
    }

    primitive_argument_type elu_operation::elu2d(mat_type&& arg,
        double alpha) const
    {
        auto elu_ = [alpha](auto const& x)
        {
            return (x >= 0.) * ( x )
                 + (x <  0.) * ( alpha * (std::exp(x) - 1.) );
        };

        if(!arg.is_ref())
        {
            arg.matrix() = blaze::map(arg.matrix(), elu_);
        }
        else
        {
            arg = blaze::map(arg.matrix(), elu_);
        }

        return primitive_argument_type{ std::move(arg) };
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type elu_operation::elu3d(mat_type&& arg,
        double alpha) const
    {
        auto elu_ = [alpha](auto const& x)
        {
            return (x >= 0.) * ( x )
                 + (x <  0.) * ( alpha * (std::exp(x) - 1.) );
        };

        if(!arg.is_ref())
        {
            arg.tensor() = blaze::map(arg.tensor(), elu_);
        }
        else
        {
            arg = blaze::map(arg.tensor(), elu_);
        }

        return primitive_argument_type{ std::move(arg) };
    }
#endif

    hpx::future<primitive_argument_type> elu_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if(operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "elu_operation::eval",
                generate_error_message(
                    "the elu_operation primitive requires exactly two "
                    "operands"));
        }

        if (!valid(operands[0]) || !valid(operands[1]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "elu_operation::eval",
                generate_error_message(
                    "the elu_operation primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                hpx::future<primitive_argument_type>&& f_mat,
                hpx::future<primitive_argument_type>&& f_alpha)
            {
                auto&& arg_mat = f_mat.get();
                auto&& arg_alpha = f_alpha.get();

                mat_type mat = extract_numeric_value(
                    std::move(arg_mat),
                    this_->name_, this_->codename_);

                alpha_type alpha = extract_numeric_value(
                    std::move(arg_alpha), this_->name_, this_->codename_);

                if(alpha.num_dimensions() != 0)
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "elu_operation::eval",
                        this_->generate_error_message(
                            "the elu primitive requires that the alpha "
                            "argument given by the operands array is "
                            "scalar"));
                }

                switch(mat.num_dimensions())
                {
                case 0:
                    return this_->elu0d(std::move(mat), alpha.scalar());

                case 1:
                    return this_->elu1d(std::move(mat), alpha.scalar());

                case 2:
                    return this_->elu2d(std::move(mat), alpha.scalar());

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->elu3d(std::move(mat), alpha.scalar());
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "elu_operation::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            },
            std::move(op0),
            value_operand(operands[1], args, name_, codename_, std::move(ctx)));
    }
}}}
