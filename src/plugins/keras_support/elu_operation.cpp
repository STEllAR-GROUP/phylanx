// Copyright (c) 2018-2019 Hartmut Kaiser
// Copyright (c) 2018-2019 Jules Pénuchot
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
     hpx::util::make_tuple("elu",
                           std::vector<std::string>{"elu(_1)", "elu(_1, _2)"},
                           &create_elu_operation,
                           &create_primitive<elu_operation>,
                           R"(a
                           Args:

                               a (array_like) : input array

            Returns:

            Returns an array of the same shape... .)") // TODO
    };

    elu_operation::elu_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
        : primitive_component_base{ std::move(operands), name, codename }
    {}

    primitive_argument_type elu_operation::elu(mat_type&& arg, double alpha) const
    {
        //  ELU activation function
        auto elu_ = [&](auto&& x)
        {
            return (x >= 0.) * ( x )
                 + (x <  0.) * ( alpha * (std::exp(x) - 1.) );
        };

        switch(arg.num_dimensions())
        {
        case 0: //  Scalar ELU
            return primitive_argument_type{mat_type::storage0d_type{
                elu_(arg.scalar()) }};

        case 1: //  Vector ELU
            return primitive_argument_type{mat_type::storage1d_type{
                arg.is_ref() ? blaze::map(arg.vector(), elu_)
                    : blaze::map(std::move(arg.vector()), elu_)
            }};

        case 2: //  Matrix ELU
            return primitive_argument_type{mat_type::storage2d_type{
                arg.is_ref() ? blaze::map(arg.matrix(), elu_)
                    : blaze::map(std::move(arg.matrix()), elu_)
            }};

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        case 3: //  Tensor ELU
            return primitive_argument_type{mat_type::storage3d_type{
                arg.is_ref() ? blaze::map(arg.tensor(), elu_)
                    : blaze::map(std::move(arg.tensor()), elu_)
            }};
#endif
        }

        /* TODO: Throw exception ? */
        return primitive_argument_type{ 0. };
    }

    hpx::future<primitive_argument_type> elu_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if(operands.size() != 1 && operands.size() != 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "elu_operation::eval",
                generate_error_message(
                    "the elu_operation primitive requires  exactly one or two "
                    "operands"));
        }

        if (!valid(operands[0]))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "elu_operation::eval",
                generate_error_message(
                    "the elu_operation primitive requires that the arguments "
                    "given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();

        //  In case alpha isn't provided (default value)
        if(operands.size() == 1)
        {
            return value_operand(operands[0], args, name_, codename_,
                std::move(ctx))
                .then(hpx::launch::sync, hpx::util::unwrapping(
                    [this_ = std::move(this_)](primitive_argument_type&& arg_mat)
                    -> primitive_argument_type
                    {
                        mat_type mat = extract_numeric_value(
                            std::move(arg_mat),
                            this_->name_, this_->codename_);

                        return this_->elu(std::move(mat));
                    }));
        }

        //  alpha is provided
        else if(operands.size() == 2 && valid(operands[1]))
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)]
                (primitive_argument_type&& arg_mat,
                    primitive_argument_type&& arg_alpha)
                {
                    mat_type mat = extract_numeric_value(
                        std::move(arg_mat),
                        this_->name_, this_->codename_);

                    alpha_type alpha = extract_numeric_value(
                        std::move(arg_alpha),
                        this_->name_, this_->codename_);

                    if(alpha.num_dimensions() != 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "elu_operation::eval",
                            this_->generate_error_message(
                                "the elu primitive requires that the alpha "
                                "argument given by the operands array is "
                                "scalar"));
                    }

                    return this_->elu(std::move(mat), alpha.scalar());
                }),
                value_operand(operands[0], args, name_, codename_, ctx),
                value_operand(operands[1], args, name_, codename_, ctx));
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "elu_operation::eval",
            generate_error_message(
                "the elu_operation primitive requires that the arguments given "
                "by the operands array are valid"));
    }
}}}
