// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
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
    match_pattern_type const cat_cross_operation::match_data =
    {
        hpx::util::make_tuple("categorical_crossentropy",
        std::vector<std::string>{
            "categorical_crossentropy(_1,_2)",
            "categorical_crossentropy(_1,_2,_3)"
        },
        &create_cat_cross_operation, &create_primitive<cat_cross_operation>,
        R"(a, axis
        Args:

            target (array_like) : input array
            output (array_like) : output array
            from_logits (boolean): boolean value

        Returns:
            The value should be the same as would be returned by the following
            Python function:

            def cat_cross(target, output, from_logits=False):
                if from_logits:
                    output = softmax(output)
                else:
                    output /= output.sum(axis=-1, keepdims=True)
                output = np.clip(output, 1e-7, 1 - 1e-7)
                return np.sum(target * -np.log(output), axis=-1, keepdims=False)
        )")
    };
    const double small = 1e-7;

    ///////////////////////////////////////////////////////////////////////////
    cat_cross_operation::cat_cross_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type cat_cross_operation::cat_cross0d() const
    {
        /*
        if(from_logits)
        {
            // softmax0d is 1
            output.scalar() = blaze::softmax(output.scalar());
        }
        else
        {
            // divide by itself is 1
            output.scalar() /= blaze::sum(output.scalar());
        }
        // regardless of from_logits, output = 1

        // Construct low and high clip regions
        primitive_argument_type lo_{small}, hi_{1-small};

        auto lo = extract_value_scalar<double>(lo_, name_, codename_ );
        auto hi = extract_value_scalar<double>(hi_, name_, codename_ );

        output.scalar() = blaze::max(blaze::min(
            output.scalar(), hi.scalar()), lo.scalar());
        // output is still 1 after clipping

        // log of 1 is zero, so res is zero
        auto res = target.scalar() * (-blaze::log(output.scalar()));
        return primitive_argument_type{ blaze::sum(res) };
        */
        return primitive_argument_type{static_cast<double>(0.)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type cat_cross_operation::cat_cross1d(
        arg_type&& target,arg_type&& output,bool from_logits) const
    {
        if(from_logits)
        {
            output.vector() = blaze::softmax(output.vector());
        }
        else
        {
            output.vector() /= blaze::sum(output.vector());
        }

        // Construct low and high clip regions
        auto sz0 = output.dimension(0);
        primitive_argument_type lo_{small}, hi_{1-small};

        auto lo = extract_value_vector<double>(lo_, sz0, name_, codename_ );
        auto hi = extract_value_vector<double>(hi_, sz0, name_, codename_ );

        output.vector() = blaze::max(
            blaze::min(output.vector(), hi.vector()), lo.vector());

        auto res = target.vector() * (-blaze::log(output.vector()));
        return primitive_argument_type{ blaze::sum(res) };
    }

    ///////////////////////////////////////////////////////////////////////////

    primitive_argument_type cat_cross_operation::cat_cross2d(
        arg_type&& target, arg_type&& output,bool from_logits) const
    {
        if(from_logits)
        {
            output.matrix() = blaze::softmax<blaze::rowwise>(output.matrix());
        }
        else
        {
            output.matrix() /= blaze::sum(output.matrix());
        }

        // Construct low and high clip regions
        auto sz0 = output.dimension(0);
        auto sz1 = output.dimension(1);
        primitive_argument_type lo_{small}, hi_{1-small};

        auto lo = extract_value_matrix<double>(lo_, sz0, sz1, name_, codename_ );
        auto hi = extract_value_matrix<double>(hi_, sz0, sz1, name_, codename_ );

        output.matrix() = blaze::max(
            blaze::min(output.matrix(), hi.matrix()), lo.matrix());

        auto res = target.matrix() % (-blaze::log(output.matrix()));
        return primitive_argument_type{ blaze::sum(res) };
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
     blaze::DynamicTensor<double> cat_cross_operation::softmax3d_axis2(
        arg_type& arg) const
    {
        auto t = arg.tensor();

        blaze::DynamicTensor<double> result(t.pages(), t.rows(), t.columns());
        for (std::size_t i = 0; i != t.pages(); ++i)
        {
            auto slice = blaze::pageslice(t, i);
            blaze::pageslice(result, i) = blaze::softmax<blaze::rowwise>(slice);
        }
        return result;
    }

    primitive_argument_type cat_cross_operation::cat_cross3d(
        arg_type&& target, arg_type&& output, bool from_logits) const
    {
        if(from_logits)
        {
            output.tensor() = softmax3d_axis2(output);
        }
        else
        {
            output.tensor() /= blaze::sum(output.tensor());
        }

        // Construct low and high clip regions
        auto sz0 = output.dimension(0);
        auto sz1 = output.dimension(1);
        auto sz2 = output.dimension(2);
        primitive_argument_type lo_{small}, hi_{1-small};

        auto lo = extract_value_tensor<double>(
            lo_, sz0, sz1, sz2, name_, codename_ );
        auto hi = extract_value_tensor<double>(
            hi_, sz0, sz1, sz2, name_, codename_ );

        output.tensor() = blaze::max(
            blaze::min(output.tensor(), hi.tensor()), lo.tensor());

        auto res = target.tensor() % (-blaze::log(output.tensor()));
        return primitive_argument_type{ blaze::sum(res) };
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> cat_cross_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        #if 0
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "cat_cross_operation::eval",
                util::generate_error_message(
                    "the cat_cross_operation primitive requires exactly one, or "
                    "two operands",
                    name_, codename_));
        }
        #endif

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cat_cross_operation::eval",
                    util::generate_error_message(
                        "the cat_cross_operation primitive requires that the "
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
                // Extract logits
                std::int64_t from_logits =
                    static_cast<bool>(false);

                // from_logits is the third argument
                if (args.size() > 2)
                {
                    if (valid(args[2]))
                        from_logits =
                            execution_tree::extract_scalar_boolean_value(
                                args[2], this_->name_, this_->codename_);
                }

                // Extract the matrix, the result should always be double
                arg_type target = extract_numeric_value(
                    std::move(args[0]), this_->name_, this_->codename_);
                arg_type output = extract_numeric_value(
                    std::move(args[1]), this_->name_, this_->codename_);

                std::size_t target_dims = target.num_dimensions();
                std::size_t output_dims = output.num_dimensions();
                HPX_ASSERT(target_dims == output_dims);

                switch (target_dims)
                {
                case 0:
                    return this_->cat_cross0d();
                case 1:
                    return this_->cat_cross1d(
                        std::move(target),std::move(output),from_logits);
                case 2:
                    return this_->cat_cross2d(
                        std::move(target),std::move(output),from_logits);
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->cat_cross3d(
                        std::move(target),std::move(output),from_logits);
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cat_cross_operation::eval",
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

