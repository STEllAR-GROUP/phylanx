// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/extract_shape.hpp>

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

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const extract_shape::match_data =
    {
        hpx::util::make_tuple("shape",
            std::vector<std::string>{"shape(_1, _2)", "shape(_1)"},
            &create_extract_shape, &create_primitive<extract_shape>,
            "m, dim\n"
            "Args:\n"
            "\n"
            "    m (object): a vector or matrix\n"
            "    dim (optional, int): the dimension to get the size of\n"
            "\n"
            "Returns:\n"
            "\n"
            "Without the optional argument, it returns a list of integers "
            "corresponding to the size of each dimension of the vector or "
            "matrix. If the optional `dim` argument is supplied, then the "
            "size for that dimension is returned as an integer."
            )
    };

    ///////////////////////////////////////////////////////////////////////////
    extract_shape::extract_shape(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type extract_shape::shape0d(arg_type&& arg) const
    {
        primitive_arguments_type result{};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape0d(arg_type&& arg,
        std::int64_t index) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::eval",
            generate_error_message(
                "index out of range"));
    }

    primitive_argument_type extract_shape::shape1d(arg_type&& arg) const
    {
        primitive_arguments_type result{
            primitive_argument_type{std::int64_t(arg.size())}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape1d(arg_type&& arg,
        std::int64_t index) const
    {
        if (index == 0)
        {
            return primitive_argument_type{std::int64_t(arg.size())};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::eval",
            generate_error_message(
                "index out of range"));
    }

    primitive_argument_type extract_shape::shape2d(arg_type&& arg) const
    {
        // return a list of numbers representing the
        // dimensions of the first argument
        auto dims = arg.dimensions();
        primitive_arguments_type result{
            primitive_argument_type{std::int64_t(dims[0])},
            primitive_argument_type{std::int64_t(dims[1])}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape2d(arg_type&& arg,
        std::int64_t index) const
    {
        if (index == 0 || index == 1)
        {
            auto dims = arg.dimensions();
            return primitive_argument_type{std::int64_t(dims[index])};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::eval",
            generate_error_message(
                "index out of range"));
    }

    hpx::future<primitive_argument_type> extract_shape::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::eval",
                generate_error_message(
                    "the extract_shape primitive requires one or two operands"));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::eval",
                generate_error_message(
                    "the extract_shape primitive requires that the "
                        "arguments given by the operands array are valid"));
        }

        auto this_ = this->shared_from_this();

        if (operands.size() == 1)
        {
            return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
                [this_ = std::move(this_)](arg_type && arg)
                -> primitive_argument_type
                {
                    auto dims = arg.num_dimensions();
                    switch (dims)
                    {
                    case 0:
                        return this_->shape0d(std::move(arg));

                    case 1:
                        return this_->shape1d(std::move(arg));

                    case 2:
                        return this_->shape2d(std::move(arg));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "extract_shape::eval",
                            this_->generate_error_message(
                                "first operand has unsupported "
                                    "number of dimensions"));
                    }
                }),
                numeric_operand(operands[0], args,
                    name_, codename_, std::move(ctx)));
        }

        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_ = std::move(this_)](arg_type && arg, std::int64_t index)
            ->  primitive_argument_type
            {
                auto dims = arg.num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->shape0d(std::move(arg), index);

                case 1:
                    return this_->shape1d(std::move(arg), index);

                case 2:
                    return this_->shape2d(std::move(arg), index);

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "extract_shape::eval",
                        this_->generate_error_message(
                            "first operand has unsupported "
                                "number of dimensions"));
                }
            }),
            numeric_operand(operands[0], args, name_, codename_, ctx),
            scalar_integer_operand_strict(operands[1], args,
                name_, codename_, ctx));
    }
}}}
