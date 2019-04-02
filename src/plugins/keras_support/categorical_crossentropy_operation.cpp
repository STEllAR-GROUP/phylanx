// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/keras_support/categorical_crossentropy_operation.hpp>
#include <phylanx/util/blaze_traits.hpp>
#include <phylanx/plugins/statistics/statistics_base.hpp>
#include <phylanx/plugins/statistics/sum_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/assert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const cat_cross_operation::match_data =
    {
        hpx::util::make_tuple("categorical_crossentropy",
        std::vector<std::string>{
            "categorical_crossentropy(_1_target,_2_output,"
                "__arg(_3_from_logits,false),__arg(_4_axis,-1))"
        },
        &create_cat_cross_operation, &create_primitive<cat_cross_operation>,
        R"(target, output, from_logits
   Args:

       target (array_like) : input array
       output (array_like) : output array
       from_logits (optional, boolean): boolean value, default = False
       axis (optional, integer: integer axis, default = -1

   Returns:
       The value should be the same as would be returned by the following
       Python function:

      def categorical_crossentropy(target, output, from_logits=False, axis=True):
           if from_logits:
               axis = -1
               output = softmax(output, axis=axis)
           else:
               output /= output.sum(axis=axis, keepdims=True)
           output = np.clip(output, 1e-7, 1 - 1e-7)
           return np.sum(target * -np.log(output), axis=axis, keepdims=False)
   )")
    };
    constexpr double clip_low = 1e-7;
    constexpr double clip_high = 1 - clip_low;

    ///////////////////////////////////////////////////////////////////////////
    cat_cross_operation::cat_cross_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type cat_cross_operation::cat_cross0d(
        arg_type&& target,arg_type&& output,bool from_logits) const
    {
        double v = 1;

        if(!from_logits)
            // usually 1, except when the output is zero
            v = output.scalar()/output.scalar();

        return primitive_argument_type{
            static_cast<double>(-target.scalar()*blaze::log(clip_high))};
    }

    ///////////////////////////////////////////////////////////////////////////
    using matrix_type =
         blaze::DynamicMatrix<double>;

    using vector_type =
         blaze::DynamicVector<double>;

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    using tensor_type =
         blaze::DynamicTensor<double>;
#endif

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type cat_cross_operation::cat_cross1d(
        arg_type&& target,arg_type&& output,bool from_logits) const
    {
        vector_type&& output_ = output.vector();
        vector_type&& target_ = target.vector();
        if(from_logits)
        {
            output_ = blaze::softmax(output_);
        }
        else
        {
            output_ /= blaze::sum(output_);
        }

        target_ = blaze::map(target_, output_,[](double t_, double o_){
            return -t_*std::log((std::min)(clip_high,(std::max)(clip_low,o_)));
        });

        double ans = blaze::sum(target_);
        output.vector() = std::move(output_);
        return primitive_argument_type{ ans };
    }

    ///////////////////////////////////////////////////////////////////////////
    vector_type sum2d_axis1(matrix_type& m) {
       vector_type out(m.rows());
       out = 0;
       for(std::size_t j=0; j < m.columns(); ++j) {
          out += blaze::column(m, j);
       }
       return out;
    }

    vector_type sum2d_axis0(matrix_type& m) {
       vector_type out(m.columns());
       out = 0;
       for(std::size_t i=0; i < m.rows(); ++i) {
          out += blaze::trans(blaze::row(m, i));
       }
       return out;
    }
    vector_type sum2d(matrix_type& m,int axis) {
        if(axis == 0)
            return sum2d_axis0(m);
        else
            return sum2d_axis1(m);
    }


    ///////////////////////////////////////////////////////////////////////////
    matrix_type sum3d_axis2(tensor_type& t) {
       matrix_type out(t.pages(),t.rows());
       out = 0;
       for(std::size_t j = 0; j < t.columns(); ++j)
          out += blaze::columnslice(t, j);
       return out;
    }
    matrix_type sum3d_axis1(tensor_type& t) {
       matrix_type out(t.columns(),t.pages());
       out = 0;
       for(std::size_t j = 0; j < t.rows(); ++j) {
          auto slice = blaze::rowslice(t, j);
          out += slice;
       }
       return out;
    }
    matrix_type sum3d_axis0(tensor_type& t) {
       matrix_type out(t.rows(),t.columns());
       out = 0;
       for(std::size_t j = 0; j < t.pages(); ++j)
          out += blaze::pageslice(t, j);
       return out;
    }
    matrix_type sum3d(tensor_type& t,int axis) {
        if(axis == 0)
            return sum3d_axis0(t);
        else if(axis == 1)
            return sum3d_axis1(t);
        else
            return sum3d_axis2(t);
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type cat_cross_operation::cat_cross2d(
        arg_type&& target, arg_type&& output,bool from_logits,int axis) const
    {
        matrix_type&& output_ = output.matrix();
        matrix_type&& target_ = target.matrix();
        if(from_logits)
        {
            output_ = blaze::softmax<blaze::rowwise>(output_);
        }
        else
        {
            auto norm = sum2d(output_,axis);
            if(axis == 0) {
                for(std::size_t i = 0; i < output_.rows(); ++i) {
                    auto slice = blaze::row(output_,i);
                    slice = blaze::map(slice, blaze::trans(norm),[](double s_,double n_){
                        return s_/n_;
                    });
                }
            } else {
                for(std::size_t j = 0; j < output_.columns(); ++j) {
                    auto slice = blaze::column(output_,j);
                    slice = blaze::map(slice, norm,[](double s_,double n_){
                        return s_/n_;
                    });
                }
            }
        }

        target_ = blaze::map(target_, output_,[](double t_, double o_){
            return -t_*std::log((std::min)(clip_high,(std::max)(clip_low,o_)));
        });
        vector_type ans = sum2d(target_,axis);
        output.matrix() = std::move(output_);
        return primitive_argument_type{ ans };
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
        arg_type&& target, arg_type&& output, bool from_logits, int axis) const
    {
        tensor_type&& output_ = output.tensor();
        tensor_type&& target_ = target.tensor();
        if(from_logits)
        {
            output_ = softmax3d_axis2(output);
        }
        else
        {
            if(axis == 0) {
                auto norm = sum3d(output_,axis);
                for(std::size_t j = 0; j < output_.pages(); ++j) {
                    auto slice = blaze::pageslice(output_,j);
                    slice = blaze::map(slice, norm,[](double s_,double n_){
                        return s_/n_;
                    });
                }
            } else if(axis == 1) {
                auto norm = sum3d(output_,axis);
                for(std::size_t j = 0; j < output_.rows(); ++j) {
                    auto slice = blaze::rowslice(output_,j);
                    slice = blaze::map(slice, norm,[](double s_,double n_){
                        return s_/n_;
                    });
                }
            } else {
                auto norm = sum3d(output_,axis);
                for(std::size_t j = 0; j < output_.columns(); ++j) {
                    auto slice = blaze::columnslice(output_,j);
                    slice = blaze::map(slice, norm,[](double s_,double n_){
                        return s_/n_;
                    });
                }
            }
        }

        target_ = blaze::map(target_, output_,[](double t_, double o_){
            return -t_*std::log((std::min)(clip_high,(std::max)(clip_low,o_)));
        });
        matrix_type ans = sum3d(target_,axis);
        if(axis == 1)
            ans = blaze::trans(ans);
        output.tensor() = std::move(output_);
        return primitive_argument_type{ ans };
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> cat_cross_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
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
                bool from_logits =
                    static_cast<bool>(false);

                // from_logits is the third argument
                if (args.size() > 2)
                {
                    if (valid(args[2]))
                        from_logits =
                            execution_tree::extract_scalar_boolean_value(
                                args[2], this_->name_, this_->codename_);
                }

                // Extract axis
                int axis = -1;
                if(!from_logits && args.size() > 3)
                {
                    if (valid(args[3]))
                        axis =
                            execution_tree::extract_scalar_integer_value(
                                args[3], this_->name_, this_->codename_);
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
                        std::move(target),std::move(output),from_logits);
                case 1:
                    return this_->cat_cross1d(
                        std::move(target),std::move(output),from_logits);
                case 2:
                    return this_->cat_cross2d(
                        std::move(target),std::move(output),from_logits,axis);
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    return this_->cat_cross3d(
                        std::move(target),std::move(output),from_logits,axis);
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

