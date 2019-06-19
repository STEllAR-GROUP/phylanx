// Copyright (c) 2019 Stevn R. Brandt
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/categorical_crossentropy_operation.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>
#include <phylanx/plugins/statistics/sum_operation.hpp>
#include <phylanx/util/assign.hpp>
#include <phylanx/util/blaze_traits.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cat_cross_operation::match_data = {
        hpx::util::make_tuple("categorical_crossentropy",
            std::vector<std::string>{
                "categorical_crossentropy(_1_target,_2_output,"
                "__arg(_3_from_logits,false),__arg(_4_axis,-1))"},
            &create_cat_cross_operation, &create_primitive<cat_cross_operation>,
            R"(target, output, from_logits
            Args:

                target (array_like) : input array
                output (array_like) : this array is not output back to the caller
                                      but is passed back with the return values.
                from_logits (optional, boolean): boolean value, default = False
                axis (optional, integer: integer axis, default = -1

            Returns:
                The value should be the same as would be returned by the following
                Python function:

                def categorical_crossentropy(
                    target, output, from_logits=False, axis=True):

                    if from_logits:
                        axis = -1
                        outval = softmax(output, axis=axis)
                    else:
                        outval /= output.sum(axis=axis, keepdims=True)
                    outval = np.clip(output, 1e-7, 1 - 1e-7)
                    res = np.sum(target * -np.log(outval), axis=axis, keepdims=False)
                    return res, outval
           )")};

    constexpr double clip_low = 1e-7;
    constexpr double clip_high = 1 - clip_low;

    ///////////////////////////////////////////////////////////////////////////
    cat_cross_operation::cat_cross_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {
    }

    primitive_argument_type cat_cross_operation::cat_cross0d(
        arg_type&& target, arg_type&& output, bool from_logits) const
    {
        double v = 1;

        if (!from_logits)
            // usually 1, except when the output is zero
            v = output.scalar() / output.scalar();

        return primitive_argument_type{
            static_cast<double>(-target.scalar() * blaze::log(clip_high))};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type cat_cross_operation::cat_cross1d(
        arg_type&& target, arg_type&& output, bool from_logits) const
    {
        assign_vector<arg_type> output_(output);
        assign_vector<arg_type> target_(target);
        if (from_logits)
        {
            output_ = blaze::softmax(output.vector());
        }
        else
        {
            output_ = output.vector() / blaze::sum(output.vector());
        }

        target_ = blaze::map(
            target.vector(), output.vector(), [](double t_, double o_) {
                return -t_ *
                    std::log((std::min)(clip_high, (std::max)(clip_low, o_)));
            });

        double ans = blaze::sum(target.vector());
        primitive_argument_type part1(std::move(ans)), part2(std::move(output));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{std::move(tup)};
    }

    ///////////////////////////////////////////////////////////////////////////
    using matrix_type = blaze::DynamicMatrix<double>;
    using vector_type = blaze::DynamicVector<double>;
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    using tensor_type = blaze::DynamicTensor<double>;
#endif

    ///////////////////////////////////////////////////////////////////////////
    vector_type sum2d_axis1(const matrix_type& m)
    {
        vector_type out(m.rows());
        out = 0;
        for (std::size_t j = 0; j < m.columns(); ++j)
        {
            out += blaze::column(m, j);
        }
        return out;
    }

    vector_type sum2d_axis0(const matrix_type& m)
    {
        vector_type out(m.columns(), 0);
        for (std::size_t i = 0; i < m.rows(); ++i)
        {
            out += blaze::trans(blaze::row(m, i));
        }
        return out;
    }

    vector_type sum2d(const matrix_type& m, int axis)
    {
        if (axis == 0)
            return sum2d_axis0(m);
        else
            return sum2d_axis1(m);
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    matrix_type sum3d_axis2(const tensor_type& t)
    {
        matrix_type out(t.pages(), t.rows(), 0);
        for (std::size_t j = 0; j < t.columns(); ++j)
            out += blaze::columnslice(t, j);
        return out;
    }

    matrix_type sum3d_axis1(const tensor_type& t)
    {
        matrix_type out(t.columns(), t.pages(), 0);
        for (std::size_t j = 0; j < t.rows(); ++j)
        {
            auto slice = blaze::rowslice(t, j);
            out += slice;
        }
        return out;
    }

    matrix_type sum3d_axis0(const tensor_type& t)
    {
        matrix_type out(t.rows(), t.columns(), 0);
        for (std::size_t j = 0; j < t.pages(); ++j)
            out += blaze::pageslice(t, j);
        return out;
    }

    matrix_type sum3d(const tensor_type& t, int axis)
    {
        if (axis == 0)
            return sum3d_axis0(t);
        else if (axis == 1)
            return sum3d_axis1(t);
        else
            return sum3d_axis2(t);
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type cat_cross_operation::cat_cross2d(
        arg_type&& target, arg_type&& output, bool from_logits, int axis) const
    {
        assign_matrix<arg_type> output_(output);
        assign_matrix<arg_type> target_(target);
        if (from_logits)
        {
            output_ = blaze::softmax<blaze::rowwise>(output.matrix());
        }
        else
        {
            auto norm = sum2d(output.matrix(), axis);
            if (axis == 0)
            {
                for (std::size_t i = 0; i < output.matrix().rows(); ++i)
                {
                    auto slice = blaze::row(output.matrix(), i);
                    slice = blaze::map(slice, blaze::trans(norm),
                        [](double s_, double n_) { return s_ / n_; });
                }
            }
            else
            {
                for (std::size_t j = 0; j < output.matrix().columns(); ++j)
                {
                    auto slice = blaze::column(output.matrix(), j);
                    slice = blaze::map(slice, norm,
                        [](double s_, double n_) { return s_ / n_; });
                }
            }
        }

        target_ = blaze::map(
            target.matrix(), output.matrix(), [](double t_, double o_) {
                return -t_ *
                    std::log((std::min)(clip_high, (std::max)(clip_low, o_)));
            });
        vector_type ans = sum2d(target.matrix(), axis);
        primitive_argument_type part1(std::move(ans)), part2(std::move(output));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{std::move(tup)};
    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    ///////////////////////////////////////////////////////////////////////////
    blaze::DynamicTensor<double> softmax3d_axis2(const tensor_type& t)
    {
        blaze::DynamicTensor<double> result(t.pages(), t.rows(), t.columns());
        for (std::size_t i = 0; i != t.pages(); ++i)
        {
            auto slice = blaze::pageslice(t, i);
            blaze::pageslice(result, i) = blaze::softmax<blaze::rowwise>(slice);
        }
        return result;
    }

    primitive_argument_type cat_cross_operation::cat_cross3d(
        arg_type&& target, arg_type&& output, bool from_logits, int axis) const
    {
        assign_tensor<arg_type> output_(output);
        assign_tensor<arg_type> target_(target);
        if (from_logits)
        {
            output_ = softmax3d_axis2(output.tensor());
        }
        else
        {
            if (axis == 0)
            {
                auto norm = sum3d(output.tensor(), axis);
                for (std::size_t j = 0; j < output.tensor().pages(); ++j)
                {
                    auto slice = blaze::pageslice(output.tensor(), j);
                    slice = blaze::map(slice, norm,
                        [](double s_, double n_) { return s_ / n_; });
                }
            }
            else if (axis == 1)
            {
                auto norm = sum3d(output.tensor(), axis);
                for (std::size_t j = 0; j < output.tensor().rows(); ++j)
                {
                    auto slice = blaze::rowslice(output.tensor(), j);
                    slice = blaze::map(slice, norm,
                        [](double s_, double n_) { return s_ / n_; });
                }
            }
            else
            {
                auto norm = sum3d(output.tensor(), axis);
                for (std::size_t j = 0; j < output.tensor().columns(); ++j)
                {
                    auto slice = blaze::columnslice(output.tensor(), j);
                    slice = blaze::map(slice, norm,
                        [](double s_, double n_) { return s_ / n_; });
                }
            }
        }

        target_ = blaze::map(
            target.tensor(), output.tensor(), [](double t_, double o_) {
                return -t_ *
                    std::log((std::min)(clip_high, (std::max)(clip_low, o_)));
            });
        matrix_type ans = sum3d(target.tensor(), axis);
        if (axis == 1)
            ans = blaze::trans(ans);
        primitive_argument_type part1(std::move(ans)), part2(std::move(output));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{std::move(tup)};
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> cat_cross_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "cat_cross_operation::eval",
                    generate_error_message(
                        "the cat_cross_operation primitive requires that the "
                        "arguments given by the operands array are valid"));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](primitive_arguments_type&& args)
            -> primitive_argument_type
            {
                // Extract logits
                bool from_logits = false;

                // from_logits is the third argument
                if (args.size() > 2 && valid(args[2]))
                {
                    from_logits = extract_scalar_boolean_value(
                        std::move(args[2]), this_->name_, this_->codename_);
                }

                // Extract axis
                int axis = -1;
                if (!from_logits && args.size() > 3 && valid(args[3]))
                {
                    axis = extract_scalar_integer_value(
                        std::move(args[3]), this_->name_, this_->codename_);
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
                    return this_->cat_cross0d(
                        std::move(target), std::move(output), from_logits);

                case 1:
                    return this_->cat_cross1d(
                        std::move(target), std::move(output), from_logits);

                case 2:
                    return this_->cat_cross2d(std::move(target),
                        std::move(output), from_logits, axis);

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->cat_cross3d(std::move(target),
                        std::move(output), from_logits, axis);
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "cat_cross_operation::eval",
                        this_->generate_error_message(
                            "operand a has an invalid number of dimensions"));
                }
            }),
            detail::map_operands(operands, functional::value_operand{},
                args, name_, codename_, std::move(ctx)));
    }
}}}
