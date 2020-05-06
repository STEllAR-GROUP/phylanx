// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/localities_annotation.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/extract_shape.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/errors/throw_exception.hpp>

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
    std::vector<match_pattern_type> const extract_shape::match_data =
    {
        match_pattern_type{"shape",
            std::vector<std::string>{"shape(_1, _2)", "shape(_1)"},
            &create_extract_shape, &create_primitive<extract_shape>, R"(
            a, dim
            Args:

                a (object): an array of arbitrary dimensions
                dim (optional, int): the dimension to get the size of

            Returns:

            Without the optional argument, it returns a list of integers
            corresponding to the size of each dimension of the array `a`. If the
            optional `dim` argument is supplied, then the size for that
            dimension is returned as an integer. If the supplied array is
            distributed, the shape is calculated on the current locality.)"},

        match_pattern_type{"__len",
            std::vector<std::string>{"__len(_1)"},
            &create_extract_shape, &create_primitive<extract_shape>, R"(
            a
            Args:

                a (object): an array of arbitrary dimensions

            Returns:

            This returns the number of elements of the given array along its
            outermost (leftmost) dimension.)"},

        match_pattern_type{"shape_d",
            std::vector<std::string>{"shape_d(_1, _2)", "shape_d(_1)"},
            &create_extract_shape, &create_primitive<extract_shape>, R"(
            a, dim
            Args:

                a (object): a distributed array of arbitrary dimensions
                dim (optional, int): the dimension to get the size of

            Returns:

            Without the optional argument, it returns a list of integers
            corresponding to the size of each dimension of the array `a` on all
            localities. If the optional `dim` argument is supplied, then the
            size for that dimension of the whole array is returned as an
            integer.)"}
    };

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        extract_shape::shape_mode extract_shape_mode(std::string const& name)
        {
            extract_shape::shape_mode result = extract_shape::local_mode;
            if (name.find("__len") != std::string::npos)
            {
                result = extract_shape::len_mode;
            }
            else if (name.find("shape_d") != std::string::npos)
            {
                result = extract_shape::dist_mode;
            }
            return result;
        }
    }

    extract_shape::extract_shape(
            primitive_arguments_type && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
      , mode_(detail::extract_shape_mode(name_))
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape0d(
        primitive_argument_type&& arg) const
    {
        return primitive_argument_type{primitive_arguments_type{}};
    }

    primitive_argument_type extract_shape::shape0d(
        primitive_argument_type&& arg, std::int64_t index) const
    {
        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::shape0d",
            generate_error_message("index out of range"));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape1d(
        primitive_argument_type&& arg) const
    {
        std::int64_t size = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            size = localities.size();
        }
        else
        {
            size = extract_numeric_value_size(arg, name_, codename_);
        }
        primitive_arguments_type result{primitive_argument_type{size}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape1d(
        primitive_argument_type&& arg, std::int64_t index) const
    {
        if (index != 0 && index != -1)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::shape1d",
                generate_error_message("index out of range"));
        }

        std::int64_t size = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            size = localities.size();
        }
        else
        {
            size = extract_numeric_value_size(arg, name_, codename_);
        }
        return primitive_argument_type{size};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape2d(
        primitive_argument_type&& arg) const
    {
        // return a list of numbers representing the dimensions of the argument
        std::int64_t rows = 0;
        std::int64_t columns = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            rows = localities.rows();
            columns = localities.columns();
        }
        else
        {
            auto dims = extract_numeric_value_dimensions(arg, name_, codename_);
            rows = dims[0];
            columns = dims[1];
        }

        primitive_arguments_type result{
            primitive_argument_type{rows}, primitive_argument_type{columns}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape2d(
        primitive_argument_type&& arg, std::int64_t index) const
    {
        if (index < -2 || index >= 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::shape2d",
                generate_error_message("index out of range"));
        }

        if (index < 0)
            index += 2;

        std::int64_t size = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            size = (index == 0) ? localities.rows() : localities.columns();
        }
        else
        {
            auto dims = extract_numeric_value_dimensions(arg, name_, codename_);
            size = dims[index];
        }
        return primitive_argument_type{size};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape3d(
        primitive_argument_type&& arg) const
    {
        // return a list of numbers representing the dimensions of the argument
        std::int64_t pages = 0;
        std::int64_t rows = 0;
        std::int64_t columns = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            pages = localities.pages();
            rows = localities.rows();
            columns = localities.columns();
        }
        else
        {
            auto dims = extract_numeric_value_dimensions(arg, name_, codename_);
            pages = dims[0];
            rows = dims[1];
            columns = dims[2];
        }

        primitive_arguments_type result{primitive_argument_type{pages},
            primitive_argument_type{rows}, primitive_argument_type{columns}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape3d(
        primitive_argument_type&& arg, std::int64_t index) const
    {
        if (index < -3 || index >= 3)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::shape3d",
                generate_error_message("index out of range"));
        }

        if (index < 0)
            index += 3;

        std::int64_t size = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            size = localities.dimensions(name_, codename_)[index];
        }
        else
        {
            auto dims = extract_numeric_value_dimensions(arg, name_, codename_);
            size = dims[index];
        }
        return primitive_argument_type{size};
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type extract_shape::shape4d(
        primitive_argument_type&& arg) const
    {
        // return a list of numbers representing the dimensions of the argument
        std::int64_t quats = 0;
        std::int64_t pages = 0;
        std::int64_t rows = 0;
        std::int64_t columns = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            quats = localities.quats();
            pages = localities.pages();
            rows = localities.rows();
            columns = localities.columns();
        }
        else
        {
            auto dims = extract_numeric_value_dimensions(arg, name_, codename_);
            quats = dims[0];
            pages = dims[1];
            rows = dims[2];
            columns = dims[3];
        }

        primitive_arguments_type result{primitive_argument_type{quats},
            primitive_argument_type{pages}, primitive_argument_type{rows},
            primitive_argument_type{columns}};
        return primitive_argument_type{std::move(result)};
    }

    primitive_argument_type extract_shape::shape4d(
        primitive_argument_type&& arg, std::int64_t index) const
    {
        if (index < -4 || index >= 4)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::shape4d",
                generate_error_message("index out of range"));
        }

        if (index < 0)
            index += 4;

        std::int64_t size = 0;
        if (mode_ == dist_mode && arg.has_annotation())
        {
            localities_information localities =
                extract_localities_information(arg, name_, codename_);
            size = localities.dimensions(name_, codename_)[index];
        }
        else
        {
            auto dims = extract_numeric_value_dimensions(arg, name_, codename_);
            size = dims[index];
        }
        return primitive_argument_type{size};
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> extract_shape::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args, eval_context ctx) const
    {
        if (operands.empty() || ((operands.size() > 1 && mode_ == len_mode)) ||
            (operands.size() > 2 && mode_ != len_mode))
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
                        hpx::future<primitive_argument_type> && farg)
                -> primitive_argument_type
                {
                    auto&& arg = farg.get();

                    std::size_t numdims = extract_numeric_value_dimension(
                        arg, this_->name_, this_->codename_);
                    if (this_->mode_ == len_mode)
                    {
                        switch (numdims)
                        {
                        case 0:
                            return this_->shape0d(std::move(arg), 0);

                        case 1:
                            return this_->shape1d(std::move(arg), 0);

                        case 2:
                            return this_->shape2d(std::move(arg), 0);

                        case 3:
                            return this_->shape3d(std::move(arg), 0);

                        case 4:
                            return this_->shape4d(std::move(arg), 0);

                        default:
                            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                                "extract_shape::eval",
                                this_->generate_error_message(
                                    "first operand has unsupported "
                                    "number of dimensions"));
                        }
                    }

                    // local or distributed shape is asked, no dim
                    switch (numdims)
                    {
                    case 0:
                        return this_->shape0d(std::move(arg));

                    case 1:
                        return this_->shape1d(std::move(arg));

                    case 2:
                        return this_->shape2d(std::move(arg));

                    case 3:
                        return this_->shape3d(std::move(arg));

                    case 4:
                        return this_->shape4d(std::move(arg));

                    default:
                        HPX_THROW_EXCEPTION(hpx::bad_parameter,
                            "extract_shape::eval",
                            this_->generate_error_message(
                                "first operand has unsupported "
                                    "number of dimensions"));
                    }
                },
                value_operand(operands[0], args,
                    name_, codename_, std::move(ctx)));
        }

        // cannot be on len_mode
        // local or distributed shape is asked on a dimension
        return hpx::dataflow(hpx::launch::sync,
            [this_ = std::move(this_)](
                    hpx::future<primitive_argument_type>&& farg,
                    hpx::future<std::int64_t>&& findex)
            ->  primitive_argument_type
            {
                auto&& arg = farg.get();

                switch (extract_numeric_value_dimension(
                    arg, this_->name_, this_->codename_))
                {
                case 0:
                    return this_->shape0d(std::move(arg), findex.get());

                case 1:
                    return this_->shape1d(std::move(arg), findex.get());

                case 2:
                    return this_->shape2d(std::move(arg), findex.get());

                case 3:
                    return this_->shape3d(std::move(arg), findex.get());

                case 4:
                    return this_->shape4d(std::move(arg), findex.get());

                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "extract_shape::eval",
                        this_->generate_error_message(
                            "first operand has unsupported number of "
                            "dimensions"));
                }
            },
            value_operand(operands[0], args, name_, codename_, ctx),
            scalar_integer_operand_strict(operands[1], args,
                name_, codename_, ctx));
    }
}}}
