// Copyright (c) 2017-2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/execution_tree/primitives/extract_shape.hpp>
#include <phylanx/ir/node_data.hpp>

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
    primitive create_extract_shape(hpx::id_type const& locality,
        std::vector<primitive_argument_type>&& operands,
        std::string const& name, std::string const& codename)
    {
        static std::string type("shape");
        return create_primitive_component(
            locality, type, std::move(operands), name, codename);
    }

    match_pattern_type const extract_shape::match_data =
    {
        hpx::util::make_tuple("shape",
            std::vector<std::string>{"shape(_1, _2)", "shape(_1)"},
            &create_extract_shape, &create_primitive<extract_shape>)
    };

    ///////////////////////////////////////////////////////////////////////////
    extract_shape::extract_shape(
            std::vector<primitive_argument_type> && operands,
            std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type extract_shape::shape0d(args_type&& args) const
    {
        if (args.size() == 1)
        {
            std::vector<primitive_argument_type> result{};
            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::eval",
            "index out of range");
    }

    primitive_argument_type extract_shape::shape1d(args_type&& args) const
    {
        if (args.size() == 1)
        {
            std::vector<primitive_argument_type> result{
                primitive_argument_type{std::int64_t(args[0].size())}};
            return primitive_argument_type{std::move(result)};
        }

        HPX_THROW_EXCEPTION(hpx::bad_parameter,
            "extract_shape::eval",
            "index out of range");
    }

    primitive_argument_type extract_shape::shape2d(args_type&& args) const
    {
        auto dims = args[0].dimensions();
        if (args.size() == 1)
        {
            // return a list of numbers representing the
            // dimensions of the first argument
            std::vector<primitive_argument_type> result{
                primitive_argument_type{std::int64_t(dims[0])},
                primitive_argument_type{std::int64_t(dims[1])}};
            return primitive_argument_type{std::move(result)};
        }

        return primitive_argument_type{
            std::int64_t(dims[std::size_t(args[1][0])])};
    }

    hpx::future<primitive_argument_type> extract_shape::eval(
        std::vector<primitive_argument_type> const& operands,
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::eval",
                execution_tree::generate_error_message(
                    "the extract_shape primitive requires one or two "
                        "operands",
                    name_, codename_));
        }

        if (!valid(operands[0]) ||
            (operands.size() == 2 && !valid(operands[1])))
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "extract_shape::eval",
                execution_tree::generate_error_message(
                    "the extract_shape primitive requires that the "
                        "arguments given by the operands array are "
                        "valid",
                    name_, codename_));
        }

        auto this_ = this->shared_from_this();
        return hpx::dataflow(hpx::launch::sync, hpx::util::unwrapping(
            [this_](args_type && args) -> primitive_argument_type
            {
                auto dims = args[0].num_dimensions();
                switch (dims)
                {
                case 0:
                    return this_->shape0d(std::move(args));
                case 1:
                    return this_->shape1d(std::move(args));
                case 2:
                    return this_->shape2d(std::move(args));
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "extract_shape::eval",
                        execution_tree::generate_error_message(
                            "first operand has unsupported "
                                "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::numeric_operand{}, args,
                name_, codename_));
    }

    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> extract_shape::eval(
        std::vector<primitive_argument_type> const& args) const
    {
        if (operands_.empty())
        {
            return eval(args, noargs);
        }
        return eval(operands_, args);
    }
}}}
