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
            &create_extract_shape, &create_primitive<extract_shape>, R"(
            m, dim
            Args:

                m (object): a vector or matrix
                dim (optional, int): the dimension to get the size of

            Returns:

            Without the optional argument, it returns a list of integers
            corresponding to the size of each dimension of the vector or
            matrix. If the optional `dim` argument is supplied, then the
            size for that dimension is returned as an integer.)")
    };

    ///////////////////////////////////////////////////////////////////////////
    extract_shape::extract_shape(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape0d() const
    {
        return primitive_argument_type{primitive_arguments_type{}};
    }

    primitive_argument_type extract_shape::shape0d(std::int64_t index) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::shape0d",
            generate_error_message("index out of range"));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape1d(std::int64_t size) const
    {
        primitive_arguments_type result{primitive_argument_type{size}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape1d(std::int64_t size,
        std::int64_t index) const
    {
        if (index == 0)
        {
            return primitive_argument_type{size};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::shape1d",
            generate_error_message("index out of range"));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape2d(
        std::int64_t rows, std::int64_t columns) const
    {
        // return a list of numbers representing the dimensions of the argument
        primitive_arguments_type result{
            primitive_argument_type{rows}, primitive_argument_type{columns}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape2d(
        std::int64_t rows, std::int64_t columns, std::int64_t index) const
    {
        if (index == 0)
        {
            return primitive_argument_type{rows};
        }
        if (index == 1)
        {
            return primitive_argument_type{columns};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::shape2d",
            generate_error_message("index out of range"));
    }

    ///////////////////////////////////////////////////////////////////////////
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
    primitive_argument_type extract_shape::shape3d(
        std::int64_t pages, std::int64_t rows, std::int64_t columns) const
    {
        // return a list of numbers representing the dimensions of the argument
        primitive_arguments_type result{primitive_argument_type{pages},
            primitive_argument_type{rows}, primitive_argument_type{columns}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape3d(std::int64_t pages,
        std::int64_t rows, std::int64_t columns, std::int64_t index) const
    {
        if (index == 0)
        {
            return primitive_argument_type{pages};
        }
        if (index == 1)
        {
            return primitive_argument_type{rows};
        }
        if (index == 2)
        {
            return primitive_argument_type{columns};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::shape3d",
            generate_error_message("index out of range"));
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
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
            return hpx::dataflow(hpx::launch::sync,
                [this_ = std::move(this_)](
                        hpx::future<primitive_argument_type>&& f)
                -> primitive_argument_type
                {
                    auto&& arg = f.get();

                    switch (extract_numeric_value_dimension(
                        arg, this_->name_, this_->codename_))
                    {
                    case 0:
                        return this_->shape0d();

                    case 1:
                        return this_->shape1d(extract_numeric_value_size(
                            arg, this_->name_, this_->codename_));

                    case 2:
                        {
                            auto dims = extract_numeric_value_dimensions(
                                arg, this_->name_, this_->codename_);
                            return this_->shape2d(dims[0], dims[1]);
                        }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                    case 3:
                        {
                            auto dims = extract_numeric_value_dimensions(
                                arg, this_->name_, this_->codename_);
                            return this_->shape3d(dims[0], dims[1], dims[2]);
                        }
#endif
                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "extract_shape::eval",
                            this_->generate_error_message(
                                "first operand has unsupported "
                                    "number of dimensions"));
                    }
                },
                value_operand(operands[0], args, name_, codename_,
                    std::move(ctx)));
        }

        auto&& op0 = value_operand(operands[0], args, name_, codename_, ctx);

        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& f1,
                    hpx::future<std::int64_t>&& f2)
            ->  primitive_argument_type
            {
                auto&& arg = f1.get();
                auto&& index = f2.get();

                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->shape0d(index);

                case 1:
                    return this_->shape1d(
                        extract_numeric_value_size(
                            arg, this_->name_, this_->codename_),
                        index);

                case 2:
                    {
                        auto dims = extract_numeric_value_dimensions(
                            arg, this_->name_, this_->codename_);
                        return this_->shape2d(dims[0], dims[1], index);
                    }

#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
                case 3:
                    {
                        auto dims = extract_numeric_value_dimensions(
                            arg, this_->name_, this_->codename_);
                        return this_->shape3d(dims[0], dims[1], dims[2], index);
                    }
#endif
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "extract_shape::eval",
                        this_->generate_error_message(
                            "first operand has unsupported "
                                "number of dimensions"));
                }
            },
            std::move(op0),
            scalar_integer_operand_strict(operands[1], args,
                name_, codename_, std::move(ctx)));
    }
}}}
