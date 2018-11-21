// Copyright (c) 2018 Bita Hasheminezhad
// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/config.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/plugins/matrixops/squeeze_operation.hpp>
#include <phylanx/util/matrix_iterators.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/naming.hpp>
#include <hpx/include/util.hpp>
#include <hpx/throw_exception.hpp>
#include <hpx/util/iterator_facade.hpp>
#include <hpx/util/optional.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <blaze/Math.h>

///////////////////////////////////////////////////////////////////////////////
namespace phylanx { namespace execution_tree { namespace primitives
{
    ///////////////////////////////////////////////////////////////////////////
    match_pattern_type const squeeze_operation::match_data =
    {
        hpx::util::make_tuple("squeeze",
        std::vector<std::string>{"squeeze()"},
        &create_squeeze_operation, &create_primitive<squeeze_operation>,
        "a, axis\n"
        "Args:\n"
        "\n"
        "    a (vector or matrix) : a vector or matrix\n"
        "    axis (optional, integer): an axis to squeeze along\n"
        "\n"
        "Returns:\n"
        "\n"
        "Remove single-dimensional entries from the shape of an array")
    };

    ///////////////////////////////////////////////////////////////////////////
    squeeze_operation::squeeze_operation(
        primitive_arguments_type&& operands,
        std::string const& name, std::string const& codename)
      : primitive_component_base(std::move(operands), name, codename)
    {}

    primitive_argument_type squeeze_operation::squeeze0d(arg_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        // works for any axis value
        return primitive_argument_type{arg.scalar()};
    }

    primitive_argument_type squeeze_operation::squeeze1d(arg_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        // works for any axis value
        auto v = arg.vector();
        if (v.size() == 1)
            return primitive_argument_type{v[0]};

        return primitive_argument_type{arg};
    }

    primitive_argument_type squeeze_operation::squeeze2d(arg_type&& arg,
        hpx::util::optional<std::int64_t> axis) const
    {
        // works for any axis value
        auto m = arg.matrix();
        if (m.columns() == 1 || m.rows() == 1)
        {
            if (m.columns() == 1 && m.rows() == 1)
                return primitive_argument_type{m(0, 0)};
            else if (m.columns() == 1)
                return primitive_argument_type{
                    blaze::DynamicVector<double>{blaze::column(m, 0ul)}};
            else if (m.rows() == 1)
                return primitive_argument_type{blaze::DynamicVector<double>{
                    blaze::trans(blaze::row(m, 0ul))}};
        }

        return primitive_argument_type{arg};
    }
    ///////////////////////////////////////////////////////////////////////////
    hpx::future<primitive_argument_type> squeeze_operation::eval(
        primitive_arguments_type const& operands,
        primitive_arguments_type const& args,
        eval_context ctx) const
    {
        if (operands.empty() || operands.size() > 2)
        {
            HPX_THROW_EXCEPTION(hpx::bad_parameter,
                "squeeze_operation::eval",
                util::generate_error_message(
                    "the squeeze_operation primitive requires exactly one, or "
                    "two operands",
                    name_, codename_));
        }

        for (auto const& i : operands)
        {
            if (!valid(i))
            {
                HPX_THROW_EXCEPTION(hpx::bad_parameter,
                    "squeeze_operation::eval",
                    util::generate_error_message(
                        "the squeeze_operation primitive requires that the "
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
                // Extract axis
                // Presence of axis changes behavior for >2d cases
                hpx::util::optional<std::int64_t> axis;

                // axis is the second argument
                if (args.size() > 1)
                {
                    if (valid(args[1]))
                        axis = execution_tree::extract_scalar_integer_value(
                            args[1], this_->name_, this_->codename_);
                }

                // Extract the matrix
                arg_type a = execution_tree::extract_numeric_value(
                    args[0], this_->name_, this_->codename_);

                std::size_t a_dims = a.num_dimensions();
                switch (a_dims)
                {
                case 0:
                    return this_->squeeze0d(std::move(a), axis);
                case 1:
                    return this_->squeeze1d(std::move(a), axis);
                case 2:
                    return this_->squeeze2d(std::move(a), axis);
                default:
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "squeeze_operation::eval",
                        util::generate_error_message("operand a has an invalid "
                                                     "number of dimensions",
                            this_->name_, this_->codename_));
                }
            }),
            detail::map_operands(
                operands, functional::value_operand{}, args, name_, codename_));
    }
}}}
