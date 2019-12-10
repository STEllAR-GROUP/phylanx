// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/plugins/keras_support/binary_crossentropy_operation.hpp>
#include <phylanx/util/blaze_traits.hpp>
#include <phylanx/util/assign.hpp>

#include <hpx/assertion.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <type_traits>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const bin_cross_operation::match_data =
    {
        hpx::util::make_tuple("binary_crossentropy",
        std::vector<std::string>{
            "binary_crossentropy(_1_target,_2_output,"
                "__arg(_3_from_logits,false))"
        },
        &create_bin_cross_operation, &create_primitive<bin_cross_operation>,
        R"(target, output, from_logits
   Args:

       target (array_like) : input array
       output (array_like) : this array is not output back to the caller
                             but is passed back with the return values.
       from_logits (optional, boolean): boolean value, default = False

   Returns:
       The value should be the same as would be returned by the following
       Python function:

    def bin_cross(target, output, from_logits=False):
        if not from_logits:
            output = np.clip(output, 1e-7, 1 - 1e-7)
            output = np.log(output / (1 - output))
        return (target * -np.log(sigmoid(output)) +
                (1 - target) * -np.log(1 - sigmoid(output)))
   )")
    };
    constexpr double clip_low = 1e-7;
    constexpr double clip_high = 1 - clip_low;

    ///////////////////////////////////////////////////////////////////////////
    bin_cross_operation::bin_cross_operation(primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type bin_cross_operation::bin_cross0d(
        arg_type&& target,arg_type&& output,bool from_logits) const
    {
        double output_ = output.scalar();
        double target_ = target.scalar();
        if(!from_logits) {
            double tmp = (std::min)(clip_high,(std::max)(clip_low,output_));
            output_ = std::log(tmp/(1-tmp));
        }
        double sig = 1/(1+exp(-output_));
        target_ = -target_*std::log(sig) - (1-target_)*std::log(1-sig);
        primitive_argument_type part1(std::move(target_)), part2(std::move(output_));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{ std::move(tup) };
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type bin_cross_operation::bin_cross1d(
        arg_type&& target,arg_type&& output,bool from_logits) const
    {
        assign_vector<arg_type> output_(output);
        assign_vector<arg_type> target_(target);
        if(!from_logits) {
            output_ = blaze::map(output.vector(),[](double o_){
                return (std::min)(clip_high,(std::max)(clip_low,o_));
            });
            target_ = blaze::map(target.vector(), output.vector(),
                    [](double t_,double o_) {
                return -t_*std::log(o_+clip_low) - (1-t_)*std::log(1 - o_ + clip_low);
            });
        } else {
            target_ = blaze::map(target.vector(), output.vector(),
                    [](double t_,double o_){
                double sig = 1/(1+exp(-o_));
                return -t_*std::log(sig) - (1-t_)*std::log(1-sig);
            });
        }
        primitive_argument_type part1(std::move(target)), part2(std::move(output));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{ std::move(tup) };
    }

    ///////////////////////////////////////////////////////////////////////////
    using matrix_type = blaze::DynamicMatrix<double>;

    using vector_type = blaze::DynamicVector<double>;

    using tensor_type = blaze::DynamicTensor<double>;

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type bin_cross_operation::bin_cross2d(
        arg_type&& target, arg_type&& output,bool from_logits) const
    {
        assign_matrix<arg_type> output_(output);
        assign_matrix<arg_type> target_(target);
        if(!from_logits) {
            output_ = blaze::map(output.matrix(),[](double o_){
                return (std::min)(clip_high,(std::max)(clip_low,o_));
            });
            target_ = blaze::map(target.matrix(), output.matrix(),
                    [](double t_,double o_) {
                return -t_*std::log(o_+clip_low) - (1-t_)*std::log(1 - o_ + clip_low);
            });
        } else {
            target_ = blaze::map(target.matrix(), output.matrix(),
                    [](double t_,double o_){
                double sig = 1/(1+exp(-o_));
                return -t_*std::log(sig) - (1-t_)*std::log(1-sig);
            });
        }
        primitive_argument_type part1(std::move(target)), part2(std::move(output));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{ std::move(tup) };
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type bin_cross_operation::bin_cross3d(
        arg_type&& target, arg_type&& output, bool from_logits) const
    {
        assign_tensor<arg_type> output_(output);
        assign_tensor<arg_type> target_(target);
        if(!from_logits) {
            output_ = blaze::map(output.tensor(),[](double o_){
                return (std::min)(clip_high,(std::max)(clip_low,o_));
            });
            target_ = blaze::map(target.tensor(), output.tensor(),
                    [](double t_,double o_) {
                return -t_*std::log(o_+clip_low) - (1-t_)*std::log(1 - o_ + clip_low);
            });
        } else {
            target_ = blaze::map(target.tensor(), output.tensor(),
                    [](double t_,double o_){
                double sig = 1/(1+exp(-o_));
                return -t_*std::log(sig) - (1-t_)*std::log(1-sig);
            });
        }
        primitive_argument_type part1(std::move(target)), part2(std::move(output));
        primitive_arguments_type both{part1, part2};
        phylanx::ir::range tup(both);
        return primitive_argument_type{ std::move(tup) };
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> bin_cross_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "bin_cross_operation::eval",
                    util::generate_error_message(
                        "the bin_cross_operation primitive requires that the "
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
                    return this_->bin_cross0d(
                        std::move(target),std::move(output),from_logits);

                case 1:
                    return this_->bin_cross1d(
                        std::move(target),std::move(output),from_logits);

                case 2:
                    return this_->bin_cross2d(
                        std::move(target),std::move(output),from_logits);

                case 3:
                    return this_->bin_cross3d(
                        std::move(target),std::move(output),from_logits);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "bin_cross_operation::eval",
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

