// Copyright (c) 2019 Bita Hasheminezhad
// Copyright (c) 2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/node_data_helpers.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/keras_support/spatial_2d_padding_operation.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>

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
    match_pattern_type const spatial_2d_padding_operation::match_data =
    {
        hpx::util::make_tuple("spatial_2d_padding",
        std::vector<std::string>{R"(
            spatial_2d_padding(_1,
            __arg(_2_padding, list(list(1,1),list(1,1))))
        )"},
        &create_spatial_2d_padding_operation,
        &create_primitive<spatial_2d_padding_operation>,
        R"(x, padding
        Args:

            x (array_like) : input 4d array
            padding (optional, a tuple of two tuples): the first tuple contains
                two integers which indicate number of pages to pad on front and
                rear respectively. The second tuple also contains two integers
                that indicate number of rows to pad on the top and bottom of
                the data. The default is ((1,1),(1,1)).

        Returns:

        Pads the second and third dimensions of a 4d array with zeros)")
    };

    ///////////////////////////////////////////////////////////////////////////
    spatial_2d_padding_operation::spatial_2d_padding_operation(
        primitive_arguments_type&& operands, std::string const& name,
        std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type spatial_2d_padding_operation::spatial_padding(
        ir::node_data<double>&& arg) const
    {
        auto q = arg.quatern();
        std::size_t quats = q.quats();
        std::size_t pages = q.pages();
        std::size_t rows  = q.rows();
        std::size_t columns = q.columns();
        blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
            quats, pages + 2UL, rows + 2UL, columns);

        for (std::size_t l = 0; l != quats; ++l)
        {
            auto rtensor = blaze::quatslice(result, l);
            auto qtensor = blaze::quatslice(q, l);
            blaze::subtensor(rtensor, 1UL, 1UL, 0UL, pages, rows, columns) =
                qtensor;
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type spatial_2d_padding_operation::spatial_padding(
        ir::node_data<double>&& arg, std::size_t pad_front,
        std::size_t pad_rear, std::size_t pad_top, std::size_t pad_bottom) const
    {
        auto q = arg.quatern();
        std::size_t quats = q.quats();
        std::size_t pages = q.pages();
        std::size_t rows  = q.rows();
        std::size_t columns = q.columns();
        blaze::DynamicArray<4UL, double> result(blaze::init_from_value, 0.0,
            quats, pages + pad_front + pad_rear, rows + pad_top + pad_bottom,
            columns);

        for (std::size_t l = 0; l != quats; ++l)
        {
            auto rtensor = blaze::quatslice(result, l);
            auto qtensor = blaze::quatslice(q, l);
            blaze::subtensor(rtensor, pad_front, pad_top, 0UL, pages, rows,
                columns) = qtensor;
        }
        return primitive_argument_type{std::move(result)};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> spatial_2d_padding_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "spatial_2d_padding_operation::eval",
                util::generate_error_message("the spatial_2d_padding "
                                             "primitive requires one, or "
                                             "two operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "spatial_2d_padding_operation::eval",
                    util::generate_error_message(
                        "the spatial_2d_padding primitive requires "
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

                ir::range padding(0);
                std::int64_t pad_front, pad_rear, pad_top, pad_bottom;
                if (args.size() > 1)
                {
                    padding = extract_list_value_strict(
                        args[1], this_->name_, this_->codename_);
                    if (padding.size() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "spatial_2d_padding_operation::eval",
                            this_->generate_error_message(
                                "spatial_2d_padding requires padding to be a "
                                "tuple of 2 tuples"));
                    }
                    auto it = padding.begin();
                    auto it_pages = extract_list_value_strict(
                        *it, this_->name_, this_->codename_);
                    auto it_rows = extract_list_value_strict(
                        *++it, this_->name_, this_->codename_);
                    if (it_pages.size() != 2 || it_rows.size() != 2)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "spatial_2d_padding_operation::eval",
                            this_->generate_error_message(
                                "spatial_2d_padding requires padding to be a "
                                "tuple of 2 tuples, each contains two "
                                "non-negative integers"));
                    }
                    pad_front =
                        extract_scalar_integer_value_strict(*it_pages.begin());
                    pad_rear = extract_scalar_integer_value_strict(
                        *++it_pages.begin());
                    pad_top =
                        extract_scalar_integer_value_strict(*it_rows.begin());
                    pad_bottom =
                        extract_scalar_integer_value_strict(*++it_rows.begin());

                    if (pad_front < 0 || pad_rear < 0 || pad_top < 0 ||
                        pad_bottom < 0)
                    {
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "spatial_2d_padding_operation::eval",
                            this_->generate_error_message(
                                "spatial_2d_padding requires each pad to be a "
                                "non-negative integer"));
                    }

                    if (pad_front == 1 && pad_rear == 1 && pad_top == 1 &&
                        pad_bottom == 1)
                    {
                        padding = ir::range(0);
                    }
                }

                if (padding.empty()) // all paddings are set to 1
                {
                    return this_->spatial_padding(extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_));
                }
                return this_->spatial_padding(
                    extract_numeric_value(
                        std::move(args[0]), this_->name_, this_->codename_),
                    pad_front, pad_rear, pad_top, pad_bottom);
            }),
            detail::map_operands(operands, functional::value_operand{}, args,
                name_, codename_, std::move(ctx)));
    }
}}}
