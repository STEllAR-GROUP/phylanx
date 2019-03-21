// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/categorical_crossentropy_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

#include <algorithm>
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

#define HERE std::cout << "Here " << __LINE__ << std::endl

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const categorical_crossentropy_operation::match_data =
    {
        hpx::util::make_tuple("categorical_crossentropy",
        std::vector<std::string>{
            "categorical_crossentropy(_1,_2)",
            "categorical_crossentropy(_1,_2,_3)"
        },
        &create_categorical_crossentropy_operation, &create_primitive<categorical_crossentropy_operation>,
        R"(a, axis
        Args:

            a (array_like) : input array
            axis (optional, integer): an axis to categorical_crossentropy along. The
                default is the last axis (axis == -1) of an array. Axis
                is effective for >1d arrays.

        Returns:

        Returns an array of the same shape which is the normalized exponential
        function of the given array.  The resulting array consists of real
        values in the range (0..1], which add up to 1 in direction of the given axis)")
    };

    ///////////////////////////////////////////////////////////////////////////
    categorical_crossentropy_operation::categorical_crossentropy_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type categorical_crossentropy_operation::categorical_crossentropy0d() const
    {
        return primitive_argument_type{static_cast<double>(1.)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type categorical_crossentropy_operation::categorical_crossentropy1d(arg_type&& target,arg_type&& output,bool from_logits) const
    {
        #if 0
        if (!arg.is_ref())
        {
            arg.vector() = blaze::categorical_crossentropy(arg.vector());
            return primitive_argument_type{std::move(arg)};
        }
        return primitive_argument_type{blaze::categorical_crossentropy(arg.vector())};
        #endif
        HERE;
        if(from_logits)
        {
            output.vector() = blaze::softmax(output.vector());
        }
        else
        {
            output.vector() /= blaze::sum(output.vector());
        }
        HERE;
        const double small = 1e-7;
        arg_type lo{small};
        arg_type hi{1-small};
        HERE;
        output.vector() = blaze::max(blaze::min(output.vector(), hi.vector()), lo.vector());
        HERE;
        auto res = target.vector() * (-blaze::log(output.vector()));
        HERE;
        double d = blaze::sum(res);
        HERE;
        return primitive_argument_type{ d };
    }

    ///////////////////////////////////////////////////////////////////////////

    primitive_argument_type categorical_crossentropy_operation::categorical_crossentropy2d(
        arg_type&& target, arg_type&& output,bool from_logits) const
    {
        return primitive_argument_type{};
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)

    primitive_argument_type categorical_crossentropy_operation::categorical_crossentropy3d(
        arg_type&& target, arg_type&& output, bool from_logits) const
    {
        return primitive_argument_type{};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> categorical_crossentropy_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        #if 0
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "categorical_crossentropy_operation::eval",
                util::generate_error_message(
                    "the categorical_crossentropy_operation primitive requires exactly one, or "
                    "two operands",
                    name_, codename_));
        }
        #endif

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "categorical_crossentropy_operation::eval",
                    util::generate_error_message(
                        "the categorical_crossentropy_operation primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {
                HERE;
                // Extract logits
                std::int64_t from_logits =
                    static_cast<bool>(false);
                HERE;

                // from_logits is the third argument
                if (args.size() > 2)
                {
                HERE;
                    if (valid(args[2]))
                        from_logits = execution_tree::extract_scalar_boolean_value(
                            args[2], this_->name_, this_->codename_);
                }
                HERE;

                // Extract the matrix, the result should always be double
                arg_type target = extract_numeric_value(
                    std::move(args[0]), this_->name_, this_->codename_);
                HERE;
                arg_type output = extract_numeric_value(
                    std::move(args[1]), this_->name_, this_->codename_);
                HERE;

                std::size_t target_dims = target.num_dimensions();
                std::size_t output_dims = output.num_dimensions();
                HERE;
                HPX_ASSERT(target_dims == output_dims);
                HERE;

                switch (target_dims)
                {
                case 0:
                    return this_->categorical_crossentropy0d();
                case 1:
                HERE;
                    return this_->categorical_crossentropy1d(std::move(target),std::move(output),from_logits);
                case 2:
                    return this_->categorical_crossentropy2d(std::move(target),std::move(output),from_logits);
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->categorical_crossentropy3d(std::move(target),std::move(output),from_logits);
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "categorical_crossentropy_operation::eval",
                        util::generate_error_message("operand a has an invalid "
                                                        "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}

