// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/batch_normalization_operation.hpp>

#include <hpx/errors/throw_exception.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#include <blaze_tensor/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives {
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const batch_normalization_operation::match_data =
    {
        hpx::util::make_tuple("batch_normalization",
        std::vector<std::string>{R"(
            batch_normalization(_1,
            _2_mean,
            _3_var,
            __arg(_4_beta, 0.0),
            __arg(_5_gamma, 1.0),
            __arg(_6_axis, -1),
            __arg(_7_epsilon, 1e-3))
        )"},
        &create_batch_normalization_operation,
        &create_primitive<batch_normalization_operation>,
        R"(x, mean, var, beta, gamma, axis, epsilon
        Args:

            x (array_like): input 4d array
            mean (array_like): mean of batch
            var (array_like): variance of batch
            beta (array_like): tensor by which to center the input. It is set
                to 0.0 by default.
            gamma (array_like): tensor by which to scale the input. It is set
                to 1.0 by default.
            axis (an integer): the axis that should be normalized
            epsilon (a float): the fuzz factor. It is set to 0.001 by default

        Returns:

        Applies batch normalization on the given array)")
    };

    ///////////////////////////////////////////////////////////////////////////
    batch_normalization_operation::batch_normalization_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    //primitive_argument_type batch_normalization_operation::spatial_padding(
    //    ir::node_data<double>&& arg) const
    //{
    //    auto q = arg.quatern();
    //    std::size_t quats = q.quats();
    //    std::size_t pages = q.pages();
    //    std::size_t rows  = q.rows();
    //    std::size_t columns = q.columns();
    //    blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
    //        quats, pages + 2UL, rows + 2UL, columns);

    //    for (std::size_t l = 0; l != quats; ++l)
    //    {
    //        auto rtensor = blaze::quatslice(result, l);
    //        auto qtensor = blaze::quatslice(q, l);
    //        blaze::subtensor(rtensor, 1UL, 1UL, 0UL, pages, rows, columns) =
    //            qtensor;
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    /////////////////////////////////////////////////////////////////////////////
    //primitive_argument_type batch_normalization_operation::spatial_padding(
    //    ir::node_data<double>&& arg, std::size_t pad_front,
    //    std::size_t pad_rear, std::size_t pad_top, std::size_t pad_bottom) const
    //{
    //    auto q = arg.quatern();
    //    std::size_t quats = q.quats();
    //    std::size_t pages = q.pages();
    //    std::size_t rows  = q.rows();
    //    std::size_t columns = q.columns();
    //    blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
    //        quats, pages + pad_front + pad_rear, rows + pad_top + pad_bottom,
    //        columns);

    //    for (std::size_t l = 0; l != quats; ++l)
    //    {
    //        auto rtensor = blaze::quatslice(result, l);
    //        auto qtensor = blaze::quatslice(q, l);
    //        blaze::subtensor(rtensor, pad_front, pad_top, 0UL, pages, rows,
    //            columns) = qtensor;
    //    }
    //    return primitive_argument_type{std::move(result)};
    //}

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> batch_normalization_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.size() < 3 || operands.size() > 7)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "batch_normalization_operation::eval",
                util::generate_error_message(
                    "the batch_normalization "
                    "primitive requires at least 3 and at most 7 operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "batch_normalization_operation::eval",
                    util::generate_error_message(
                        "the batch_normalization primitive requires "
                        "that the arguments given by the operands array are "
                        "valid",
                        name_, codename_));
            }
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync,
            hpx::util::unwrapping([this_ = std::move(this_)](
                                      primitive_arguments_type&& args)
                                      -> primitive_argument_type {

                //ir::range padding(0);
                //std::int64_t pad_front, pad_rear, pad_top, pad_bottom;
                //if (args.size() > 1)
                //{
                //    padding = extract_list_value_strict(
                //        args[1], this_->name_, this_->codename_);
                //    if (padding.size() != 2)
                //    {
                //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                //            "batch_normalization_operation::eval",
                //            this_->generate_error_message(
                //                "batch_normalization requires padding to be a "
                //                "tuple of 2 tuples"));
                //    }
                //    auto it = padding.begin();
                //    auto it_pages = extract_list_value_strict(
                //        *it, this_->name_, this_->codename_);
                //    auto it_rows = extract_list_value_strict(
                //        *++it, this_->name_, this_->codename_);
                //    if (it_pages.size() != 2 || it_rows.size() != 2)
                //    {
                //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                //            "batch_normalization_operation::eval",
                //            this_->generate_error_message(
                //                "batch_normalization requires padding to be a "
                //                "tuple of 2 tuples, each contains two "
                //                "non-negative integers"));
                //    }
                //    pad_front =
                //        extract_scalar_integer_value_strict(*it_pages.begin());
                //    pad_rear = extract_scalar_integer_value_strict(
                //        *++it_pages.begin());
                //    pad_top =
                //        extract_scalar_integer_value_strict(*it_rows.begin());
                //    pad_bottom =
                //        extract_scalar_integer_value_strict(*++it_rows.begin());

                //    if (pad_front < 0 || pad_rear < 0 || pad_top < 0 ||
                //        pad_bottom < 0)
                //    {
                //        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                //            "batch_normalization_operation::eval",
                //            this_->generate_error_message(
                //                "batch_normalization requires each pad to be a "
                //                "non-negative integer"));
                //    }

                //    if (pad_front == 1 && pad_rear == 1 && pad_top == 1 &&
                //        pad_bottom == 1)
                //    {
                //        padding = ir::range(0);
                //    }
                //}

                //if (padding.empty()) // all paddings are set to 1
                //{
                //    return this_->spatial_padding(extract_numeric_value(
                //        std::move(args[0]), this_->name_, this_->codename_));
                //}
                //return this_->spatial_padding(
                //    extract_numeric_value(
                //        std::move(args[0]), this_->name_, this_->codename_),
                //    pad_front, pad_rear, pad_top, pad_bottom);
                                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "batch_normalization_operation::eval",
                            this_->generate_error_message(
                                "batch_normalization requires padding to be a "
                                "tuple of 2 tuples"));
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
